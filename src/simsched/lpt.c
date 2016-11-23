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
#include <string.h>
#include <math.h>

#include <mylib/util.h>

#include <simulator.h>

/**
 * @brief Best scheduler data.
 */
static struct
{
	unsigned ntasks;       /**< Number of tasks.               */
	unsigned nthreads;     /**< Number of threads.             */
	const unsigned *tasks; /**< Tasks.                         */
	unsigned *taskmap;     /**< Task map.             */
} scheduler_data = {
	0,
	0,
	NULL,
	NULL,
};

/**
 * @brief Sorts the workload.
 * 
 * @param a Workload
 * @param n Number of tasks.
 * 
 * @returns Sorting map.
 */
static unsigned *sort(unsigned *a, unsigned n)
{
	unsigned *map;
	
	map = smalloc(n*sizeof(unsigned));
	for (unsigned i = 0; i < n; i++)
		map[i] = i;
		
	/* Sort. */
	for (unsigned i = 0; i < n - 1; i++)
	{
		for (unsigned j = i + 1; j < n; j++)
		{
			/* Swap. */
			if (a[j] < a[i])
			{
				unsigned tmp1;
				unsigned tmp2;
				
				tmp1 = a[j]; tmp2   = map[j];
				a[j] = a[i]; map[j] = map[i];
				a[i] = tmp1; map[i] = tmp2;
			}
		}
	}
	
	return (map);
} 

/**
 * @brief Initializes the lpt scheduler.
 * 
 * @details Initializes the lpt scheduler.
 * 
 * @param tasks    Tasks to assign.
 * @param ntasks   Number of tasks.
 * @param nthreads Number of threads.
 */
void scheduler_lpt_init
(const unsigned *tasks, unsigned ntasks, unsigned nthreads)
{
	unsigned *workload;      /* Workload of current thread. */
	unsigned *sorted_tasks; /* Sorted tasks.                */
	unsigned *sorting_map;  /* Sorting map.                 */
	
	/* Already initialized. */
	if (scheduler_data.taskmap != NULL)
		return;
		
	sorted_tasks = smalloc(ntasks*sizeof(unsigned));
	memcpy(sorted_tasks, tasks, ntasks*sizeof(unsigned));
	workload = smalloc(nthreads*sizeof(unsigned));
	memset(workload, 0, nthreads*sizeof(unsigned));
	
	sorting_map = sort(sorted_tasks, ntasks);
	
	/* Initialize scheduler data. */
	scheduler_data.ntasks = ntasks;
	scheduler_data.taskmap = smalloc(ntasks*sizeof(unsigned));
	scheduler_data.tasks = tasks;
	scheduler_data.nthreads = nthreads;
		
	/* Assign tasks to threads. */
	for (unsigned i = ntasks; i > 0; i--)
	{
		unsigned tid = 0;

		for (unsigned j = 1; j < nthreads; j++)
		{
			if (workload[j] < workload[tid])
				tid = j;
		}

		workload[tid] += tasks[i - 1];
		scheduler_data.taskmap[sorting_map[i - 1]] = tid;
	}
	
	/* House keeping. */
	free(sorting_map);
	free(workload);
	free(sorted_tasks);
}

/**
 * @brief Finalizes the lpt scheduler.
 * 
 * @details Finalizes the lpt scheduler.
 */
void scheduler_lpt_end(void)
{
	free(scheduler_data.taskmap);
	scheduler_data.taskmap = NULL;
}

/**
 * @brief Best scheduler.
 * 
 * @details Schedule tasks to threads statically.
 * 
 * @param tid Thread to be scheduled.
 * 
 * @returns Number of tasks assigned to the thread @p tid.
 */
unsigned scheduler_lpt_sched(unsigned tid)
{
	unsigned n;        /* Number of tasks. */
	unsigned workload; /* Workload amount. */
	
	/* Get next tasks. */
	n = 0;
	workload = 0;
	for (unsigned i = 0; i < scheduler_data.ntasks; i++)
	{
		/* This task is mine. */
		if (scheduler_data.taskmap[i] == tid)
		{
			n++;
			workload += scheduler_data.tasks[i];
			threads[tid].ntasks++;
			if (scheduler_data.tasks[i] < threads[tid].min)
				threads[tid].min = scheduler_data.tasks[i];
			if (scheduler_data.tasks[i] > threads[tid].max)
				threads[tid].max = scheduler_data.tasks[i];
			scheduler_data.taskmap[i] = scheduler_data.nthreads;
		}
	}
	
	threads[tid].workload += workload;
	dqueue_insert(tid, workload);
	
	return (n);
}

