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
 * @brief workload aware scheduler data.
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
 * @brief Initializes the workload aware scheduler.
 * 
 * @details Initializes the workload aware scheduler.
 * 
 * @param tasks    Tasks to assign.
 * @param ntasks   Number of tasks.
 * @param nthreads Number of threads.
 */
void scheduler_workload_aware_init
(const double *tasks, unsigned ntasks, unsigned nthreads)
{
	unsigned tid;
	double avg;
	double *workload;
	
	/* Already initialized. */
	if (scheduler_data.taskmap != NULL)
		return;
	
	workload = scalloc(nthreads, sizeof(double));
	
	/* Initialize scheduler data. */
	scheduler_data.ntasks = ntasks;
	scheduler_data.taskmap = smalloc(ntasks*sizeof(unsigned));
	scheduler_data.tasks = tasks;
	scheduler_data.nthreads = nthreads;
		
	/* Compute average workload size. */
	avg = 0.0;
	for (unsigned i = 0; i < ntasks; i++)
		avg += tasks[i];
	avg /= nthreads;
	
	/* Assign tasks to threads. */
	tid = 0;
	for (unsigned i = 0; i < ntasks; i++)
	{
		/*
		 * Too many tasks already assigned to
		 * this thread, so skip to the next one.
		 */
		if (workload[tid] + tasks[i] > avg)
		{
			unsigned j;
			
			for (j = (tid + 1)%nthreads; j != tid; j = (j + 1)%nthreads)
			{
				if (workload[j] + tasks[i] < avg)
					break;
			}
			
			if (j == tid)
			{
				unsigned k;
				
				k = 0;
				
				for (j = 1; j < nthreads; j++)
				{
					if (workload[j] + tasks[i] < workload[k] + tasks[i])
						k = j;
				}
				
				j = k;
			}
		
			tid = j;
		}
		
		scheduler_data.taskmap[i] = tid;
		workload[tid] += tasks[i];
	}
	
	/* House keeping. */
	free(workload);
}

/**
 * @brief Finalizes the workload aware scheduler.
 * 
 * @details Finalizes the workload aware scheduler.
 */
void scheduler_workload_aware_end(void)
{
	free(scheduler_data.taskmap);
	scheduler_data.taskmap = NULL;
}

/**
 * @brief workload aware scheduler.
 * 
 * @details Schedule tasks to threads considering the workload behavior.
 * 
 * @param tid Thread to be scheduled.
 * 
 * @returns Number of tasks assigned to the thread @p tid.
 */
unsigned scheduler_workload_aware_sched(unsigned tid)
{
	unsigned n;      /* Number of tasks. */
	double workload; /* Workload amount. */
	
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

