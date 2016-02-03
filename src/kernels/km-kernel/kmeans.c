/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * kmeans.c - kmeans() implementation.
 */

#include <stdio.h>
#include <math.h>
#include <omp.h>
#include <string.h>
#include <stdlib.h>

#include <papi.h>

#include <mylib/util.h>

#include "vector.h"

extern int verbose;
extern int nthreads;

/*
 * For now, libgomp hopes that we will
 * fill these structures. A better
 * way to achieve the same think would
 * be to do something like:
 * 
 *   #pragma omp paralell for tasks(myarray, ntasks)
 */
unsigned *__tasks;
unsigned __ntasks;

/* Kmeans data. */
float mindistance;   /* Minimum distance.             */
int npoints;         /* Number of points.             */
vector_t *data;      /* Data being clustered.         */
int ncentroids;      /* Number of clusters.           */
int *map;            /* Map of clusters.              */
vector_t *centroids; /* Data centroids.               */
vector_t *tmp;       /* Temporary centroids.          */
int *dirty;          /* Dirty centroid?               */
int *too_far;        /* Are there any points too far? */
int *has_changed;    /* has any centroid change?      */

#if defined(_STATIC_SCHEDULE_)

/*
 * Populates clusters.
 */
static void populate(void)
{
	int tid;        /* Thread ID.          */
	int i, j;       /* Loop indexes.       */
	float tmp;      /* Auxiliary variable. */
	float distance; /* Distance.           */
	
	memset(too_far, 0, nthreads*sizeof(int));
	memset(dirty, 0, ncentroids*sizeof(int));
	
	/* Iterate over data points. */
	#pragma omp parallel private(i, j, tmp, distance, tid) default(shared)
	{
		tid = omp_get_thread_num();
		
		#pragma omp for
		for (i = 0; i < npoints; i++)
		{	
			distance = vector_distance(centroids[map[i]], data[i]);
			
			/* Look for closest cluster. */
			for (j = 0; j < ncentroids; j++)
			{	
				/* Point is in this cluster. */
				if (j == map[i])
					continue;
					
				tmp = vector_distance(centroids[j], data[i]);
				
				/* Found. */
				if (tmp < distance)
				{
					map[i] = j;
					distance = tmp;
					
					#pragma omp critical
					dirty[j] = 1;
				}
			}
			
			/* Cluster is too far away. */
			if (distance > mindistance)
				too_far[tid] = 1;
		}
	}
}

/*
 * Computes cluster's centroids.
 */
static void compute_centroids(void)
{
	int tid;        /* Thread ID.          */
	int i, j;       /* Loop indexes.       */
	int population; /* Cluster population. */
	int max, min, sum;
	int load[nthreads];
	
	if (verbose)
		memset(load, 0, nthreads*sizeof(int));
	
	memset(has_changed, 0, nthreads*sizeof(int));
	
	/* Compute means. */
	#pragma omp parallel private(i, j, population, tid) default(shared) 
	{	
		tid = omp_get_thread_num();
		
		#pragma omp for schedule(static)
		for (i = 0; i < ncentroids; i++)
		{
			/* Cluster did not change. */
			if (!dirty[i])
				continue;
				
			/* Initialize temporary vector.*/
			vector_assign(tmp[tid], centroids[i]);
			vector_clear(centroids[i]);
			
			/* Compute cluster's mean. */
			population = 0;
			for (j = 0; j < npoints; j++)
			{
				/* Not a member of this cluster. */
				if (map[j] != i)
					continue;			
				
				vector_add(centroids[i], data[j]);
					
				population++;
			}		
			if (population > 1)
				vector_mult(centroids[i], 1.0/population);
			
			
			if (verbose)
				load[omp_get_thread_num()] += population;
			
			has_changed[tid] = 1;
		}
	}
	
	if (verbose)
	{
		sum = 0; min = INT_MAX; max = INT_MIN;
		for (i = 0; i < nthreads;i++)
		{
			sum += load[i];
			if (load[i] < min)
				min = load[i];
			if (load[i] > max)
				max = load[i];
		}
		fprintf(stderr, "Load Imbalance: %f\n", ((float)(max - min))/sum);
	}
}

#elif defined(_DYNAMIC_SCHEDULE_)

/*
 * Populates clusters.
 */
