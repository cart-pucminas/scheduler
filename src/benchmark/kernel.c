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

#include <omp.h>
#include <stdbool.h>

#include <common.h>

extern bool use_floating_point;
extern unsigned chunksize;

/**
 * @brief Performs some integer precision work.
 */
static void worki(double foo)
{
	unsigned bar = 0.0;
	unsigned n = (unsigned) 100*foo;
	
	for (unsigned i = 0; i < n; i++)
		bar = bar + 1;
	
}

/**
 * @brief Performs some floating-point precision work.
 */
static void workf(double foo)
{
	double bar = 0.0;
	unsigned n = 100*foo;
	
	for (unsigned i = 0; i < n; i++)
		bar = bar + 1.0;
}


/**
 * @brief Simulates a synthetic numeric kernel.
 */
void kernel
(const double *tasks, unsigned ntasks, unsigned nthreads, unsigned scheduler)
{
	void (*work)(double);
	
	work = (use_floating_point) ? workf : worki;

	/* Dynamic scheduler. */
	if (scheduler == SCHEDULER_STATIC)
	{
		#pragma omp parallel for schedule(dynamic, chunksize) num_threads(nthreads)
		for (unsigned i = 0; i < ntasks; i++)
			work(tasks[i]);
	}
	
	/* Static scheduler. */
	else
	{
		#pragma omp parallel for schedule(static, chunksize) num_threads(nthreads)
		for (unsigned i = 0; i < ntasks; i++)
			work(tasks[i]);
	}
}
