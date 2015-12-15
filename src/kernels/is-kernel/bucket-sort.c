/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 */

#include <limits.h>
#include <omp.h>
#include <stdio.h>

#include <mylib/util.h>

#include "list.h"

#define NBUCKETS 8192

extern int verbose;
extern int nthreads;

/*
 * Merge sort algorithm.
 */
extern void mergesort(struct list *l);

#if defined(_STATIC_SCHEDULE_)

static omp_lock_t locks[NBUCKETS];

/*
 * Bucket sort algorithm.
 */
void bucketsort(int *array, int n)
{
	int off;                     /* Offset for numbers.      */
	int min;                     /* Min number in array.     */
	int max;                     /* Max number in array.     */
	int maxp;                    /* Max private.             */
	int minp;                    /* Min private.             */
	int i, j;                    /* Loop indexes.            */
	int range;                   /* Bucket range.            */
	struct list **buckets;       /* Buckets.                 */
	unsigned ptrs[NBUCKETS + 1]; /* Cumulative bucket sizes. */
	
	/* Create buckets. */
	buckets = smalloc(NBUCKETS*sizeof(struct list *));
	for (i = 0; i < NBUCKETS; i++)
	{
		buckets[i] = list_create();
		omp_init_lock(&locks[i]);
	}	

	min = INT_MAX;
	max = INT_MIN;
	
	#pragma omp parallel private(i, j, minp, maxp)
	{
		/* Find max number in the array. */
		minp = INT_MAX;
		maxp = INT_MIN;
		#pragma omp for 
		for (i = 0; i < n; i++)
		{
			/* Found min. */
			if (array[i] < minp)
				minp = array[i];
			else if (array[i] > maxp)
				maxp = array[i];
		}
		#pragma omp critical
		{
			if (maxp > max)
				max = maxp;
				
			if (minp < min)
				min = minp;
		}
		
		/* Distribute numbers into buckets. */
		#pragma omp master
		{
			range = abs(max - min)/NBUCKETS;
			off = (min < 0) ? abs(min) : 0;
		}
		#pragma omp barrier

		#pragma omp for schedule (static)
		for (i = 0; i < n; i++)
		{
			j = (array[i] + off)/range;
			if (j >= NBUCKETS)
				j = NBUCKETS - 1;
			
			omp_set_lock(&locks[j]);	
			list_push(buckets[j], array[i]);
			omp_unset_lock(&locks[j]);
		}
			
		/* Balance workload. */
		#pragma omp master
		{
			ptrs[0] = 0;
			for (j = 1; j < (NBUCKETS + 1); j++)
				ptrs[j] = ptrs[j - 1] + list_length(buckets[j - 1]);
		}
		#pragma omp barrier
		
		/* Sort Each bucket. */
		#pragma omp for schedule(static)
		for (i = 0; i < NBUCKETS; i++)
		{
			if (!list_empty(buckets[i]))
					mergesort(buckets[i]);
		}
		
		/* Rebuild array. */
		#pragma omp for schedule(static)
		for (i = 0; i < NBUCKETS; i++)
		{
			j = ptrs[i];
			
			while (!list_empty(buckets[i]))
				array[j++] = list_pop(buckets[i]);
		}
	}
	
	/* House keeping. */
	for (i = 0; i < NBUCKETS; i++)
	{
		list_destroy(buckets[i]);
		omp_destroy_lock(&locks[i]);
	}
	free(buckets);
}

#elif defined(_DYNAMIC_SCHEDULE_)

static omp_lock_t locks[NBUCKETS];

/*
 * Bucket sort algorithm.
 */
void bucketsort(int *array, int n)
{
	int off;                     /* Offset for numbers.      */
	int min;                     /* Min number in array.     */
	int max;                     /* Max number in array.     */
	int maxp;                    /* Max private.             */
	int minp;                    /* Min private.             */
	int i, j;                    /* Loop indexes.            */
	int range;                   /* Bucket range.            */
	struct list **buckets;       /* Buckets.                 */
	unsigned ptrs[NBUCKETS + 1]; /* Cumulative bucket sizes. */
	
	/* Create buckets. */
	buckets = smalloc(NBUCKETS*sizeof(struct list *));
	for (i = 0; i < NBUCKETS; i++)
	{
		buckets[i] = list_create();
		omp_init_lock(&locks[i]);
	}	
	
	min = INT_MAX;
	max = INT_MIN;
	
	#pragma omp parallel private(i, j, minp, maxp)
	{
		/* Find max number in the array. */
		minp = INT_MAX;
		maxp = INT_MIN;
		#pragma omp for 
		for (i = 0; i < n; i++)
		{
			/* Found min. */
			if (array[i] < minp)
				minp = array[i];
			else if (array[i] > maxp)
				maxp = array[i];
		}
		#pragma omp critical
		{
			if (maxp > max)
				max = maxp;
				
			if (minp < min)
				min = minp;
		}
		
		/* Distribute numbers into buckets. */
		#pragma omp master
		{
			range = abs(max - min)/NBUCKETS;
			off = (min < 0) ? abs(min) : 0;
		}
		#pragma omp barrier

		#pragma omp for schedule (static)
		for (i = 0; i < n; i++)
		{
			j = (array[i] + off)/range;
			if (j >= NBUCKETS)
				j = NBUCKETS - 1;
			
			omp_set_lock(&locks[j]);	
			list_push(buckets[j], array[i]);
			omp_unset_lock(&locks[j]);
		}
			
		/* Balance workload. */
		#pragma omp master
		{
			ptrs[0] = 0;
			for (j = 1; j < (NBUCKETS + 1); j++)
				ptrs[j] = ptrs[j - 1] + list_length(buckets[j - 1]);
		}
		#pragma omp barrier
		
		/* Sort Each bucket. */
		#pragma omp for schedule(dynamic)
		for (i = 0; i < NBUCKETS; i++)
		{
			if (!list_empty(buckets[i]))
					mergesort(buckets[i]);
		}
		
		/* Rebuild array. */
		#pragma omp for schedule(dynamic)
		for (i = 0; i < NBUCKETS; i++)
		{
			j = ptrs[i];
			
			while (!list_empty(buckets[i]))
				array[j++] = list_pop(buckets[i]);
		}
	}
	
	/* House keeping. */
	for (i = 0; i < NBUCKETS; i++)
	{
		list_destroy(buckets[i]);
		omp_destroy_lock(&locks[i]);
	}
	free(buckets);
}

