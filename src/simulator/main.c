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

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <mylib/util.h>
#include <common.h>
#include <simulator.h>
#include <pdf.h>

/**
 * @brief Supported probability density functions.
 */
/**@{*/
#define NR_PDFS      4 /**< Number of supported PDFs. */
#define PDF_BETA     1 /**< Beta.                     */
#define PDF_GAMMA    2 /**< Gamma.                    */
#define PDF_GAUSSIAN 3 /**< Gaussian.                 */
#define PDF_POISSON  4 /**< Poisson.                  */
/**@}*/

/**
 * @brief Name of supported probability density functions.
 */
static const char *pdfnames[NR_PDFS] = {
	"beta",     /* Beta.     */
	"gamma",    /* Gammma.   */
	"gaussian", /* Gaussian. */
	"poisson"   /* Poisson.  */
};

/**
 * @brief Supported kernels.
 */
/**{@*/
#define NR_KERNELS         4 /**< Number of supported kernels. */
#define KERNEL_LINEAR      1 /**< Linear kernel.               */
#define KERNEL_LOGARITHMIC 2 /**< Logarithmic kernel.          */
#define KERNEL_QUADRATIC   3 /**< Quadratic kernel.            */
#define KERNEL_CUBIC       4 /**< Cubic kernel.                */
/**@}*/

/**
 * @brief Name of supported kernel types.
 */
static const char *kernelnames[NR_KERNELS] = {
	"linear",    /* Linear.    */
	"logarithm", /* Logarithm. */
	"quadratic", /* Quadratic. */
	"cubic"      /* Cubic.     */
};

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
	int nthreads;    /**< Number of threads.                     */
	int ntasks;      /**< Number of tasks.                       */
	int pdf;         /**< Probability density function.          */
	double skewness; /**< Probability density function skewness. */
	int sort;        /**< Sorting order.                         */
	int kernel;      /**< Kernel type.                           */
	int scheduler;   /**< Loop scheduler.                        */
} args = { 0, 0, 0, 0.0, 0, 0, SCHEDULER_NONE};

/**
 * @brief Chunk size for the dynamic scheduling.
 */
unsigned chunksize = 0;

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
	printf("  static            Simulate static loop scheduling\n");
	printf("  dynamic           Simulate dynamic loop scheduling\n");
	printf("  workload-aware    Simulate workload-aware loop scheduling\n");
	printf("  smart-round-robin Simulate smart round-robin loop scheduling\n");
	printf("Options:\n");
	printf("  --nthreads <number>    Number of threads\n");
	printf("  --niterations <number> Number iterations in the parallel loop\n");
	printf("  --pdf <name>           Probability desity function for random numbers.\n");
	printf("        beta               a = 0.5 and b = 0.5\n");
	printf("        gamma              a = 1.0 and b = 2.0 \n");
	printf("        gaussian           x = 0.0 and std = 1.0\n");
	printf("        poisson                                \n");
	printf("  --skewness <number>    PDF skewness\n");
	printf("  --sort <type>         Loop iteration sorting\n");
	printf("         ascending      Ascending order\n");
	printf("         descending     Descending order\n");
	printf("         random         Random order\n");
	printf("  --kernel <name>       Kernel type\n");
	printf("           linear       Linear O(n)\n");
	printf("           logarithm    Logarithm O(n log n)\n");
	printf("           quadratic    Quadratic O(n^2)\n");
	printf("           cubic        Cubic O(n^3)\n");
	printf("  --chunksize <num>     Chunk size for the dynamic scheduling\n");
	printf("  --help                Display this message\n");

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

/**
 * @brief Gets PDF id.
 * 
 * @param pdfname PDF name.
 * 
 * @returns PDF id.
 */
static int getpdf(const char *pdfname)
{
	for (int i = 0; i < NR_PDFS; i++)
	{
		if (!strcmp(pdfname, pdfnames[i]))
			return (i + 1);
	}
	
	error("unsupported probability density function");
	
	/* Never gets here. */
	return (-1);
}

/**
 * @brief Gets kernel type.
 * 
 * @param kernelname Kernel name.
 * 
 * @returns Kernel type.
 */
static int getkernel(const char *kernelname)
{
	for (int i = 0; i < NR_KERNELS; i++)
	{
		if (!strcmp(kernelname, kernelnames[i]))
			return (i + 1);
	}
	
	error("unsupported kernel type");
	
	/* Never gets here. */
	return (-1);
}

/*============================================================================*
 *                             Argument Checking                              *
 *============================================================================*/

/**
 * @brief Checks program arguments.
 */
static void checkargs(const char *pdfname, const char *sortname, const char *kernelname)
{
	/* Check parameters. */
	if (!(args.nthreads > 0))
		error("invalid number of threads");
	else if (!(args.ntasks > 0))
		error("invalid number of iterations in the parallel loop");
	else if (pdfname == NULL)
		error("missing probability density function");
	else if (!(args.skewness > 0.1))
		error("invalid skewness for probability density function");
	else if (sortname == NULL)
		error("invalid tasks sorting type");
	else if (kernelname == NULL)
		error("invalid kernel type");
	else if (args.scheduler == SCHEDULER_NONE)
		error("invalid scheduler");
}

