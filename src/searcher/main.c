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
#include "searcher.h"

/**
 * @name Program arguments.
 */
/**@{*/
struct
{
	/**
	 * @name Synthetic Workload Arguments
	 */
	/**@{*/
	const char *infile;  /**< Input data file.                           */
	unsigned ntasks;     /**< Number of tasks.                           */
	unsigned nthreads;   /**< Number of threads.                         */
	/**@}*/
	
	/**
	 * @name Genetic Algorithm Arguments
	 */
	/**@{*/
	double crossover;   /**< Crossover rate.        */
	double elitism;     /**< Elitism rate.          */
	double mutation;    /**< Mutation rate.         */
	unsigned ngen;      /**< Number of generations. */
	unsigned popsize;   /**< Population size.       */
	double replacement; /**< Replacement rate.      */
	/**@}*/
} args = {
	NULL, 0, 0,
	0.0, 0.0, 0.0, 0, 0, 0.0
};

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
	printf("  Synthetic Application\n");
	printf("    --input <filename>     Input workload file\n");
	printf("    --ntasks <num>         Number of tasks\n");
	printf("    --nthreads <num>       Number of threads\n");
	printf("  Genetic Algorithm\n");
	printf("    --crossover <num>      Crossover rate\n");
	printf("    --elitism <num>        Elitism rate\n");
	printf("    --mutation <num>       Mutation rate\n");
	printf("    --ngen <num>           Number of generations\n");
	printf("    --popsize <num>        Population size\n");
	printf("    --replacement <num>    Replacement rate\n");
	printf("  Other Options\n");
	printf("    --help                 Display this message\n");

	exit(EXIT_SUCCESS);
}

/**
 * @brief Checks program arguments.
 * 
 * @details Checks program arguments and terminates execution if they are not 
 *          valid.
 */
static void checkargs(void)
{
	if (args.infile == NULL)
		printf("missing input file");
	
	if (args.ntasks == 0)
		error("invalid number of tasks");
		
	if (args.nthreads == 0)
		error("invalid number of threads");
	
	if ((args.crossover <= 0.0) || (args.crossover >= 1.0))
		error("invalid crossover rate");
	
	if ((args.elitism <= 0.0) || (args.elitism >= 1.0))
		error("invalid elitism rate");
	
	if ((args.mutation <= 0.0) || (args.mutation >= 1.0))
		error("invalid mutation rate");
	
	if (args.ngen == 0)
		error("invalid number of generations");
		
	if (args.popsize == 0)
		error("invalid population size");
	
	if ((args.replacement <= 0.0) || (args.replacement >= 1.0))
		error("invalid replacement rate");
}

/**
 * @brief Reads command line arguments.
 * 
 * @details Reads command line arguments.
 */
static void readargs(int argc, const char **argv)
{
	enum states{
		STATE_READ_ARG,       /* Read argument.                    */
		STATE_SET_PDF,        /* Set probability density function. */
		STATE_SET_INFILE,     /* Set input data file.              */
		STATE_SET_NTASKS,     /* Set number of tasks.              */
		STATE_SET_NTHREADS,   /* Set number of threads.            */
		STATE_SET_CROSSOVER,  /* Set crossover rate.               */
		STATE_SET_ELITISM,    /* Set elitism rate.                 */
		STATE_SET_MUTATION,   /* Set mutation rate.                */
		STATE_SET_NGEN,       /* Set number of generations.        */
		STATE_SET_POPSIZE,    /* Set population size.              */
		STATE_SET_REPLACEMENT /* Set replacement rate.             */
	};
	
	unsigned state;
	
	state = STATE_READ_ARG;
	
	/* Parse command line arguments. */
	for (int i = 1; i < argc; i++)
	{
		const char *arg = argv[i];
		
		/* Set value. */
		if (state != STATE_READ_ARG)
		{
			switch (state)
			{
				case STATE_SET_INFILE:
					args.infile = arg;
					break;
					
				case STATE_SET_NTASKS:
					args.ntasks = atoi(arg);
					break;
					
				case STATE_SET_NTHREADS:
					args.nthreads = atoi(arg);
					break;
				
				case STATE_SET_CROSSOVER:
					args.crossover = atof(arg);
					break;
				
				case STATE_SET_ELITISM:
					args.elitism = atof(arg);
					break;
				
				case STATE_SET_MUTATION:
					args.mutation = atof(arg);
					break;
					
				case STATE_SET_NGEN:
					args.ngen = atoi(arg);
					break;
				
				case STATE_SET_POPSIZE:
					args.popsize = atoi(arg);
					break;
				
				case STATE_SET_REPLACEMENT:
					args.replacement = atof(arg);
					break;
			}
			
			state = STATE_READ_ARG;
			
			continue;
		}
		
		/* Parse command. */
		if (!strcmp(arg, "--input"))
			state = STATE_SET_INFILE;
		else if (!strcmp(arg, "--ntasks"))
			state = STATE_SET_NTASKS;
		else if (!strcmp(arg, "--nthreads"))
			state = STATE_SET_NTHREADS;
		else if (!strcmp(arg, "--pdf"))
			state = STATE_SET_PDF;
		else if (!strcmp(arg, "--crossover"))
			state = STATE_SET_CROSSOVER;
		else if (!strcmp(arg, "--elitism"))
			state = STATE_SET_ELITISM;
		else if (!strcmp(arg, "--mutation"))
			state = STATE_SET_MUTATION;
		else if (!strcmp(arg, "--ngen"))
			state = STATE_SET_NGEN;
		else if (!strcmp(arg, "--popsize"))
			state = STATE_SET_POPSIZE;
		else if (!strcmp(arg, "--replacement"))
			state = STATE_SET_REPLACEMENT;
		else
			usage();
	}
	
	checkargs();
}

/**
 * @brief Reads input file
 * 
 * @param infile Input filename.
 * @param ntasks Number of tasks.
 */
static unsigned *readfile(const char *infile, unsigned ntasks)
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
	
	srandnum(0);
	
	tasks = readfile(args.infile, args.ntasks);
	
	ga(tasks,
	   args.ntasks,
	   args.nthreads,
	   args.popsize,
	   args.ngen,
	   args.crossover,
	   args.elitism,
	   args.mutation,
	   args.replacement);
		
	/* House keeping. */
	free(tasks);
	
	return (EXIT_SUCCESS);
}