#elif defined(_RUNTIME_SCHEDULE_)

static omp_lock_t locks[NBUCKETS];

/*
 * For now, libgomp hopes that we will
 * fill these structures. A better
 * way to achieve the same think would
 * be to do something like:
 * 
 *   #pragma omp paralell for tasks(myarray, ntasks)
 */
unsigned tasks1[NBUCKETS];
unsigned tasks2[NBUCKETS];
unsigned *__tasks;
unsigned __ntasks = NBUCKETS;

/*
 * Bucket sort algorithm.
 */
void bucketsort(int *array, int n)
{
	int off;                     /* Offset for numbers.      */
	int min;                     /* Min number in array.     */
	int max;                     /* Max number in array.     */
	int maxp;                    /* Max private.             */
	int minp;                    /* Min private.             */
	int i, j;                    /* Loop indexes.            */
	int range;                   /* Bucket range.            */
	struct list **buckets;       /* Buckets.                 */
	unsigned ptrs[NBUCKETS + 1]; /* Cumulative bucket sizes. */
	
	/* Create buckets. */
	buckets = smalloc(NBUCKETS*sizeof(struct list *));
	for (i = 0; i < NBUCKETS; i++)
	{
		buckets[i] = list_create();
		omp_init_lock(&locks[i]);
	}
	
	min = INT_MAX;
	max = INT_MIN;
	
	#pragma omp parallel private(i, j, minp, maxp)
	{
		/* Find max number in the array. */
		minp = INT_MAX;
		maxp = INT_MIN;
		#pragma omp for 
		for (i = 0; i < n; i++)
		{
			/* Found min. */
			if (array[i] < minp)
				minp = array[i];
			else if (array[i] > maxp)
				maxp = array[i];
		}
		#pragma omp critical
		{
			if (maxp > max)
				max = maxp;
				
			if (minp < min)
				min = minp;
		}
		
		/* Distribute numbers into buckets. */
		#pragma omp master
		{
			range = abs(max - min)/NBUCKETS;
			off = (min < 0) ? abs(min) : 0;
		}
		#pragma omp barrier

		#pragma omp for schedule (static)
		for (i = 0; i < n; i++)
		{
			j = (array[i] + off)/range;
			if (j >= NBUCKETS)
				j = NBUCKETS - 1;
			
			omp_set_lock(&locks[j]);	
			list_push(buckets[j], array[i]);
			omp_unset_lock(&locks[j]);
		}
			
		/* Balance workload. */
		#pragma omp master
		{
			ptrs[0] = 0;
			for (i = 0, j = 1; i < NBUCKETS; i++, j++)
			{
				tasks2[i] = tasks1[i] = list_length(buckets[i]);
				ptrs[j] = ptrs[j - 1] + list_length(buckets[i]);
				
				if (verbose){
					fprintf(stderr, "%u\n", tasks1[i]);
				}
			}

			__tasks = tasks1;
		}
		#pragma omp barrier
		
		/* Sort Each bucket. */
		#pragma omp for schedule(runtime)
		for (i = 0; i < NBUCKETS; i++)
		{
			if (!list_empty(buckets[i]))
				mergesort(buckets[i]);
		}
		
		#pragma omp master
		__tasks = tasks2;
		#pragma omp barrier
		
		/* Rebuild array. */
		#pragma omp for schedule(runtime)
		for (i = 0; i < NBUCKETS; i++)
		{
			j = ptrs[i];
			
			while (!list_empty(buckets[i]))
				array[j++] = list_pop(buckets[i]);
		}
	}
	
	/* House keeping. */
	for (i = 0; i < NBUCKETS; i++)
	{
		list_destroy(buckets[i]);
		omp_destroy_lock(&locks[i]);
	}
	free(buckets);
}

#endif

