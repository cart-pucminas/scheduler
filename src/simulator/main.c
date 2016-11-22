/*
 * Copyright(C) 2015 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <mylib/util.h>
#include <common.h>
#include <simulator.h>

/**
 * @brief Sorting order types.
 */
/**@{*/
#define SORT_ASCENDING  1 /**< Ascending sort.  */
#define SORT_DESCENDING 2 /**< Descending sort. */
#define SORT_RANDOM     3 /**< Random sort.     */
/**@}*/

/**
 * @name Program Parameters
 */
static struct
{
	const char *input; /**< Input data file.         */
	int nthreads;      /**< Number of threads.       */
	int ntasks;        /**< Number of tasks.         */
	int sort;          /**< Sorting order.           */
	int scheduler;     /**< Loop scheduler.          */
	int seed;          /**< Seed for task shuffling. */
} args = { NULL, 0, 0, 0, SCHEDULER_NONE, 0 };

/**
 * @brief Chunk size for the dynamic scheduling.
 */
unsigned chunksize = 1;

/**
 * @brief Threads.
 */
struct thread *threads = NULL;

/**
 * @brief Prints program usage and exits.
 * 
 * @details Prints program usage and exits gracefully.
 */
static void usage(void)
{
	printf("Usage: scheduler [options] <scheduler>\n");
	printf("Brief: loop scheduler simulator\n");
	printf("Scheduler:\n");
	printf("  lpt               Simulate longest processing time first scheduling\n");
	printf("  opt               Simulate optimum scheduling\n");
	printf("  static            Simulate static loop scheduling\n");
	printf("  dynamic           Simulate dynamic loop scheduling\n");
	printf("  workload-aware    Simulate workload-aware loop scheduling\n");
	printf("  smart-round-robin Simulate smart round-robin loop scheduling\n");
	printf("Options:\n");
	printf("  --input <filename>     Input workload file\n");
	printf("  --nthreads <number>    Number of threads\n");
	printf("  --niterations <number> Number iterations in the parallel loop\n");
	printf("  --sort <type>          Loop iteration sorting\n");
	printf("         ascending       Ascending order\n");
	printf("         descending      Descending order\n");
	printf("         random          Random order\n");
	printf("  --chunksize <num>      Chunk size for the dynamic scheduling\n");
	printf("  --seed <num>           Seed for task shuffling\n");
	printf("  --help                 Display this message\n");

	exit(EXIT_SUCCESS);
}


/*============================================================================*
 *                                Get Routines                                *
 *============================================================================*/

/**
 * @brief Gets tasks sorting type.
 * 
 * @param sortname Tasks sorting name.
 * 
 * @param Tasks sorting type.
 */
static int getsort(const char *sortname)
{
	if (!strcmp(sortname, "ascending"))
		return (SORT_ASCENDING);
	else if (!strcmp(sortname, "descending"))
		return (SORT_DESCENDING);
	else if (!strcmp(sortname, "random"))
		return (SORT_RANDOM);
	
	error("unsupported tasks sorting type");
	
	/* Never gets here. */
	return (-1);
}

/*============================================================================*
 *                             Argument Checking                              *
 *============================================================================*/

/**
 * @brief Checks program arguments.
 */
static void checkargs(const char *sortname)
{
	/* Check parameters. */
	if (!(args.nthreads > 0))
		error("invalid number of threads");
	else if (!(args.ntasks > 0))
		error("invalid number of iterations in the parallel loop");
	else if (sortname == NULL)
		error("invalid tasks sorting type");
	else if (args.scheduler == SCHEDULER_NONE)
		error("invalid scheduler");
	else if (args.input == NULL)
		error("missing input workload file");
}

/**
 * @brief Reads command line arguments.
 * 
 * @details Reads command line arguments.
 */
static void readargs(int argc, const char **argv)
{
	const char *sortname = NULL;
	
	/* Parse command line arguments. */
	for (int i = 1; i < argc; i++)
	{	
		/* Parse command. */
		if (!strcmp(argv[i], "--nthreads"))
			args.nthreads = atoi(argv[++i]);
		else if (!strcmp(argv[i], "--niterations"))
			args.ntasks = atoi(argv[++i]);
		else if (!strcmp(argv[i], "--sort"))
			sortname = argv[++i];
		else if (!strcmp(argv[i], "--chunksize"))
			chunksize = atoi(argv[++i]);
		else if (!strcmp(argv[i], "--input"))
			args.input = argv[++i];
		else if (!strcmp(argv[i], "--help"))
			usage();
		else if (!strcmp(argv[i], "--seed"))
			args.seed = atoi(argv[++i]);
		else
		{
			if (!strcmp(argv[i], "static"))
				args.scheduler = SCHEDULER_STATIC;
			else if (!strcmp(argv[i], "dynamic"))
				args.scheduler = SCHEDULER_DYNAMIC;
			else if (!strcmp(argv[i], "workload-aware"))
				args.scheduler = SCHEDULER_WORKLOAD_AWARE;
			else if (!strcmp(argv[i], "srr"))
				args.scheduler = SCHEDULER_SMART_ROUND_ROBIN;
			else if (!strcmp(argv[i], "lpt"))
				args.scheduler = SCHEDULER_LPT;
			else if (!strcmp(argv[i], "opt"))
				args.scheduler = SCHEDULER_OPT;
		}
	}
	
	checkargs(sortname);
	
	args.sort = getsort(sortname);
}

