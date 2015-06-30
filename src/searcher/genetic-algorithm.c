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

#include <string.h>
#include <limits.h>

#include <mylib/ai.h>
#include <mylib/util.h>

#include "searcher.h"

/**
 * @brief Casts a gene.
 */
#define GENE(x) ((unsigned *) (x))

/**
 * @brief Tasks.
 */
static const double *tasks = NULL;

/**
 * @brief Destroys a gene.
 * 
 * @param gen Gene.
 */
static void gene_destroy(gene_t gen)
{
	free(gen);
}

/**
 * @brief Generates a random gene.
 * 
 * @returns A random gene.
 */
static gene_t gene_generate(void)
{
	unsigned *gen;
	
	gen = smalloc(ntasks*sizeof(unsigned));
	
	/* Generate gene. */
	for (unsigned i = 0; i < ntasks; i++)
		gen[i] = randnum()%nthreads;
	
	return (gen);	
}

/**
 * @brief Evaluates a gene.
 * 
 * @param gen Gene to evaluate.
 * 
 * @returns Fitness of the gene.
 */
static double gene_evaluate(gene_t gen)
{
	double min, max;  /* max and min workloads. */
	double *workload; /* Per-thred workload.    */
	
	workload = scalloc(nthreads, sizeof(double));
	
	/* Compute workload of threads. */
	for (unsigned i = 0; i < ntasks; i++)
	{
		unsigned tid;
		
		tid = GENE(gen)[i];
		
		workload[tid] += tasks[i];
	}
	
	/* Compute load imbalance. */
	max = 0;
	min = UINT_MAX;
	for (unsigned i = 0; i < nthreads; i++)
	{
		if (workload[i] < min)
			min = workload[i];
		else if (workload[i] > max)
			max = workload[i];
	}
	
	/* House keeping. */
	free(workload);
	
	return (1.0/(max - min));
}

/**
 * @brief Crossover two genes.
 * 
 * @param gen0 First gene.
 * @param gen1 Second gene.
 * @param child Child to generate.
 * 
 * @returns Child gene.
 */
static gene_t gene_crossover(gene_t gen0, gene_t gen1, int child)
{
	unsigned *gen;                  /* Child gene.      */
	static unsigned crosspoint = 0; /* Crossover point. */
	
	gen = smalloc(ntasks*sizeof(unsigned));
	
	/* Generate crossover point. */
	if (crosspoint == 0)
	{
		do
		{
			crosspoint = randnum()%ntasks;
		} while ((crosspoint == 0) || (crosspoint == (ntasks - 1)));
	}
	
	/* First child. */
	if (child == 0)
	{
		memcpy(&gen[0], &GENE(gen0)[0], crosspoint*sizeof(unsigned));
		memcpy(&gen[crosspoint], &GENE(gen1)[crosspoint], 
										(ntasks - crosspoint)*sizeof(unsigned));
	}
	
	/* Second child. */
	else
	{
		memcpy(&gen[0], &GENE(gen1)[0], crosspoint*sizeof(unsigned));
		memcpy(&gen[crosspoint], &GENE(gen0)[crosspoint],
										(ntasks - crosspoint)*sizeof(unsigned));
		
		/* Reset crossover point. */
		crosspoint = 0;
	}
	
	return (gen);
}

/**
 * @brief Mutates a gene.
 * 
 * @param gen Gene.
 * 
 * @returns Mutated gene
 */
static gene_t gene_mutate(gene_t gen)
{
	GENE(gen)[randnum()%ntasks] = randnum()%nthreads;
	
	return (gen);
}

/**
 * @brief Genetic algorithm problem.
 */
static struct genome problem = {
	0.10, /* Mutation rate.    */
	0.80, /* Crossover rate.   */
	0.01, /* Elitism rate.     */
	0.90, /* Replacement rate. */
	&gene_generate,  /* generate()  */
	&gene_evaluate,  /* evaluate()  */
	&gene_crossover, /* crossover() */
	&gene_mutate,    /* mutation()  */
	&gene_destroy    /* destroy()   */
};

/**
 * @brief Genetic algorithm for searching a good loop scheduling.
 */
void ga(const double *_tasks, unsigned popsize, unsigned ngen)
{
	gene_t bestgen;
	double total;
	double *workload;
	
	tasks = _tasks;
	workload = scalloc(nthreads, sizeof(double));
	
	bestgen = genetic_algorithm(&problem, popsize, ngen,
		GA_OPTIONS_STATISTICS | GA_OPTIONS_CONVERGE);
	
	/* Print statistics. */
	total = 0.0;
	for (unsigned i = 0; i < ntasks; i++)
	{
		total += tasks[i];
		workload[GENE(bestgen)[i]] += tasks[i];
	}
	for (unsigned i = 0; i < nthreads; i++)
		fprintf(stderr, "%u;%lf\n", i, 100*workload[i]/total);
	
	/* House keeping. */
	free(workload);
	free(bestgen);
}
