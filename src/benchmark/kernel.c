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
#include <string.h>

#include <common.h>
#include <mylib/util.h>

#include "benchmark.h"

/* Workloads. */
unsigned *__tasks; /* Tasks.           */
unsigned __ntasks; /* Number of tasks. */


/**
 * @brief Performs some integer precision work.
 */
static void work(unsigned n)
{
	(void)n;
}


/**
 * @brief Simulates a synthetic numeric kernel.
 */
void kernel
(const unsigned *tasks, unsigned ntasks, unsigned nthreads, unsigned scheduler)
{	
	(void) nthreads;

	/* Dynamic scheduler. */
	if (scheduler == SCHEDULER_STATIC)
	{
		#pragma omp parallel for schedule(dynamic, chunksize)
		for (unsigned i = 0; i < ntasks; i++)
			work(tasks[i]);
	}
	
	/* Smart round-robin scheduler. */
	else if (scheduler == SCHEDULER_SMART_ROUND_ROBIN)
	{
		__ntasks = ntasks;
		__tasks = smalloc(ntasks*sizeof(unsigned));
		memcpy(__tasks, tasks, ntasks*sizeof(unsigned));
		
		#pragma omp parallel for schedule(runtime)
		for (unsigned i = 0; i < ntasks; i++)
			work(tasks[i]);
		
		free(__tasks);
	}
	
	/* Static scheduler. */
	else
	{
		#pragma omp parallel for schedule(static, chunksize)
		for (unsigned i = 0; i < ntasks; i++)
			work(tasks[i]);
	}
}