static void populate(void)
{
	int tid;        /* Thread ID.          */
	int i, j;       /* Loop indexes.       */
	float tmp;      /* Auxiliary variable. */
	float distance; /* Distance.           */
	
	memset(too_far, 0, nthreads*sizeof(int));
	memset(dirty, 0, ncentroids*sizeof(int));
	
	/* Iterate over data points. */
	#pragma omp parallel private(i, j, tmp, distance, tid) default(shared)
	{
		tid = omp_get_thread_num();
		
		#pragma omp for
		for (i = 0; i < npoints; i++)
		{	
			distance = vector_distance(centroids[map[i]], data[i]);
			
			/* Look for closest cluster. */
			for (j = 0; j < ncentroids; j++)
			{	
				/* Point is in this cluster. */
				if (j == map[i])
					continue;
					
				tmp = vector_distance(centroids[j], data[i]);
				
				/* Found. */
				if (tmp < distance)
				{
					map[i] = j;
					distance = tmp;
					
					#pragma omp critical
					dirty[j] = 1;
				}
			}
			
			/* Cluster is too far away. */
			if (distance > mindistance)
				too_far[tid] = 1;
		}
	}
}

/*
 * Computes cluster's centroids.
 */
static void compute_centroids(void)
{
	int tid;        /* Thread ID.          */
	int i, j;       /* Loop indexes.       */
	int population; /* Cluster population. */
	int max, min, sum;
	int load[nthreads];
	
	if (verbose)
		memset(load, 0, nthreads*sizeof(int));
	
	memset(has_changed, 0, nthreads*sizeof(int));
	
	/* Compute means. */
	#pragma omp parallel private(i, j, population, tid) default(shared) 
	{	
		tid = omp_get_thread_num();
		
		#pragma omp for schedule(dynamic)
		for (i = 0; i < ncentroids; i++)
		{
			/* Cluster did not change. */
			if (!dirty[i])
				continue;
				
			/* Initialize temporary vector.*/
			vector_assign(tmp[tid], centroids[i]);
			vector_clear(centroids[i]);
			
			/* Compute cluster's mean. */
			population = 0;
			for (j = 0; j < npoints; j++)
			{
				/* Not a member of this cluster. */
				if (map[j] != i)
					continue;			
				
				vector_add(centroids[i], data[j]);
					
				population++;
			}		
			if (population > 1)
				vector_mult(centroids[i], 1.0/population);
			
			if (verbose)
				load[omp_get_thread_num()] += population;
			
			has_changed[tid] = 1;
		}
	}
	
	if (verbose)
	{
		sum = 0; min = INT_MAX; max = INT_MIN;
		for (i = 0; i < nthreads;i++)
		{
			sum += load[i];
			if (load[i] < min)
				min = load[i];
			if (load[i] > max)
				max = load[i];
		}
		fprintf(stderr, "Load Imbalance: %f\n", ((float)(max - min))/sum);
	}
}

#elif defined(_RUNTIME_SCHEDULE_)

unsigned *__tasks2;

/*
 * Populates clusters.
 */
static void populate(void)
{
	int tid;            /* Thread ID.          */
	int i, j;           /* Loop indexes.       */
	float tmp_distance; /* Auxiliary variable. */
	float distance;     /* Distance.           */

	memset(too_far, 0, nthreads*sizeof(int));
	memset(dirty, 0, ncentroids*sizeof(int));
	
	/* Iterate over data points. */
	#pragma omp parallel private(i, j, tmp_distance, distance, tid) default(shared)
	{
		tid = omp_get_thread_num();
		
		#pragma omp for
		for (i = 0; i < npoints; i++)
		{	
			distance = vector_distance(centroids[map[i]], data[i]);
			
			/* Look for closest cluster. */
			for (j = 0; j < ncentroids; j++)
			{	
				/* Point is in this cluster. */
				if (j == map[i])
					continue;
					
				tmp_distance = vector_distance(centroids[j], data[i]);
				
				/* Found. */
				if (tmp_distance < distance)
				{
					#pragma omp critical
					{
						
						__tasks[map[i]]--;
						__tasks[j]++;
						dirty[j] = 1;
					}
					
					distance = tmp_distance;
					map[i] = j;
				}
			}
			
			/* Cluster is too far away. */
			if (distance > mindistance)
				too_far[tid] = 1;
		}
	}
}

/*
 * Computes cluster's centroids.
 */
