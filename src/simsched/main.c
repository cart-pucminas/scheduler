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
	array_tt  threads;                 /**< Working threads.          */
	const struct scheduler *scheduler; /**< Loop scheduling strategy. */
} args = { NULL, 0, NULL, NULL };

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
	printf("  --arch <filename>     Architecture file\n");
	printf("  --input <filename>    Input workload file\n");
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
 * @brief Gets threads.
 *
 * @param filename Architecture filename.
 *
 * @returns Working threads.
 */
static array_tt get_threads(const char *filename)
{
	FILE *file;       /* Architecture file.         */
	int nthreads;     /* Number of working threads. */
	array_tt threads; /* Working threads.           */

	if ((file = fopen(filename, "r")) == NULL)
		error("failed to open architecture file");

	assert(fscanf(file, "%d", &nthreads) == 1);
	if (nthreads < 1)
		error("bad architecture file");

	threads = array_create(nthreads);

	for (int i = 0; i < nthreads; i++)
	{
		thread_tt t;  /* Thread.                       */
		int capacity; /* Thread's processing capacity. */
		
		assert(fscanf(file, "%d", &capacity) == 1);

		t = thread_create(capacity);
		array_set(threads, i, t);
	}

	/* House keeping. */
	fclose(file);

	return (threads);
}

/**
 * @brief Checks program arguments.
 *
 * @param wfilename Input workload filename.
 * @param afilename Input architecture filename.
 */
static void checkargs(const char *wfilename, const char *afilename)
{
	if (afilename == NULL)
		error("missing architecture file");
	if (wfilename == NULL)
		error("missing input workload file");
	if (args.scheduler == NULL)
		error("missing loop scheduling strategy");
}

/**
 * @brief Reads command line arguments.
 *
 * @param argc Argument count.
 * @param argv Argument variables.
 */
static void readargs(int argc, const char **argv)
{
	const char *wfilename = NULL;
	const char *afilename = NULL;

	/* Parse command line arguments. */
	for (int i = 1; i < argc; i++)
	{	
		if (!strcmp(argv[i], "--arch"))
			afilename = argv[++i];
		else if (!strcmp(argv[i], "--input"))
			wfilename = argv[++i];
		else if (!strcmp(argv[i], "--help"))
			usage();
		else
		{
			if (!strcmp(argv[i], "dynamic"))
				args.scheduler = sched_dynamic;
			else if (!strcmp(argv[i], "lpt"))
				args.scheduler = sched_lpt;
			else if (!strcmp(argv[i], "srr"))
				args.scheduler = sched_srr;
			else if (!strcmp(argv[i], "static"))
				args.scheduler = sched_static;
			else
				error("unsupported loop scheduling strategy");
		}
	}

	checkargs(wfilename, afilename);

	args.workload = get_workload(wfilename);
	args.threads = get_threads(afilename);
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

	simshed(args.workload, args.threads, args.scheduler);

	/* House keeping, */
	for (int i = 0; i < array_size(args.threads); i++)
	{
		thread_tt t = array_get(args.threads, i);
		thread_destroy(t);
	}
	array_destroy(args.threads);
	workload_destroy(args.workload);

	return (EXIT_SUCCESS);
}
