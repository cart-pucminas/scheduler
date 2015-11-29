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

#include <stdlib.h>

#include <mylib/util.h>
#include <common.h>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#define _SORT_

/**
 * @brief Multiplying factor.
 */
#define FACTOR 1000.0

/**
 * @name Flat Distribution Parameters
 */
/**@{*/
#define FLAT_MIN   0.0
#define FLAT_MAX 512.0
/**@}*/

/**
 * @name Gaussian Distribution Parameters
 */
/**@{*/
#define GUASSIAN_STDDEV  1.0
#define GUASSIAN_MEAN   32.0
/**@}*/

/**
 * @name Poisson Distribution Parameters
 */
/**@{*/
#define POISSON_MU 4.0
/**@}*/

/**
 * @name Gamma Distribution Parameters
 */
/**@{*/
#define GAMMA_A 1.0
#define GAMMA_B 2.0
/**@}*/

/**
 * @name Beta Distribution Parameters
 */
/**@{*/
#define BETA_A   0.5
#define BETA_B   0.5
#define BETA_M 512.0
/**@}*/

/**
 * @brief Supported probability distributions.
 */
const char *distributions[NDISTRIBUTIONS] = {
	"random",  /* Random.  */
	"normal",  /* Normal.  */
	"poisson", /* Poisson. */
	"gamma",   /* Gammma.  */
	"beta"     /* Beta.    */
};

#ifdef _SORT_

/**
 * @brief Compares two unsigned integers.
 * 
 * @details Compares the unsigned integer pointed to by @p a with the unsigned
 *          integer pointed to by @p b.
 * 
 * @returns The difference between @p a and @p b.
 */
static int cmp(const void *a, const void *b)
{
	return ((*(unsigned *)a) - (*(unsigned *)b));
}

#endif

/**
 * @brief Generates tasks.
 */
unsigned *create_tasks(unsigned distribution, unsigned ntasks)
{
	unsigned *tasks;
	gsl_rng * r;
	const gsl_rng_type * T;
	
	/* Setup random number generator. */
	gsl_rng_env_setup();
	T = gsl_rng_default;
	r = gsl_rng_alloc(T);
	
	/* Create tasks. */
	tasks = smalloc(ntasks*sizeof(unsigned));
	switch (distribution)
	{
		/* Random distribution. */
		case DISTRIBUTION_RANDOM:
		{
			for (unsigned i = 0; i < ntasks; i++)
				tasks[i] = (unsigned)(gsl_ran_flat(r, FLAT_MIN, FLAT_MAX)*FACTOR);
		} break;
		
		/* Normal distribution. */
		case DISTRIBUTION_NORMAL:
		{
			for (unsigned i = 0; i < ntasks; i++)
			{
				double num;
				
				do
				{
					num = gsl_ran_gaussian(r, GUASSIAN_STDDEV) + GUASSIAN_MEAN;
				} while (num < 0.0);
				
				tasks[i] = (unsigned)(num*FACTOR);
			}
		} break;
		
		/* Poisson Distribution. */
		case DISTRIBUTION_POISSON:
		{
			for (unsigned i = 0; i < ntasks; i++)
				tasks[i] = (unsigned)(gsl_ran_poisson(r, POISSON_MU)*FACTOR);
		} break;
		
		/* Gamma distribution. */
		case DISTRIBUTION_GAMMA:
		{
			for (unsigned i = 0; i < ntasks; i++)
			{
				double num;
				do
				{
					num = gsl_ran_gamma(r, GAMMA_A, GAMMA_B);
				} while (num < 0.0);
				tasks[i] = (unsigned)(ntasks*tasks[i]);
			}
		} break;
		
		/* Beta distribution. */
		case DISTRIBUTION_BETA:
		{
			for (unsigned i = 0; i < ntasks; i++)
				tasks[i] = (unsigned)(gsl_ran_beta(r, BETA_A, BETA_B)*BETA_M*FACTOR);
		} break;
	}
		
#ifdef _SORT_	
		qsort(tasks, ntasks, sizeof(unsigned), cmp);
#endif
	
	/* House keeping. */		
	gsl_rng_free(r);
	
	return (tasks);
}
