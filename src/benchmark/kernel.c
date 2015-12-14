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
#include <stdlib.h>
#include <stdint.h>

#include <common.h>
#include <mylib/util.h>

#include "benchmark.h"

/* Workloads. */
unsigned *__tasks; /* Tasks.           */
unsigned __ntasks; /* Number of tasks. */

/**
 * @brief Benchmark kernel.
 */
void kernel(int n, int load)
{
	int sum = 0;
	
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < load; j++)
			sum++;		
	}
}

/**
 * @brief Synthetic benchmark. 
 */
void benchmark(
	const unsigned *tasks,
	unsigned ntasks,
	unsigned niterations,
	unsigned nthreads,
	unsigned load,
	unsigned scheduler)
{
	uint64_t end;
	uint64_t start;
	
	start = get_time();
	
	for (unsigned k = 0; k < niterations; k++)
	{	
		/* Dynamic scheduler. */
		if (scheduler == SCHEDULER_DYNAMIC)
		{
			#pragma omp parallel for schedule(dynamic) num_threads(nthreads)
			for (unsigned i = 0; i < ntasks; i++)
				kernel(tasks[i], load);
		}
		
		/* Smart round-robin scheduler. */
		else if (scheduler == SCHEDULER_SMART_ROUND_ROBIN)
		{		
			__ntasks = ntasks;
			__tasks = smalloc(ntasks*sizeof(unsigned));
			memcpy(__tasks, tasks, ntasks*sizeof(unsigned));
			
			#pragma omp parallel for schedule(runtime) num_threads(nthreads)
			for (unsigned i = 0; i < ntasks; i++)
				kernel(tasks[i], load);
			
			free(__tasks);
		}
		
		/* Static scheduler. */
		else
		{
			#pragma omp parallel for schedule(static) num_threads(nthreads)
			for (unsigned i = 0; i < ntasks; i++)
				kernel(tasks[i], load);
		}
	}
	
	end = get_time();
	
	printf("%.2lf\n", (end - start)/1000.0);
}