/*============================================================================*
 *                           WORKLOAD GENERATOR                               *
 *============================================================================*/

/**
 * @brief Greater than.
 * 
 * @param a1 First element.
 * @param a2 Second element.
 * 
 * @returns One if @p a1 is greater than @p a2 and minus one otherwise.
 */
static int greater(const void *a1, const void *a2)
{
	return ((*((unsigned *)a1) > *((unsigned *)a2)) ? 1 : -1);
}

/**
 * @brief Less than.
 * 
 * @param a1 First element.
 * @param a2 Second element.
 * 
 * @returns One if @p a1 is less than @p a2 and minus one otherwise.
 */
static int less(const void *a1, const void *a2)
{
	return ((*((unsigned *)a1) < *((unsigned *)a2)) ? 1 : -1);
}

/**
 * @brief Shuffles and array.
 * 
 * @param a    Target array.
 * @param n    Size of target array.
 * @param seed Seed for shuffling.
 */
static void array_shuffle(unsigned *a, unsigned n, int seed)
{
	/* Let us be totally random. */
	srand(seed);
	
	/* Shuffle array. */
	for (unsigned i = 0; i < n - 1; i++)
	{
		unsigned j; /* Shuffle index.  */
		unsigned t; /* Temporary data. */
		
		j = i + rand()/(RAND_MAX/(n - i) + 1);
			
		t = a[i];
		a[i] = a[j];
		a[j] = t;
	}
}

/**
 * @brief Sorts tasks.
 * 
 * @param tasks  Target tasks.
 * @param ntasks Number of tasks.
 * @param type   Sorting type.
 * @param seed   Seed for shuffling.
 */
static void tasks_sort(unsigned *tasks, unsigned ntasks, int type, int seed)
{
	/* Random sort. */
	if (type == SORT_RANDOM)
		array_shuffle(tasks, ntasks, seed);

	/* Ascending sort. */
	else if (type == SORT_ASCENDING)
		qsort(tasks, ntasks, sizeof(unsigned), greater);

	/* Descending sort. */
	else
		qsort(tasks, ntasks, sizeof(unsigned), less);
}

/**
 * @brief Reads input file
 * 
 * @param input Input filename.
 * @param ntasks Number of tasks.
 */
static unsigned *readfile(const char *input, unsigned ntasks)
{
	FILE *fp;
	unsigned *tasks;
	
	tasks = smalloc(ntasks*sizeof(unsigned));
	
	fp = fopen(input, "r");
	assert(fp != NULL);
	
	/* Read file. */
	for (unsigned i = 0; i < ntasks; i++)
	{
		if (fscanf(fp, "%u", &tasks[i]) == EOF)
		{
			if (feof(fp))
				error("unexpected end of file");
			else if (ferror(fp))
				error("cannot read file");
			else
				error("unknown error");
			break;
		}
	}
	
	/* I/O error. */
	if (ferror(fp))
		error("cannot read input file");
	
	fclose(fp);
	
	return (tasks);
}

/*============================================================================*
 *                               THREAD MANAGER                               *
 *============================================================================*/

/**
 * @brief Spawn threads.
 */
static void threads_spawn(void)
{
	/* Create threads. */
	threads = smalloc(args.nthreads*sizeof(struct thread));
	for (int i = 0; i < args.nthreads; i++)
	{
		threads[i].tid = i;
		threads[i].workload = 0;
		threads[i].ntasks = 0;
		threads[i].avg = 0;
		threads[i].max = 0;
		threads[i].min = UINT_MAX;
	}
}

/**
 * @brief Joins threads.
 */
static void threads_join(void)
{
	free(threads);
}

/*============================================================================*
 *                                  SIMULATOR                                 *
 *============================================================================*/

/**
 * @brief Loop scheduler simulator.
 */
int main(int argc, const const char **argv)
{
	unsigned total;
	unsigned max, min;
	unsigned *tasks;
	
	readargs(argc, argv);

	tasks = readfile(args.input, args.ntasks);
	
	tasks_sort(tasks, args.ntasks, args.sort, args.seed);
		
	threads_spawn();
	schedule(tasks, args.ntasks, args.nthreads, args.scheduler);
		
	/* Print statistics. */
	max = 0; min = UINT_MAX; total = 0;
	for (int i = 0; i < args.nthreads; i++)
	{
		if (min > threads[i].workload)
			min = threads[i].workload;
		if (max < threads[i].workload)
			max = threads[i].workload;
		total += threads[i].workload;
	}
	fprintf(stderr, "Total Cycles: %u\n", max);
	fprintf(stderr, "Load Imbalance: %lf\n", ((double)(max-min))/total);
	fprintf(stderr, "Slowdown: %lf\n", ((double)(max)/min));
	threads_join();

	/* House keeping. */
	free(tasks);
		
	return (EXIT_SUCCESS);
}
