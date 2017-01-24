/*
 * Copyright(C) 2016 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This file is part of Scheduler.
 *
 * Scheduler is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * Scheduler is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Scheduler; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <stdlib.h>
#include <stdbool.h>

#include <mylib/util.h>
#include <mylib/dqueue.h>
#include <mylib/queue.h>

#include <scheduler.h>

/**
 * @brief Dynamic scheduler data.
 */
static struct
{
	int i0;                     /**< Last iteration scheduled. */
	const_workload_tt workload; /**< Workload.                 */
	array_tt threads;           /**< Threads.                  */
	thread_tt *taskmap;         /**< Scheduling.               */
} scheddata = { 0, NULL, NULL, NULL };

/**
 * @brief Initializes the dynamic scheduler.
 * 
 * @param workload Target workload.
 * @param threads  Target threads.
 */
void scheduler_dynamic_init(const_workload_tt workload, array_tt threads)
{
	int ntasks;
	
	/* Sanity check. */
	assert(workload != NULL);
	assert(threads != NULL);

	/* Already initialized. */
	if (scheddata.taskmap != NULL)
		return;
	
	ntasks = workload_ntasks(workload);

	/* Initialize scheduler data. */
	scheddata.i0 = 0;
	scheddata.workload = workload;
	scheddata.threads = threads;
	scheddata.taskmap = smalloc(ntasks*sizeof(thread_tt));
		
	/* Assign tasks to threads. */
	for (int i = 0; i < ntasks; i++)
		scheddata.taskmap[i] = NULL;
}

/**
 * @brief Finalizes the dynamic scheduler.
 */
void scheduler_dynamic_end(void)
{
	free(scheddata.taskmap);
	scheddata.taskmap = NULL;
}

/**
 * @brief Dynamic scheduler.
 * 
 * @param running Target queue of running threads.
 * @param t       Target thread
 * 
 * @returns Number scheduled tasks,
 */
int scheduler_dynamic_sched(dqueue_tt running, thread_tt t)
{
	int n = 0;         /* Number of tasks scheduled. */
	int wsize = 0;     /* Size of assigned work.     */
	int chunksize = 1; /* Chunk size.                */

	/* Get next tasks. */
	for (int i = 0; i < workload_ntasks(scheddata.workload); i++)
	{
		/* Skip tasks from other threads. */
		if (scheddata.taskmap[i] != NULL)
			continue;

		n++;
		wsize += thread_assign(t, workload_task(scheddata.workload, i));
		scheddata.taskmap[i] = t;

		/* We have already scheduled enough. */
		if (n >= chunksize)
		{
			scheddata.i0 = i + 1;
			break;
		}
	}
	
	dqueue_insert(running, t, wsize);

	return (n);
}

/**
 * @brief Dynamic scheduler.
 */
static struct scheduler _sched_dynamic = {
	false,
	scheduler_dynamic_init,
	scheduler_dynamic_sched,
	scheduler_dynamic_end
};

const struct scheduler *sched_dynamic = &_sched_dynamic;

