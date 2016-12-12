/*
 * Copyright(C) 2016 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This file is part of SimSched.
 *
 * SimSched is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * SimSched is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with SimSched; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <stdlib.h>
#include <string.h>

#include <mylib/util.h>

#include <scheduler.h>
#include <workload.h>

/**
 * @name Program Parameters
 */
static struct
{
	workload_tt workload;              /**< Input workload.           */
	int nthreads;                      /**< Number of threads.        */
	const struct scheduler *scheduler; /**< Loop scheduling strategy. */
} args = { NULL, 0, NULL };

/*============================================================================*
 * ARGUMENT CHECKING                                                          *
 *============================================================================*/

/**
 * @brief Prints program usage and exits.
 */
static void usage(void)
{
	printf("Usage: simsched [options] <scheduler>\n");
	printf("Brief: loop scheduler simulator\n");
	printf("Options:\n");
	printf("  --input <filename>    Input workload file\n");
	printf("  --nthreads <number>   Number of threads\n");
	printf("  --help                Display this message\n");
	printf("Loop Schedulers:\n");
	printf("  dynamic  Dynamic Scheduling\n");
	printf("  lpt      Longest Processing Time First Scheduling\n");
	printf("  srr      Smart Round-Robin Scheduling\n");
	printf("  static   Static Scheduling\n");

	exit(EXIT_SUCCESS);
}

/**
 * @brief Gets workload.
 *
 * @param filename Input workload filename.
 *
 * @returns A workload.
 */
static workload_tt get_workload(const char *filename)
{
	FILE *input;   /* Input workload file. */
	workload_tt w; /* Workload.            */

	input = fopen(filename, "r");
	if (input == NULL)
		error("cannot open input workload file");

	w = workload_read(input);

	fclose(input);

	return (w);
}

/**
 * @brief Checks program arguments.
 *
 * @param filename Input workload filename.
 */
static void checkargs(const char *filename)
{
	if (filename == NULL)
		error("missing input workload file");
	if (args.scheduler == NULL)
		error("missing loop scheduling strategy");
	if (args.nthreads <= 1)
		error("invalid number of threads");
}

/**
 * @brief Reads command line arguments.
 *
 * @param argc Argument count.
 * @param argv Argument variables.
 */
static void readargs(int argc, const char **argv)
{
	const char *filename = NULL;

	/* Parse command line arguments. */
	for (int i = 1; i < argc; i++)
	{	
		if (!strcmp(argv[i], "--input"))
			filename = argv[++i];
		else if (!strcmp(argv[i], "--nthreads"))
			args.nthreads = atoi(argv[++i]);
		else if (!strcmp(argv[i], "--help"))
			usage();
		else
		{
			if (!strcmp(argv[i], "dynamic"))
				args.scheduler = sched_dynamic;
			else if (!strcmp(argv[i], "lpt"))
				args.scheduler = sched_lpt;
			else if (!strcmp(argv[i], "srr"))
				args.scheduler = NULL;
			else if (!strcmp(argv[i], "static"))
				args.scheduler = sched_static;
			else
				error("unsupported loop scheduling strategy");
		}
	}

	checkargs(filename);

	args.workload = get_workload(filename);
}

/*============================================================================*
 * LOOP SCHEDULER SIMULATOR                                                   *
 *============================================================================*/

/**
 * @brief A loop scheduler simulator
 */
int main(int argc, const const char **argv)
{
	readargs(argc, argv);

	simshed(args.workload, args.nthreads, args.scheduler);

	/* House keeping, */
	workload_destroy(args.workload);

	return (EXIT_SUCCESS);
}
