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
 * @brief Static scheduler data.
 */
static struct
{
	const_workload_tt workload; /**< Workload.   */
	array_tt threads;           /**< Threads.    */
	thread_tt *taskmap;         /**< Scheduling. */
	int chunksize;              /**< Chunksize.  */
} scheddata = { NULL, NULL, NULL, 1 };

/**
 * @brief Initializes the static scheduler.
 * 
 * @param workload  Target workload.
 * @param threads   Target threads.
 * @param chunksize Chunk size.
 */
void scheduler_static_init(const_workload_tt workload, array_tt threads, int chunksize)
{
	int tidx;      /* Index of working thread. */
	int ntasks;    /* Workload size.           */
	
	/* Sanity check. */
	assert(workload != NULL);
	assert(threads != NULL);
	assert(chunksize > 0);

	/* Already initialized. */
	if (scheddata.taskmap != NULL)
		return;
	
	ntasks = workload_ntasks(workload);

	/* Initialize scheduler data. */
	scheddata.workload = workload;
	scheddata.threads = threads;
	scheddata.taskmap = smalloc(ntasks*sizeof(thread_tt));
		
	/* Assign tasks to threads. */
	tidx = 0;
	for (int i = 0; i < ntasks; i += chunksize)
	{
	
		for (int j = 0; j < chunksize; j++)
		{
			if (i + j >= ntasks)
				break;

			scheddata.taskmap[i + j] = array_get(threads, tidx);
		}

		tidx = (tidx + 1)%array_size(threads);
	}
}

/**
 * @brief Finalizes the static scheduler.
 */
void scheduler_static_end(void)
{
	free(scheddata.taskmap);
	scheddata.taskmap = NULL;
}

/**
 * @brief Static scheduler.
 * 
 * @param running Target queue of running threads.
 * @param t       Target thread
 * 
 * @returns Number scheduled tasks,
 */
int scheduler_static_sched(dqueue_tt running, thread_tt t)
{
	int n = 0;     /* Number of tasks scheduled. */
	int wsize = 0; /* Size of assigned work.     */

	/* Get next tasks. */
	for (int i = 0; i < workload_ntasks(scheddata.workload); i++)
	{
		/* Skip tasks from other threads. */
		if (scheddata.taskmap[i] != t)
			continue;

		n++;
		wsize += thread_assign(t, workload_task(scheddata.workload, i));
	}
	
	dqueue_insert(running, t, wsize);

	return (n);
}

/**
 * @brief Static scheduler.
 */
static struct scheduler _sched_static = {
	false,
	scheduler_static_init,
	scheduler_static_sched,
	scheduler_static_end
};

const struct scheduler *sched_static = &_sched_static;
