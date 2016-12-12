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

#include <mylib/util.h>
#include <mylib/dqueue.h>
#include <mylib/queue.h>

#include <scheduler.h>

/**
 * @brief LPT scheduler data.
 */
static struct
{
	const_workload_tt workload; /**< Workload.   */
	array_tt threads;           /**< Threads.    */
	thread_tt *taskmap;         /**< Scheduling. */
} scheddata = { NULL, NULL, NULL };

/**
 * @brief Initializes the lpt scheduler.
 * 
 * @param workload Target workload.
 * @param threads  Target threads.
 */
void scheduler_lpt_init(const_workload_tt workload, array_tt threads)
{
	int ntasks;   /* Number of tasks.              */
	int nthreads; /* Number of threads.            */
	int *map;     /* Task sorting map.             */
	int *wsize;   /* Workload assigned to threads. */
	
	/* Sanity check. */
	assert(workload != NULL);
	assert(threads != NULL);

	/* Already initialized. */
	if (scheddata.taskmap != NULL)
		return;
	
	ntasks = workload_ntasks(workload);
	nthreads = array_size(threads);

	/* Initialize scheduler data. */
	scheddata.workload = workload;
	scheddata.threads = threads;
	scheddata.taskmap = smalloc(ntasks*sizeof(thread_tt));

	map = workload_sortmap(workload);
	wsize = smalloc(nthreads*sizeof(int));
	memset(wsize, 0, nthreads*sizeof(int));

	/* Assign tasks to threads. */
	for (int i = 0; i < ntasks; i++)
	{
		int tidx = 0;

		for (int j = 1; j < array_size(threads); j++)
		{
			if (wsize[j] < wsize[tidx])
				tidx = j;
		}

		wsize[tidx] += workload_task(workload, map[i]);
		scheddata.taskmap[map[i]] = array_get(threads, tidx);
	}
	
	/* House keeping. */
	free(map);
	free(wsize);
}

/**
 * @brief Finalizes the lpt scheduler.
 */
void scheduler_lpt_end(void)
{
	free(scheddata.taskmap);
	scheddata.taskmap = NULL;
}

/**
 * @brief LPT scheduler.
 * 
 * @param running Target queue of running threads.
 * @param t       Target thread
 * 
 * @returns Number scheduled tasks,
 */
int scheduler_lpt_sched(dqueue_tt running, thread_tt t)
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
 * @brief LPT scheduler.
 */
static struct scheduler _sched_lpt = {
	scheduler_lpt_init,
	scheduler_lpt_sched,
	scheduler_lpt_end
};

const struct scheduler *sched_lpt = &_sched_lpt;

