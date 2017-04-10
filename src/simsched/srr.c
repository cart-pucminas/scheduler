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
#include <string.h>
#include <stdbool.h>

#include <mylib/util.h>
#include <mylib/dqueue.h>
#include <mylib/queue.h>

#include <scheduler.h>

/**
 * @brief SRR scheduler data.
 */
static struct
{
	const_workload_tt workload; /**< Workload.   */
	array_tt threads;           /**< Threads.    */
	thread_tt *taskmap;         /**< Scheduling. */
} scheddata = { NULL, NULL, NULL };

/**
 * @brief Initializes the srr scheduler.
 * 
 * @param workload  Target workload.
 * @param threads   Target threads.
 * @param chunksize Chunk size.
 */
void scheduler_srr_init(const_workload_tt workload, array_tt threads, int chunksize)
{
	int ntasks;   /* Number of tasks.      */
	int nthreads; /* Number of threads.    */
	int *map;     /* Task sorting map.     */
	int tidx;     /* Current thread index. */

	((void) chunksize);

	/* Sanity check. */
	assert(workload != NULL);
	assert(threads != NULL);

	/* Already initialized. */
	if (scheddata.taskmap != NULL)
		return;
	
	ntasks = workload_ntasks(workload);
	nthreads = array_size(threads);

	nchunks = ntasks/2;

	/* Initialize scheduler data. */
	scheddata.workload = workload;
	scheddata.threads = threads;
	scheddata.taskmap = smalloc(ntasks*sizeof(thread_tt));

	map = workload_sortmap(workload);

	/* Assign tasks to threads. */
	tidx = 0;
	if (ntasks%2)
	{
		scheddata.taskmap[map[0]] = array_get(threads, tidx);
		
		/* Balance workload. */
		for (int i = 1; i < ntasks/2; i++)
		{
			scheddata.taskmap[map[i]] = array_get(threads, tidx);
			scheddata.taskmap[map[ntasks - i]] = array_get(threads, tidx);
			
			/* Wrap around. */
			tidx = (tidx + 1)%nthreads;
		}
		
		return;
	}
	else
	{
		for (int i = 0; i < ntasks/2; i++)
		{
			scheddata.taskmap[map[i]] = array_get(threads, tidx);
			scheddata.taskmap[map[ntasks - i - 1]] = array_get(threads, tidx);
			
			/* Wrap around. */
			tidx = (tidx + 1)%nthreads;
		}
	}
	
	/* House keeping. */
	free(map);
}

/**
 * @brief Finalizes the srr scheduler.
 */
void scheduler_srr_end(void)
{
	free(scheddata.taskmap);
	scheddata.taskmap = NULL;
}

/**
 * @brief SRR scheduler.
 * 
 * @param running Target queue of running threads.
 * @param t       Target thread
 * 
 * @returns Number scheduled tasks,
 */
int scheduler_srr_sched(dqueue_tt running, thread_tt t)
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
		wsize += workload_task(scheddata.workload, i);
		thread_assign(t, workload_task(scheddata.workload, i));
	}
	
	dqueue_insert(running, t, wsize);

	return (n);
}

/**
 * @brief SRR scheduler.
 */
static struct scheduler _sched_srr = {
	false,
	scheduler_srr_init,
	scheduler_srr_sched,
	scheduler_srr_end
};

const struct scheduler *sched_srr = &_sched_srr;


