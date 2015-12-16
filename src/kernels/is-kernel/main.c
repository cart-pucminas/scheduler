/*
 * Copyright(C) 2014 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * Integer-Sort Benchmark Kernel.
 */

#include <math.h>
#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <mylib/util.h>

/**
 * @brief Input data paramters.
 */
/**@{*/
#define FACTOR          134217728 /**< Multiplying factor. */
#define GAUSSIAN_STDDEV 0.85      /**< Standard deviation. */
#define GAUSSIAN_MEAN   2.0       /**< Mean.               */
#define MIN             0.0       /**< Minimum value.      */
#define MAX             4.0       /**< Maximum value.      */
/**@*/

/*
 * Bucket sort algorithm.
 */
extern void bucketsort(int *array, int n);

/*
 * Problem.
 */
struct problem
{
	int n; /* Number of elements. */
};

/* Problem sizes. */
static struct problem tiny        = {  33554432 };
static struct problem small       = {  67108864 };
static struct problem workstation = { 134217728 };
static struct problem standard    = { 268435456 };
static struct problem large       = { 536870912 };

/* Benchmark parameters. */
int verbose = 0;                  /* Be verbose?        */
int nthreads = 1;                 /* Number of threads. */
static struct problem *p = &tiny; /* Problem.           */

/*
 * Prints program usage and exits.
 */
static void usage(void)
{
	printf("Usage: kmeans [options]\n");
	printf("Brief: Kmeans Benchmark Kernel\n");
	printf("Options:\n");
	printf("  --help             Display this information and exit\n");
	printf("  --nthreads <value> Set number of threads\n");
	printf("  --class <name>     Set problem class:\n");
	printf("                       - small\n");
	printf("                       - workstation\n");
	printf("                       - standard\n");
	printf("                       - large\n");
	printf("  --verbose          Be verbose\n");
	exit(0);
}

/*
 * Reads command line arguments.
 */
static void readargs(int argc, char **argv)
{
	int i;     /* Loop index.       */
	char *arg; /* Working argument. */
	int state; /* Processing state. */
	
	/* State values. */
	#define READ_ARG     0 /* Read argument.         */
	#define SET_NTHREADS 1 /* Set number of threads. */
	#define SET_CLASS    2 /* Set problem class.     */
	
	state = READ_ARG;
	
	/* Read command line arguments. */
	for (i = 1; i < argc; i++)
	{
		arg = argv[i];
		
		/* Set value. */
		if (state != READ_ARG)
		{
			switch (state)
			{
				/* Set problem class. */
				case SET_CLASS :
					if (!strcmp(argv[i], "tiny"))
						p = &tiny;
					else if (!strcmp(argv[i], "small"))
						p = &small;
					else if (!strcmp(argv[i], "workstation"))
						p = &workstation;
					else if (!strcmp(argv[i], "standard"))
						p = &standard;
					else if (!strcmp(argv[i], "large"))
						p = &large;
					else 
						usage();
					state = READ_ARG;
					break;
				
				/* Set number of threads. */
				case SET_NTHREADS :
					nthreads = atoi(arg);
					state = READ_ARG;
					break;
				
				default:
					usage();			
			}
			
			continue;
		}
		
		/* Parse argument. */
		if (!strcmp(arg, "--verbose"))
			verbose = 1;
		else if (!strcmp(arg, "--nthreads"))
			state = SET_NTHREADS;
		else if (!strcmp(arg, "--class"))
			state = SET_CLASS;
		else
			usage();
	}
	
	/* Invalid argument(s). */
	if (nthreads < 1)
		usage();
}

/*
 * Runs benchmark.
 */
int main(int argc, char **argv)
{
	double tmp;     /* Temporary number.   */
	int *a;         /* Array to be sorted. */
	uint64_t end;   /* End time.           */
	uint64_t start; /* Start time.         */
	
	readargs(argc, argv);
	
	srandnum(0);
	omp_set_num_threads(nthreads);
	
	/* Benchmark initialization. */
	a = smalloc(p->n*sizeof(int));
	for (int i = 0; i < p->n; i++)
	{
		do
		{
			tmp = normalnum(GAUSSIAN_MEAN, GAUSSIAN_STDDEV);
		} while ((tmp < MIN) || (tmp > MAX));
		
		a[i] = (int)ceil(tmp*FACTOR);
		fprintf(stderr, "%d\n", a[i]);
	}
	
	/* Cluster data. */
	start = timer_get();
	bucketsort(a, p->n);
	end = timer_get();
	printf("%f\n", (end - start)/1000.0);
	
	/* House keeping. */
	free(a);
	
	return (0);
}
