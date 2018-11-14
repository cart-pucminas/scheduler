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

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <mylib/util.h>
#include <mylib/dqueue.h>
#include <mylib/queue.h>

#include <scheduler.h>

/**
 * @brief KASS scheduler data.
 */
static struct
{
	int initialized;            /**< Scheduler initialized?            */
	int chunksize;              /**< Chunks size.                      */
	int *wqueues_start;         /**< Start of work queues.             */
	int *wqueues_end;           /**< Length of work queues.            */
	int *wqueues_i0;            /**< Current iteration on work queues. */
	const_workload_tt workload; /**< Workload.                         */
	array_tt threads;           /**< Threads.                          */
} scheddata = { 0, 0, NULL, NULL, NULL, NULL, NULL };

/**
 * @brief Computes the static partitioning for a uniform workload.
 */
static void scheduler_kass_static_uniform_workload(void)
{
	int ntasks;   /* Number of tasks.   */
	int nthreads; /* Number of threads. */
	int chunklen; /* Chunk size.        */
	
	ntasks = workload_ntasks(scheddata.workload);
	nthreads = array_size(scheddata.threads);
	chunklen = ntasks/nthreads;
	
	/* Create work queues. */
	scheddata.wqueues_start[0] = 0;
	for (int i = 0, j = 0; i < ntasks; i += chunklen)
	{
		if (i == (ntasks - 1))
		{
			scheddata.wqueues_end[j] = ntasks - 1;
			break;
		}

		j++;
	}
}

/**
 * @brief Computes the static partitioning for a homogeneous platform.
 */
static void scheduler_kass_static_homogeneous_platform(int wsize)
{
	int size;        /* Size of current chunk. */
	int ntasks;      /* Number of tasks.       */
	int nthreads;    /* Number of threads.     */
	int chunkweight; /* Chunk size.            */
	
	ntasks = workload_ntasks(scheddata.workload);
	nthreads = array_size(scheddata.threads);
	chunkweight = wsize/nthreads;
	
	/* Create work queues. */
	size = 0;
	scheddata.wqueues_start[0] = 0;
	for (int i = 0, j = 0; i < ntasks; i++)
	{
		if ((i == (ntasks - 1)) || (j == (nthreads - 1)))
		{
			scheddata.wqueues_end[j] = ntasks - 1;
			break;
		}

		/* Next partition. */
		if (size >= chunkweight)
		{
			scheddata.wqueues_end[j++] = i - 1;

			size = 0;
			scheddata.wqueues_start[j] = i;
		}
		
		size += workload_task(scheddata.workload, i);
	}
}

/**
 * @brief Computes workload statistics.
 */
static void workload_stats(double *total, double *mean, double *stddev)
{
	double wtotal = 0;                               /* Size.                */
	double wmean = 0;                                 /* Mean.               */
	double wstddev = 0;                               /* Standard deviation. */
	int ntasks = workload_ntasks(scheddata.workload); /* Number of tasks.    */

	/* Compute mean. */
	for (int i = 0; i < ntasks; i++)
		wtotal += workload_task(scheddata.workload, i);
	wmean = wtotal/((double) ntasks);

	/* Compute standard deviation (sample). */
	for (int i = 0; i < ntasks; i++)
		wstddev += pow(workload_task(scheddata.workload, i) - wmean, 2);
	wstddev = sqrt(wstddev/(ntasks - 1));

	if (total != NULL) *total = wtotal;
	if (mean != NULL) *mean = wmean;
	if (stddev != NULL) *stddev = wstddev;
}

/**
 * @brief Compute thread statistics.
 */
static void thread_stats(double *total, double *mean, double *stddev)
{
	double ttotal = 0;                            /* Size.               */
	double tmean = 0;                             /* Mean.               */
	double tstddev = 0;                           /* Standard deviation. */
	int nthreads = array_size(scheddata.threads); /* Number of threads.  */

	/* Compute mean. */
	for (int i = 0; i < nthreads; i++)
		ttotal += thread_capacity(array_get(scheddata.threads, i));
	tmean = ttotal/((double) nthreads);

	/* Compute standard deviation (sample). */
	for (int i = 0; i < nthreads; i++)
		tstddev += pow(thread_capacity(array_get(scheddata.threads, i) ) - tmean, 2);
	tstddev = sqrt(tstddev/(nthreads-1));

	/* Save results. */
	if (total != NULL) *total = ttotal;
	if (mean != NULL) *mean = tmean;
	if (stddev != NULL) *stddev = tstddev;
}

#ifdef DEBUG_KASS

/**
 * @brief Dump work queues.
 */
static void scheduler_kass_dump(void)
{
	int nthreads = array_size(scheddata.threads);

	for (int i = 0; i < nthreads; i++)
	{
		fprintf(stderr, "wqueue %3d: [%5d, %5d]\n", 
				i,
				scheddata.wqueues_start[i],
				scheddata.wqueues_end[i]
		);
	}
}

#endif /* DEBUG_KASS */

/**
 * @brief Static scheduler.
 */