static void compute_centroids(void)
{
	int tid;             /* Thread ID.           */
	int i, j;            /* Loop indexes.        */
	int pop;             /* Centroid population. */
	unsigned *tmp__tasks; /* Used for swap.       */
	int max, min, sum;
	int load[nthreads];
	
	if (verbose)
		memset(load, 0, nthreads*sizeof(int));
	
	memset(has_changed, 0, nthreads*sizeof(int));
	
	/* Remove clean tasks. */
	for (i = 0; i < ncentroids; i++)
	{
		if (!dirty[i])
			__tasks[i] = 0;
	}
	
	/* Compute means. */
	#pragma omp parallel private(i, j, pop, tid) default(shared) 
	{	
		tid = omp_get_thread_num();
		
		#pragma omp for schedule(runtime)
		for (i = 0; i < ncentroids; i++)
		{
			/* Initialize temporary vector.*/
			vector_assign(tmp[tid], centroids[i]);
			vector_clear(centroids[i]);
			
			/* Compute cluster's mean. */
			pop = 0;
			for (j = 0; j < npoints; j++)
			{
				/* Not a member of this cluster. */
				if (map[j] != i)
					continue;			
				
				vector_add(centroids[i], data[j]);
					
				pop++;
			}		
			if (pop > 1)
				vector_mult(centroids[i], 1.0/pop);
			
			/* Cluster mean has changed. */
			if (!vector_equal(tmp[tid], centroids[i]))
				has_changed[tid] = 1;
			
			if (verbose)
				load[omp_get_thread_num()] += pop;
			
			__tasks2[i] = pop;
		}
	}
	
	if (verbose)
	{
		sum = 0; min = INT_MAX; max = INT_MIN;
		for (i = 0; i < nthreads;i++)
		{
			sum += load[i];
			if (load[i] < min)
				min = load[i];
			if (load[i] > max)
				max = load[i];
		}
		fprintf(stderr, "Load Imbalance: %f\n", ((float)(max - min))/sum);
	}
	
	/* Black magic =) */
	tmp__tasks = __tasks;
	__tasks = __tasks2;
	__tasks2 = tmp__tasks;
}

#endif

/*
 * Clusters data. 
 */
int *kmeans(vector_t *_data, int _npoints, int _ncentroids, float _mindistance)
{
	int i, j;  /* Loop indexes. */
	int again; /* Loop again?   */
	int events[4] = { PAPI_L1_DCM, PAPI_L2_DCM, PAPI_L2_DCA, PAPI_L3_DCA };
	long long hwcounters[4];

	((void)__tasks);
	((void)__ntasks);
	
	/* Setup parameters. */
	data = _data;
	npoints = _npoints;
	ncentroids = _ncentroids;
	mindistance = _mindistance;
	
	/* Create auxiliary structures. */
	map  = scalloc(npoints, sizeof(int));
	too_far = smalloc(nthreads*sizeof(int));
	has_changed = smalloc(nthreads*sizeof(int));
	dirty = smalloc(ncentroids*sizeof(int));
	centroids = smalloc(ncentroids*sizeof(vector_t));
#ifdef _RUNTIME_SCHEDULE_
	__ntasks = ncentroids;
	__tasks = scalloc(ncentroids, sizeof(unsigned));
	__tasks2 = smalloc(ncentroids*sizeof(unsigned));
	__tasks[0] = npoints;
#endif
	for (i = 0; i < ncentroids; i++)
	{
		j = randnum()%npoints;
		centroids[i] = vector_create(vector_size(data[0]));
		vector_assign(centroids[i], data[j]);
		map[j] = i;
#ifdef _RUNTIME_SCHEDULE_
		__tasks[i]++;
		__tasks[0]--;
#endif
	}
	tmp = smalloc(nthreads*sizeof(vector_t));
	for (i = 0; i < nthreads; i++)
		tmp[i] = vector_create(vector_size(data[0]));
	
	/* Cluster data. */
	do
	{
		if (verbose)
		{
			/* Setup PAPI. */
			if (PAPI_start_counters(events, 4) != PAPI_OK)
			{
				fprintf(stderr, "failed to setup PAPI\n");
				exit(EXIT_FAILURE);
			}
		}
		
		populate();
		compute_centroids();

		/* Check if we need to loop. */
		for (i = 0; i < nthreads; i++)
		{
			/* We will need another iteration. */
			if (too_far[i] && has_changed[i])
				break;		
		}

		again = (i < nthreads) ? 1 : 0;
		
		if (verbose)
		{
			/* Exit PAPI. */
			if (PAPI_stop_counters(hwcounters, sizeof(events)) != PAPI_OK)
			{
				fprintf(stderr, "failed to read hardware counters\n");
				exit(EXIT_FAILURE);
			}
			
			fprintf(stderr, "L1 Misses: %lld\n", hwcounters[0]);
			fprintf(stderr, "L2 Misses: %lld\n", hwcounters[1]);
			fprintf(stderr, "L2 Accesses: %lld\n", hwcounters[2]);
			fprintf(stderr, "L3 Accesses: %lld\n", hwcounters[3]);
		}

	} while (again);
	
	
	/* House keeping.  */
	for (i = 0; i < ncentroids; i++)
		vector_destroy(centroids[i]);
	free(centroids);
	for (i = 0; i < nthreads; i++)
		vector_destroy(tmp[i]);
	free(tmp);
	free(too_far);
	free(has_changed);
	free(dirty);
#ifdef _RUNTIME_SCHEDULE_
	free(__tasks);
	free(__tasks2);
#endif

	return (map);
}
