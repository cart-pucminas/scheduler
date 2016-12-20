/*
 * Copyright(C) 2016 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This file is part of Scheduler.
 *
 * Scheduler is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * Scheduler is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Scheduler; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <mylib/util.h>

#include <statistics.h>
#include <workload.h>

/**
 * @name Program arguments.
 */
static struct
{
	distribution_tt (*dist)(void); /**< Probability distribution. */
	int nclasses;                  /**< Number of task classes.   */
	int ntasks;                    /**< Number of tasks.          */
	enum workload_sorting sorting; /**< Workload sorting.         */
	int skewness;                  /**< Workload skewness.        */
} args = { NULL, 0, 0, WORKLOAD_SHUFFLE, WORKLOAD_SKEWNESS_NULL };

/*============================================================================*
 * ARGUMENT CHECKING                                                          *
 *============================================================================*/

/**
 * @brief Prints program usage and exits.
 */
static void usage(void)
{
	printf("Usage: generator [options]\n");
	printf("Brief: workload generator\n");
	printf("Options:\n");
	printf("  --dist <name>          Probability distribution for task classes.\n");
	printf("         beta                a = 0.5 and b = 0.5\n");
	printf("         exponential         mu = 1.0\n");
	printf("         gamma               a = 5.0 and b = 1.0\n");
	printf("         gaussian            x = 0.0 and std = 1.0\n");
	printf("         uniform             a = 0.0 and b = 01.0\n");
	printf("  --nclasses <number>    Number of task classes.\n");
	printf("  --ntasks <number>      Number tasks.\n");
	printf("  --skewness <type>      Workload skewness.\n");
	printf("             left           Left\n");
	printf("             right          Right\n");
	printf("  --seed <number>        Seed value\n");
	printf("  --sort <type>          Tasks sorting,\n");
	printf("         ascending           Ascending order\n");
	printf("         descending          Descending order\n");
	printf("         shuffle             Shuffle\n");
	printf("  --help                 Display this message.\n");

	exit(EXIT_SUCCESS);
}

/**
 * @brief Gets a probability distribution.
 * 
 * @param distname Name of probability distribution.
 * 
 * @returns A probability distribution.
 */
static distribution_tt (*getdist(const char *distname))(void)
{
	if (!strcmp(distname, "beta"))
		return (dist_beta);
	if (!strcmp(distname, "exponential"))
		return (dist_exponential);
	if (!strcmp(distname, "gamma"))
		return (dist_gamma);
	if (!strcmp(distname, "gaussian"))
		return (dist_gaussian);
	if (!strcmp(distname, "uniform"))
		return (dist_uniform);
	
	error("unsupported probability distribution");
	
	/* Never gets here. */
	return (NULL);
}

/**
 * @brief Gets tasks sorting type.
 * 
 * @param sortname Tasks sorting name.
 * 
 * @returns Tasks sorting type.
 */
static enum workload_sorting getsort(const char *sortname)
{
	if (!strcmp(sortname, "ascending"))
		return (WORKLOAD_ASCENDING);
	else if (!strcmp(sortname, "descending"))
		return (WORKLOAD_DESCENDING);
	else if (!strcmp(sortname, "shuffle"))
		return (WORKLOAD_SHUFFLE);
	
	error("unsupported sorting type");
	
	/* Never gets here. */
	return (-1);
}

/**
 * @brief Gets workload skewness type.
 *
 * @param skewnessname Skewness name.
 *
 * @returns Workload skewness type.
 */
static int getskewness(const char *skewnessname)
{
	if (!strcmp(skewnessname, "left"))
		return (WORKLOAD_SKEWNESS_LEFT);
	if (!strcmp(skewnessname, "right"))
		return (WORKLOAD_SKEWNESS_RIGHT);

	error("unsupported workload skewness");

	/* Never gets here. */
	return (-1);
}

/**
 * @brief Checks program arguments.
 *
 * @param distname Name of probability distribution.
 * @param sortname Task sorting name.
 * @param skewnessname Skewness name.
 */
static void checkargs(const char *distname, const char *sortname, const char *skewnessname)
{
	if (distname == NULL)
		error("missing probability distribution");
	if (!(args.nclasses > 0))
		error("invalid number of task classes");
	if (!(args.ntasks > 0))
		error("invalid number of tasks");
	if (skewnessname == NULL)
		error("missing workload skewness");
	if (sortname == NULL)
		error("invalid task sorting");
}

/**
 * @brief Reads command line arguments.
 *
 * @param argc Argument count.
 * @param argv Argument variables.
 */
static void readargs(int argc, const char **argv)
{
	const char *distname = NULL;
	const char *sortname = NULL;
	const char *skewnessname = NULL;
	
	/* Parse command line arguments. */
	for (int i = 1; i < argc; i++)
	{	
		if (!strcmp(argv[i], "--dist"))
			distname = argv[++i];
		else if (!strcmp(argv[i], "--nclasses"))
			args.nclasses = atoi(argv[++i]);
		else if (!strcmp(argv[i], "--ntasks"))
			args.ntasks = atoi(argv[++i]);
		else if (!strcmp(argv[i], "--skewness"))
			skewnessname = argv[++i];
		else if (!strcmp(argv[i], "--seed"))
			srand(atoi(argv[++i]));
		else if (!strcmp(argv[i], "--sort"))
			sortname = argv[++i];
		else
			usage();
	}
	
	checkargs(distname, sortname, skewnessname);
	
	args.dist = getdist(distname);
	args.sorting = getsort(sortname);
	args.skewness = getskewness(skewnessname);
}

/*============================================================================*
 * WORKLOAD GENERATOR                                                         *
 *============================================================================*/

/**
 * @brief A synthetic workload generator.
 */
int main(int argc, const const char **argv)
{
	distribution_tt dist; /* Underlying probability distribution.   */
	histogram_tt hist;    /* Histogram of probability distribution. */
	workload_tt w;        /* Workload.                              */

	readargs(argc, argv);

	dist = args.dist();
	hist = distribution_histogram(dist, args.nclasses);
	w = workload_create(hist, args.skewness, args.ntasks);
	workload_sort(w, args.sorting);

	workload_write(stdout, w);

	/* House keeping, */
	distribution_destroy(dist);
	histogram_destroy(hist);
	workload_destroy(w);

	return (EXIT_SUCCESS);
}