static void scheduler_kass_static(void)
{
	double tmean, tstddev;                        /* Thread statistics.   */
	double wtotal, wmean, wstddev;                /* Workload statistics. */
	int nthreads = array_size(scheddata.threads); /* Number of threads.   */

	workload_stats(&wtotal, &wmean, &wstddev);
	thread_stats(NULL, &tmean, &tstddev);

	/* Initialize work queues. */
	for (int i = 0; i < nthreads; i++)
	{
		scheddata.wqueues_start[i] = -1;
		scheddata.wqueues_end[i] = -1;
	}

	/* Compute required statistics. */
	workload_stats(&wtotal, &wmean, &wstddev);
	thread_stats(NULL, &tmean, &tstddev);

	/* Compute initial partitioning. */
	if (wstddev/wmean < 0.1)
		scheduler_kass_static_uniform_workload();
	else if (tstddev/tmean < 0.1)
		scheduler_kass_static_homogeneous_platform(wtotal);
	else
		error("heterogeneous platforms currently unsupported");

	/* Initialize head of work queues. */
	for (int i = 0; i < nthreads; i++)
		scheddata.wqueues_i0[i] = scheddata.wqueues_start[i];

#ifdef DEBUG_KASS
	scheduler_kass_dump();
#endif /* DEBUG_KASS */
}

/**
 * @brief Initializes KASS.
 * 
 * @param workload  Target workload.
 * @param threads   Target threads.
 * @param chunkweight Chunk size.
 */
void scheduler_kass_init(const_workload_tt workload, array_tt threads, int chunksize)
{
	int nthreads;
	
	/* Sanity check. */
	assert(workload != NULL);
	assert(threads != NULL);
	assert(chunksize > 0);

	/* Already initialized. */
	if (scheddata.initialized)
		return;
	
	/* Aliases. */
	nthreads = array_size(threads);

	/* Initialize scheduler structures. */
	scheddata.workload = workload;
	scheddata.threads = threads;
	scheddata.chunksize = chunksize;
	scheddata.wqueues_start = smalloc(nthreads*sizeof(int));
	scheddata.wqueues_end = smalloc(nthreads*sizeof(int));
	scheddata.wqueues_i0 = smalloc(nthreads*sizeof(int));

	scheduler_kass_static();

	scheddata.initialized = 1;
}

/**
 * @brief Finalizes the KASS scheduler.
 */
void scheduler_kass_end(void)
{
	free(scheddata.wqueues_i0);
	free(scheddata.wqueues_end);
	free(scheddata.wqueues_start);
	scheddata.initialized = 0;
}

/**
 * @brief KASS scheduler.
 * 
 * @param running Target queue of running threads.
 * @param t       Target thread
 * 
 * @returns Number scheduled tasks,
 */
int scheduler_kass_sched(dqueue_tt running, thread_tt t)
{
	int tid;        /* Thread ID.                 */
	int wqueue;     /* Working queue.             */
	int chunksize;  /* Number of tasks scheduled. */
	int wsize;      /* Size of assigned work.     */
	int nthreads;   /* Number of threads.         */
	int nremaining; /* Number of remaining tasks. */

	tid = thread_gettid(t);
	nthreads = array_size(scheddata.threads);

	wqueue = tid%nthreads;

	/* Find an unfinished working queue. */
	for (wqueue = (tid + 1)%nthreads; ; wqueue = (wqueue + 1)%nthreads)
	{
		/* Done. */
		if (wqueue == (tid%nthreads))
			return (0);

		if (scheddata.wqueues_i0[wqueue] < 0)
			continue;

		if (scheddata.wqueues_i0[wqueue] <= scheddata.wqueues_end[wqueue])
			break;
	}

	nremaining = scheddata.wqueues_end[wqueue] - scheddata.wqueues_i0[wqueue] + 1;

	/* Compute chunk length. */
	chunksize = (int) round((1.0/scheddata.chunksize)*(nremaining));
	if (chunksize < 1)
		chunksize = 1;
	if (chunksize > nremaining)
		chunksize = nremaining;

	assert(chunksize != 0);

	/* Schedule iterations. */
	wsize = 0;
	for (int i = scheddata.wqueues_i0[wqueue]; i < (scheddata.wqueues_i0[wqueue] + chunksize); i++)
	{
		wsize += workload_task(scheddata.workload, i);
		thread_assign(t, workload_task(scheddata.workload, i));

		if (i == scheddata.wqueues_end[wqueue])
		{
			chunksize = i - scheddata.wqueues_i0[wqueue] + 1;
			break;
		}
	}

	/* Update schedule data. */
	scheddata.wqueues_i0[wqueue] += chunksize;	
	
	dqueue_insert(running, t, wsize);

	nchunks++;
	return (chunksize);
}

/**
 * @brief KASS scheduler.
 */
static struct scheduler _sched_kass = {
	false,
	scheduler_kass_init,
	scheduler_kass_sched,
	scheduler_kass_end
};

const struct scheduler *sched_kass = &_sched_kass;

