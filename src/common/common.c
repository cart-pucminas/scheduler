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

/**
 * @brief Supported probability distributions.
 */
const char *distributions[NDISTRIBUTIONS] = {
	"random",  /* Random.  */
	"normal",  /* Normal.  */
	"poisson", /* Poisson. */
	"gamma"    /* Gammma.  */
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
	return ((*(double *)a) - (*(double *)b));
}

#endif

/**
 * @brief Generates tasks.
 */
double *create_tasks(unsigned distribution, unsigned ntasks)
{
	double *tasks;
	
	/* Create tasks. */
	tasks = smalloc(ntasks*sizeof(double));
	switch (distribution)
	{
		/* Random distribution. */
		case DISTRIBUTION_RANDOM:
		{
			for (unsigned i = 0; i < ntasks; i++)
				tasks[i] = randnum()%ntasks;
		} break;
		
		/* Normal distribution. */
		case DISTRIBUTION_NORMAL:
		{
			for (unsigned i = 0; i < ntasks; i++)
			{
				double num;
				
				do
				{
					num = normalnum(32.0, 1.0);
				} while (num < 0.0);
				
				tasks[i] = num;
			}
		} break;
		
		/* Poisson Distribution. */
		case DISTRIBUTION_POISSON:
		{
			for (unsigned i = 0; i < ntasks; i++)
			{
				double num;
				
				do
				{
					num = poissonnum(8.0);
				} while (num < 0.0);
				
				tasks[i] = num;
			}
		} break;
		
		/* Gamma Distribution. */
		case DISTRIBUTION_GAMMA:
		{
			gsl_rng * r;
			const gsl_rng_type * T;
			
			gsl_rng_env_setup();
			
			T = gsl_rng_default;
			r = gsl_rng_alloc (T);
			
			for (unsigned i = 0; i < ntasks; i++)
			{
				double num;
				do
				{
					num = gsl_ran_gamma(r, 0.5, 1.0);
				} while (num < 0.0);
				
				tasks[i] = num;
			}
			
			gsl_rng_free(r);
		} break;
	}
		
#ifdef _SORT_	
		qsort(tasks, ntasks, sizeof(double), cmp);
#endif
	
	return (tasks);
}
