/*
 * Copyright(C) 2017 Pedro H. Penna <pedrohenriquepenna@gmail.com>
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

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#include <mylib/util.h>
#include <mylib/dqueue.h>
#include <mylib/queue.h>

#include <scheduler.h>

/**
 * @brief Guided scheduler data.
 */
static struct
{
	int i0;                     /**< Last iteration scheduled. */
	const_workload_tt workload; /**< Workload.                 */
	array_tt threads;           /**< Threads.                  */
	int chunksize;              /**< Chunksize.                */
} scheddata = { 0, NULL, NULL, 1 };

/**
 * @brief Initializes the guided scheduler.
 * 
 * @param workload  Target workload.
 * @param threads   Target threads.
 * @param chunksize Chunk size.
 */
void scheduler_guided_init(const_workload_tt workload, array_tt threads, int chunksize)
{
	/* Sanity check. */
	assert(workload != NULL);
	assert(threads != NULL);
	assert(chunksize > 0);

	/* Initialize scheduler data. */
	scheddata.i0 = 0;
	scheddata.workload = workload;
	scheddata.threads = threads;
	scheddata.chunksize = chunksize;
}

/**
 * @brief Finalizes the guided scheduler.
 */
void scheduler_guided_end(void)
{
}

/**
 * @brief Guided scheduler.
 * 
 * @param running Target queue of running threads.
 * @param t       Target thread
 * 
 * @returns Number scheduled tasks,
 */
int scheduler_guided_sched(dqueue_tt running, thread_tt t)
{
	int chunksize;        /* Number of tasks scheduled. */
	int wsize;    /* Size of assigned work.     */
	int ntasks;   /* Number of tasks.           */
	int nthreads; /* Number of hteads.          */

	ntasks = workload_ntasks(scheddata.workload);

	/* Done. */
	if (scheddata.i0 == ntasks)
		return (0);

	nchunks++;

	nthreads = array_size(scheddata.threads);

	/* Comput chunksize. */
	chunksize = (ntasks - scheddata.i0)/(2*nthreads);
	if (chunksize < scheddata.chunksize)
		chunksize = scheddata.chunksize;
	if (chunksize > ntasks - scheddata.i0)
		chunksize = ntasks - scheddata.i0;

	/* Schedule iterations. */
	wsize = 0;
	for (int i = scheddata.i0; i < (scheddata.i0 + chunksize); i++)
	{
		wsize += workload_task(scheddata.workload, i);
		thread_assign(t, workload_task(scheddata.workload, i));
	}

	/* Update schedule data. */
	scheddata.i0 += chunksize;	
	
	dqueue_insert(running, t, wsize);

	return (chunksize);
}

/**
 * @brief Guided scheduler.
 */
static struct scheduler _sched_guided = {
	false,
	scheduler_guided_init,
	scheduler_guided_sched,
	scheduler_guided_end
};

const struct scheduler *sched_guided = &_sched_guided;

