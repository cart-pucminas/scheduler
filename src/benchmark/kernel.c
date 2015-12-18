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
	double time[nthreads];
	double total_time = 0;
	double elapsed_time = 0;

	for (unsigned k = 0; k < niterations; k++)
	{	
		/* Dynamic scheduler. */
		if (scheduler == SCHEDULER_DYNAMIC)
		{
			#pragma omp parallel num_threads(nthreads)
			{
				uint64_t end;
				uint64_t start;

				start = timer_get();

				#pragma omp for schedule(dynamic) nowait
				for (unsigned i = 0; i < ntasks; i++)
					kernel(tasks[i], load);
	
				end = timer_get();
				time[omp_get_thread_num()] += (end - start)/1000.0;
			}
		}
		
		/* Smart round-robin scheduler. */
		else if (scheduler == SCHEDULER_SMART_ROUND_ROBIN)
		{		
			__ntasks = ntasks;
			__tasks = smalloc(ntasks*sizeof(unsigned));
			memcpy(__tasks, tasks, ntasks*sizeof(unsigned));
			
			#pragma omp parallel num_threads(nthreads)
			{
				uint64_t end;
				uint64_t start;

				start = timer_get();

				#pragma omp for schedule(runtime) nowait
				for (unsigned i = 0; i < ntasks; i++)
					kernel(tasks[i], load);
	
				end = timer_get();
				time[omp_get_thread_num()] += (end - start)/1000.0;
			}
			free(__tasks);
		}
		
		/* Static scheduler. */
		else
		{
			#pragma omp parallel num_threads(nthreads)
			{
				uint64_t end;
				uint64_t start;

				start = timer_get();

				#pragma omp for schedule(static) nowait
				for (unsigned i = 0; i < ntasks; i++)
					kernel(tasks[i], load);
	
				end = timer_get();
				time[omp_get_thread_num()] += (end - start)/1000.0;
			}
		}
	}

	/* Print statistics. */
	for (unsigned i = 0; i < nthreads; i++)
	{
		elapsed_time = (time[i] > elapsed_time) ? time[i] : elapsed_time;
		total_time += time[i];
		printf("thread %d: %.2lf\n", i, time[i]);
	}
	printf("%.2lf %.2lf\n", total_time, elapsed_time);
}
