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
#include <math.h>

#include <mylib/util.h>
#include <mylib/dqueue.h>
#include <mylib/queue.h>

#include <scheduler.h>

/**
 * @brief WAS scheduler data.
 */
static struct
{
	const_workload_tt workload; /**< Workload.   */
	array_tt threads;           /**< Threads.    */
	thread_tt *taskmap;         /**< Scheduling. */
} scheddata = { NULL, NULL, NULL };

/**
 * @brief Applies thread quota correction.
 *
 * @param threads Target threads.
 * @param nrounds Number of rounds.
 * @param quota   Target quota.
 */
static void was_quota_correct(const_array_tt threads, int nrounds, int *quota)
{
	int nthreads;

	nthreads = array_size(threads);

	/* Check quota. */
	for (int i = 0; i < nthreads; i++)
		nrounds -= quota[i];

	/* Missing rounds. */
	if (nrounds > 0)
	{
		int tidx = 0;

		while (nrounds > 0)
		{
			quota[tidx]++;

			tidx = (tidx + 1)%nthreads;
			nrounds--;
		}
	}

	/* Too much rounds. */
	else if (nrounds < 0)
	{
		int tidx = nthreads - 1;

		while (nrounds < 0)
		{
			quota[tidx]--;

			tidx = (tidx - 1)%nthreads;
			nrounds++;
		}
	}
}

/**
 * @brief Computes round quota for threads.
 *
 * @param threads Target threads.
 * @param nrounds  Number of rounds.
 *
 * @returns Round quota for threads.
 */
static int *was_quota_compute(const_array_tt threads, int nrounds)
{
	double total; /* Total capacity.    */
	int nthreads; /* Number of threads. */
	int *quota;   /* Task quota.        */

	/* Sanity check. */
	assert(threads != NULL);

	nthreads = array_size(threads);
	
	quota = smalloc(nthreads*sizeof(int));

	/* Initialize quota[]. */
	for (int i = 0; i < nthreads; i++)
		quota[i] = 0;

	/* Compute total capacity. */
	total = 0;
	for (int i = 0; i < nthreads; i++)
		total += 1.0/thread_capacity(array_get(threads, i));

	/* Compute quota. */
	for (int i = 0; i < nthreads; i++)
		quota[i] = round(nrounds*((1.0/thread_capacity(array_get(threads, i)))/total));

	was_quota_correct(threads, nrounds, quota);

	return (quota);
}

/**
 * @brief Initializes the was scheduler.
 * 
 * @param workload Target workload.
 * @param threads  Target threads.
 */
void scheduler_was_init(const_workload_tt workload, array_tt threads)
{
	int k;        /* Scheduling offset.      */
	int ntasks;   /* Number of tasks.        */
	int nthreads; /* Number of threads.      */
	int *map;     /* Task sorting map.       */
	int tidx;     /* Current thread index.   */
	double *load; /* Current thread load.    */
	int *quota;   /* Task quota for threads. */
	
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
	load = smalloc(nthreads*sizeof(double));
	for (int i = 0; i < nthreads; i++)
		load[i] = 0;

	/* Compute remaining tasks. */
	k = ntasks%(2*nthreads);

	quota = was_quota_compute(threads, (ntasks - k)/2);

	assert(quota[0] > 0);

	/* Assign tasks to threads. */
	tidx = 0;
	for (int i = k; i < k + (ntasks - k)/2; i++)
	{
		int l = map[i];
		int r = map[ntasks - ((i - k) + 1)];

		/* Wrap around. */
		while (quota[tidx] <= 0)
			tidx = (tidx + 1)%nthreads;

		scheddata.taskmap[l] = array_get(threads, tidx);
		scheddata.taskmap[r] = array_get(threads, tidx);

		load[tidx] += workload_task(workload, map[l]);
		load[tidx] += workload_task(workload, map[r]);

		quota[tidx]--;
		
		tidx = (tidx + 1)%nthreads;
	}

	/* Assign remaining tasks. */
	for (int i = k - 1; i >= 0; i--)
	{
		int leastoverload;

		leastoverload = 0;

		/* Find least overload thread. */
		for (int j = 1; j < nthreads; j++)
		{
			if (load[j] < load[leastoverload])
				leastoverload = j;
		}

		scheddata.taskmap[map[i]] = array_get(threads, leastoverload);

		load[leastoverload] += workload_task(workload, map[i]);
	}

	/* House keeping. */
	free(quota);
	free(load);
	free(map);
}

/**
 * @brief Finalizes the was scheduler.
 */
void scheduler_was_end(void)
{
	free(scheddata.taskmap);
	scheddata.taskmap = NULL;
}

/**
 * @brief WAS scheduler.
 * 
 * @param running Target queue of running threads.
 * @param t       Target thread
 * 
 * @returns Number scheduled tasks,
 */
int scheduler_was_sched(dqueue_tt running, thread_tt t)
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
 * @brief WAS scheduler.
 */
static struct scheduler _sched_was = {
	scheduler_was_init,
	scheduler_was_sched,
	scheduler_was_end
};

const struct scheduler *sched_was = &_sched_was;


