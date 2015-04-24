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

#include "simulator.h"

/**
 * @brief Static scheduler data.
 */
static struct
{
	unsigned ntasks;       /**< Number of tasks.   */
	unsigned *taskmap;     /**< Task map.          */
	const unsigned *tasks; /**< Tasks.             */
	unsigned nthreads;     /**< Number of threads. */
} scheduler_data = {
	0,
	NULL,
	NULL,
	0
};

/**
 * @brief Initializes the static scheduler.
 * 
 * @details Initializes the static scheduler.
 * 
 * @param tasks    Tasks to assign.
 * @param ntasks   Number of tasks.
 * @param nthreads Number of threads.
 */
void scheduler_static_init
(const unsigned *tasks, unsigned ntasks, unsigned nthreads)
{
	unsigned tasks_per_thread;
	
	/* Already initialized. */
	if (scheduler_data.taskmap != NULL)
		return;
	
	/* Initialize scheduler data. */
	scheduler_data.ntasks = ntasks;
	scheduler_data.taskmap = smalloc(ntasks*sizeof(unsigned));
	scheduler_data.tasks = tasks;
	scheduler_data.nthreads = nthreads;
		
	/* Assign tasks to threads. */
	tasks_per_thread = ntasks/nthreads;
	for (unsigned i = 0; i < ntasks; i++)
	{
		scheduler_data.taskmap[i] = (i/tasks_per_thread >= nthreads) ? 
			nthreads - 1 : i/tasks_per_thread;
	}
}

/**
 * @brief Finalizes the static scheduler.
 * 
 * @details Finalizes the static scheduler.
 */
void scheduler_static_end(void)
{
	free(scheduler_data.taskmap);
	scheduler_data.taskmap = NULL;
}

/**
 * @brief Static scheduler.
 * 
 * @details Schedule tasks to threads statically.
 * 
 * @param tid Thread to be scheduled.
 * 
 * @returns Number of tasks assigned to the thread @p tid.
 */
unsigned scheduler_static_sched(unsigned tid)
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
			threads[tid].workload += workload;
			threads[tid].ntasks++;
			if (workload < threads[tid].min)
				threads[tid].min = workload;
			if (workload > threads[tid].max)
				threads[tid].max = workload;
			scheduler_data.taskmap[i] = scheduler_data.nthreads;
		}
	}
	
	dqueue_insert(tid, workload);
	
	return (n);
}

