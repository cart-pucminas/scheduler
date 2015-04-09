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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <mylib/util.h>

#include "simulator.h"

/**
 * @name Random Distribution Parameters
 */
/**@{*/
#define RAND_MAX_SIZE 8192 /**< Maximum task size. */
/**@}*/

/**
 * @name Simulation Parameters
 */
/**@{*/
static unsigned nthreads = 32;              /**< Number of threads.        */
static unsigned ntasks = 1024;              /**< Number of tasks.          */
static unsigned distribution = 0;           /**< Probability distribution. */
static unsigned scheduler = SCHEDULER_NONE; /**< Loop scheduler.           */
/**@}*/

/**
 * @brief Number of supported probability distributions.
 */
#define NDISTRIBUTIONS 2

/**
 * @brief Supported probability distributions.
 */
static char *distributions[NDISTRIBUTIONS] = {
	"random", /* Random. */
	"normal"  /* Normal. */
};

/**
 * @brief Prints program usage and exits.
 * 
 * @details Prints program usage and exits gracefully.
 */
static void usage(void)
{
	printf("Usage: 	scheduler [options] <scheduler>\n");
	printf("Brief: loop scheduler simulator\n");
	printf("Scheduler:\n");
	printf("  static         Simulate static loop scheduling");
	printf("  dynamic        Simulate dynamic loop scheduling");
	printf("  workload-aware Simulate workload-aware loop scheduling");
	printf("Options:\n");
	printf("  --nthreads <num>       Number of threads\n");
	printf("  --ntasks <num>         Number of tasks\n");
	printf("  --distribution <name>  Probability density function\n");
	printf("  --help                 Display this message\n");
}

/**
 * @brief Reads command line arguments.
 * 
 * @details Reads command line arguments.
 */
static void readargs(int argc, const char **argv)
{
	enum states{
		STATE_READ_ARG,          /* Read argument.         */
		STATE_SET_NTHREADS,      /* Set number of threads. */
		STATE_SET_NTASKS,        /* Set number of tasks.   */
		STATE_SET_DISTRIBUTION}; /* Set distribution.      */
	
	unsigned state;                /* Current state.     */
	const char *distribution_name; /* Distribution name. */
	
	state = STATE_READ_ARG;
	distribution_name = NULL;
	
	/* Parse command line arguments. */
	for (int i = 1; i < argc; i++)
	{
		const char *arg = argv[i];
		
		/* Set value. */
		if (state != STATE_READ_ARG)
		{
			switch (state)
			{
				case STATE_SET_NTHREADS:
					nthreads = atoi(arg);
					state = STATE_READ_ARG;
					break;
				
				case STATE_SET_NTASKS:
					ntasks = atoi(arg);
					state = STATE_READ_ARG;
					break;
				
				case STATE_SET_DISTRIBUTION:
					distribution_name = arg;
					state = STATE_READ_ARG;
					break;
			}
			
			continue;
		}
		
		/* Parse command. */
		if (!strcmp(arg, "--nthreads"))
			state = STATE_SET_NTHREADS;
		else if (!strcmp(arg, "--ntasks"))
			state = STATE_SET_NTASKS;
		else if (!strcmp(arg, "--distribution"))
			state = STATE_SET_DISTRIBUTION;
		else if (!strcmp(arg, "--help"))
			state = STATE_SET_DISTRIBUTION;
		else if (!strcmp(arg, "--distribution"))
			usage();
		else if (!strcmp(arg, "static"))
			scheduler = SCHEDULER_STATIC;
		else if (!strcmp(arg, "dynamic"))
			scheduler = SCHEDULER_DYNAMIC;
		else if (!strcmp(arg, "workload-aware"))
			scheduler = SCHEDULER_WORKLOAD_AWARE;
	}
	
	/* Check parameters. */
	if (nthreads == 0)
		error("invalid number of threads");
	else if (ntasks == 0)
		error("invalid number of tasks");
	else if (scheduler == SCHEDULER_NONE)
		error("invalid scheduler");
	if (distribution_name != NULL)
	{
		for (unsigned i = 0; i < NDISTRIBUTIONS; i++)
		{
			if (!strcmp(distribution_name, distributions[i]))
			{
				distribution = i;
				goto out;
			}
		}
		error("unknown distribution");
	}

out:
	return;
}

/**
 * @brief Loop scheduler simulator.
 */
int main(int argc, const const char **argv)
{
	unsigned *tasks;
	
	readargs(argc, argv);

	/* Create tasks. */
	info("creating tasks...", VERBOSE_INFO);
	tasks = smalloc(ntasks*sizeof(unsigned));
	for (unsigned i = 0; i < ntasks; i++)
		tasks[i] = randnum()%RAND_MAX_SIZE + 1;
		
	schedule(tasks, ntasks, nthreads, scheduler);
	
	return (EXIT_SUCCESS);
}
