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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <mylib/util.h>
#include <common.h>
#include "searcher.h"

/**
 * @name Searching Parameters
 */
/**@{*/
unsigned nthreads = 0;            /**< Number of threads.        */
unsigned ntasks = 0;              /**< Number of tasks.          */
static unsigned distribution = 0; /**< Probability distribution. */
static unsigned ngen = 0;         /**< Number of generations.    */
static unsigned popsize = 0;      /**< Population size.          */
/**@}*/


/**
 * @brief Input data file.
 */
static const char *infile = NULL;

/**
 * @brief Prints program usage and exits.
 * 
 * @details Prints program usage and exits gracefully.
 */
static void usage(void)
{
	printf("Usage: searcher [options]\n");
	printf("Brief: searches for a good loop scheduling\n");
	printf("Options:\n");
	printf("  --nthreads <num>       Number of threads\n");
	printf("  --ntasks <num>         Number of tasks\n");
	printf("  --distribution <name>  Input probability density function\n");
	printf("  --input <filename>     Use input file as input data");
	printf("  --ngen <num>           Number of generations\n");
	printf("  --popsize <num>        Population size\n");
	printf("  --help                 Display this message\n");

	exit(EXIT_SUCCESS);
}

/**
 * @brief Reads command line arguments.
 * 
 * @details Reads command line arguments.
 */
static void readargs(int argc, const char **argv)
{
	enum states{
		STATE_READ_ARG,         /* Read argument.             */
		STATE_SET_NTHREADS,     /* Set number of threads.     */
		STATE_SET_NTASKS,       /* Set number of tasks.       */
		STATE_SET_DISTRIBUTION, /* Set distribution.          */
		STATE_SET_NGEN,         /* Set number of generations. */
		STATE_SET_POPSIZE,      /* Population size.           */
		STATE_SET_INPUT};       /* Set input data.            */
	
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
					break;
				
				case STATE_SET_NTASKS:
					ntasks = atoi(arg);
					break;
				
				case STATE_SET_DISTRIBUTION:
					distribution_name = arg;
					break;
					
				case STATE_SET_NGEN:
					ngen = atoi(arg);
					break;
				
				case STATE_SET_POPSIZE:
					popsize = atoi(arg);
					break;
					
				case STATE_SET_INPUT:
					infile = arg;
					break;
			}
			
			state = STATE_READ_ARG;
			
			continue;
		}
		
		/* Parse command. */
		if (!strcmp(arg, "--nthreads"))
			state = STATE_SET_NTHREADS;
		else if (!strcmp(arg, "--ntasks"))
			state = STATE_SET_NTASKS;
		else if (!strcmp(arg, "--distribution"))
			state = STATE_SET_DISTRIBUTION;
		else if (!strcmp(arg, "--ngen"))
			state = STATE_SET_NGEN;
		else if (!strcmp(arg, "--popsize"))
			state = STATE_SET_POPSIZE;
		else if (!strcmp(arg, "--input"))
			state = STATE_SET_INPUT;
		else if (!strcmp(arg, "--help"))
			usage();
	}
	
	/* Check parameters. */
	if (nthreads == 0)
		error("invalid number of threads");
	else if (ntasks == 0)
		error("invalid number of tasks");
	else if (ngen == 0)
		error("invalid number of generations");
	else if (popsize == 0)
		error("invalid population size");
	if (distribution_name == NULL)
	{
		if (infile == NULL)
			error("invalid input file");
	}
	else
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
 * @brief Reads input file
 * 
 * @param infile Input filename.
 */
static unsigned *readfile(const char *infile)
{
	FILE *fp;
	unsigned *tasks;
	
	tasks = smalloc(ntasks*sizeof(unsigned));
	
	fp = fopen(infile, "r");
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

/**
 * @brief Searches for a good loop scheduling.
 */
int main(int argc, const const char **argv)
{
	unsigned *tasks;
	
	readargs(argc, argv);
	
	srandnum(time(NULL));
	
	tasks = (infile != NULL) ?
		readfile(infile) : create_tasks(distribution, ntasks);
	
	ga(tasks, popsize, ngen);
		
	/* House keeping. */
	free(tasks);
	
	return (EXIT_SUCCESS);
}
