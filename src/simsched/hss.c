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

#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include <mylib/util.h>
#include <mylib/dqueue.h>
#include <mylib/queue.h>

#include <scheduler.h>

/**
 * @brief HSS scheduler data.
 */
static struct
{
	int i0;                     /**< Last iteration scheduled. */
	const_workload_tt workload; /**< Workload.                 */
	array_tt threads;           /**< Threads.                  */
	int chunksize;              /**< Chunksize.                */
	int wremaining;             /**< Remaining workload.       */
} scheddata = { 0, NULL, NULL, 1, 0 };

/**
 * @brief Initializes the hss scheduler.
 * 
 * @param workload  Target workload.
 * @param threads   Target threads.
 * @param chunksize Chunk size.
 */
void scheduler_hss_init(const_workload_tt workload, array_tt threads, int chunksize)
{
	int ntasks;     /* Number of tasks. */
	int wremaining; /* Total workload.  */

	/* Sanity check. */
	assert(workload != NULL);
	assert(threads != NULL);
	assert(chunksize > 0);

	/* Compute remaining workload. */
	ntasks = workload_ntasks(workload);
	wremaining = 0;
	for (int i = 0; i < ntasks; i++)
		wremaining += workload_task(workload, i);

	/* Initialize scheduler data. */
	scheddata.i0 = 0;
	scheddata.workload = workload;
	scheddata.threads = threads;
	scheddata.chunksize = chunksize;
	scheddata.wremaining = wremaining;
}

/**
 * @brief Finalizes the hss scheduler.
 */
void scheduler_hss_end(void)
{
}

/**
 * @brief HSS scheduler.
 * 
 * @param running Target queue of running threads.
 * @param t       Target thread
 * 
 * @returns Number scheduled tasks,
 */
int scheduler_hss_sched(dqueue_tt running, thread_tt t)
{
	int n;        /* Number of tasks scheduled.      */
	int k;        /* Number of scheduled iterations. */
	int wsize;    /* Size of assigned work.          */
	int ntasks;   /* Number of tasks.                */
	int nthreads; /* Number of hteads.               */

	nchunks++;

	ntasks = workload_ntasks(scheddata.workload);
	nthreads = array_size(scheddata.threads);

	/* Comput chunksize. */
	n = ceil(scheddata.wremaining/(1.5*nthreads));
	if (n < scheddata.chunksize)
		n = scheddata.chunksize;

	/* Schedule iterations. */
	wsize = 0; k = 0;
	for (int i = scheddata.i0; i < ntasks; i++)
	{
		int w1;
		int w2;

		k++;
		wsize += workload_task(scheddata.workload, i);
		thread_assign(t, workload_task(scheddata.workload, i));

		w1 = wsize;
		w2 = (i + 1 < ntasks) ? 
			wsize + workload_task(scheddata.workload, i + 1) : 0;

		/* Keep scheduling. */
		if (w2 <= n)
			continue;

		/* Best fit. */
		if (w1 >= n)
			break;

		/* Best range approximation. */
		if ((w2 - n) > (n - w1))
			break;
	}

	/* Update scheduler data. */
	scheddata.i0 += k;
	scheddata.wremaining -= wsize;
	
	dqueue_insert(running, t, wsize);

	return (k);
}

/**
 * @brief HSS scheduler.
 */
static struct scheduler _sched_hss = {
	false,
	scheduler_hss_init,
	scheduler_hss_sched,
	scheduler_hss_end
};

const struct scheduler *sched_hss = &_sched_hss;