/**
 * @brief Reads command line arguments.
 * 
 * @details Reads command line arguments.
 */
static void readargs(int argc, const char **argv)
{
	const char *sortname = NULL;
	const char *pdfname = NULL; 
	const char *kernelname = NULL;
	
	/* Parse command line arguments. */
	for (int i = 1; i < argc; i++)
	{	
		/* Parse command. */
		if (!strcmp(argv[i], "--nthreads"))
			args.nthreads = atoi(argv[++i]);
		else if (!strcmp(argv[i], "--niterations"))
			args.ntasks = atoi(argv[++i]);
		else if (!strcmp(argv[i], "--pdf"))
			pdfname = argv[++i];
		else if (!strcmp(argv[i], "--skewness"))
			args.skewness = atof(argv[++i]);
		else if (!strcmp(argv[i], "--sort"))
			sortname = argv[++i];
		else if (!strcmp(argv[i], "--kernel"))
			kernelname = argv[++i];
		else if (!strcmp(argv[i], "--chunksize"))
			chunksize = atoi(argv[++i]);
		else if (!strcmp(argv[i], "--help"))
			usage();
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
		}
	}
	
	checkargs(pdfname, sortname, kernelname);
	
	args.pdf = getpdf(pdfname);
	args.sort = getsort(sortname);
	args.kernel = getkernel(kernelname);
}

/*============================================================================*
 *                           WORKLOAD GENERATOR                               *
 *============================================================================*/

/**
 * @brief Builds tasks histogram.
 * 
 * @param pdf         Probability density functions.
 * @param ntasks Number of tasks.
 * @param skewness    Skewness for probability density function.
 * 
 * @returns tasks histogram.
 */
static double *histogram_create(unsigned pdf, unsigned ntasks, double skewness)
{
	double *h = NULL;
	
	/* Generate input data. */
	switch (pdf)
	{
		/* Beta distribution. */
		case PDF_BETA:
			h = beta(ntasks, skewness);
			break;
			
		/* Gamma distribution. */
		case PDF_GAMMA:
			h = gamma(ntasks, skewness);
			break;
			
		/* Gaussian distribution. */
		case PDF_GAUSSIAN:
			h = gaussian(ntasks, skewness);
			break;
			
		/* Poisson distribution. */
		case PDF_POISSON:
			h = poisson(ntasks, skewness);
			break;
			
		/* Shouldn't happen. */
		default:
			error("unsupported probability density function");
			break;
	}
	
	return (h);
}

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
 * @param a Target array.
 * @param n Size of target array.
 */
static void array_shuffle(unsigned *a, unsigned n)
{
	/* Let us be totally random. */
	srand(time(NULL));
	
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
 */
static void tasks_sort(unsigned *tasks, unsigned ntasks, int type)
{
	/* Random sort. */
	if (type == SORT_RANDOM)
		array_shuffle(tasks, ntasks);

	/* Ascending sort. */
	else if (type == SORT_ASCENDING)
		qsort(tasks, ntasks, sizeof(unsigned), greater);

	/* Descending sort. */
	else
		qsort(tasks, ntasks, sizeof(unsigned), less);
}

/**
 * @brief Create tasks.
 * 
 * @param h tasks histogram.
 * @param ntasks Number of tasks.
 * 
 * @returns Tasks.
 */
static unsigned *tasks_create(const double *h, unsigned ntasks)
{
	unsigned *tasks;
	const unsigned FACTOR = 100000000;
	
	tasks = smalloc(ntasks*sizeof(unsigned));
	
	for (unsigned i = 0; i < ntasks; i++)
	{
		double x;
		
		x = h[i]*FACTOR;
		
		/* Check for corner cases. */
		if (x < 0)
			error("bad multiplying factor");
		
		tasks[i] = ceil(x);
	}
	
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
	double *h;
	unsigned cycles;
	unsigned *tasks;
	
	readargs(argc, argv);
	
	threads_spawn();

	h = histogram_create(args.pdf, args.ntasks, args.skewness);
	tasks = tasks_create(h, args.ntasks);
	tasks_sort(tasks, args.ntasks, args.sort);
	
	schedule(tasks, args.ntasks, args.nthreads, args.scheduler);
	
	/* Print statistics. */
	cycles = 0;
	for (int i = 0; i < args.nthreads; i++)
	{
		if (threads[i].workload > cycles)
			cycles = threads[i].workload;
		fprintf(stderr, "thread: %u\n", threads[i].workload);
	}
	fprintf(stderr, "Total Cycles: %u\n", cycles);
	
	/* House keeping. */
	threads_join();
	free(h);
	free(tasks);
	
	return (EXIT_SUCCESS);
}
