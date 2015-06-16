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
 * @brief Dynamic scheduler data.
 */
static struct
{
	unsigned ntasks;     /**< Number of tasks.   */
	unsigned *taskmap;   /**< Task map.          */
	const double *tasks; /**< Tasks.             */
	unsigned nthreads;   /**< Number of threads. */
} scheduler_data = {
	0,
	NULL,
	NULL,
	0
};

/**
 * @brief Initializes the dynamic scheduler.
 * 
 * @details Initializes the dynamic scheduler.
 * 
 * @param tasks    Tasks to assign.
 * @param ntasks   Number of tasks.
 * @param nthreads Number of threads.
 */
void scheduler_dynamic_init
(const double *tasks, unsigned ntasks, unsigned nthreads)
{
	/* Already initialized. */
	if (scheduler_data.taskmap != NULL)
		return;
	
	/* Initialize scheduler data. */
	scheduler_data.ntasks = ntasks;
	scheduler_data.taskmap = smalloc(ntasks*sizeof(unsigned));
	scheduler_data.tasks = tasks;
	scheduler_data.nthreads = nthreads;
	
	/* Initialize task map. */
	for (unsigned i = 0; i < scheduler_data.ntasks; i++)
		scheduler_data.taskmap[i] = nthreads;
}

/**
 * @brief Finalizes the dynamic scheduler.
 * 
 * @details Finalizes the dynamic scheduler.
 */
void scheduler_dynamic_end(void)
{
	/* Compute average task size. */
	for (unsigned i = 0; i < scheduler_data.nthreads; i++)
	{
		threads[i].avg = (threads[i].ntasks > 0) ?
			((double) threads[i].workload)/threads[i].ntasks : 0;
	}
	
	free(scheduler_data.taskmap);
	scheduler_data.taskmap = NULL;
}

/**
 * @brief Dynamic scheduler.
 * 
 * @details Schedule tasks to threads dynamically.
 * 
 * @param tid Thread to be scheduled.
 * 
 * @returns Number of tasks assigned to the thread @p tid.
 */
unsigned scheduler_dynamic_sched(unsigned tid)
{
	unsigned n;             /* Number of tasks.               */
	unsigned workload;      /* Workload amount.               */
	static unsigned i0 = 0; /* Starting point to search from. */
	
	/* Get next tasks. */
	n = 0;
	workload = 0;
	for (unsigned i = i0; i < scheduler_data.ntasks; i++)
	{
		/* Get this task. */
		if (scheduler_data.taskmap[i] == scheduler_data.nthreads)
		{
			n++;
			workload += scheduler_data.tasks[i];
			threads[tid].ntasks++;
			if (scheduler_data.tasks[i] < threads[tid].min)
				threads[tid].min = scheduler_data.tasks[i];
			if (scheduler_data.tasks[i] > threads[tid].max)
				threads[tid].max = scheduler_data.tasks[i];
			scheduler_data.taskmap[i] = tid;
			i0 = i;
			
			/* We have already scheduled enough. */
			if (n >= chunksize)
				break;
		}
	}
	
	threads[tid].workload += workload;
	dqueue_insert(tid, workload);
	
	return (n);
}

