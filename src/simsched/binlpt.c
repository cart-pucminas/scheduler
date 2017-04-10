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
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <mylib/util.h>
#include <mylib/dqueue.h>
#include <mylib/queue.h>

#include <scheduler.h>

/**
 * @brief BinLPT scheduler data.
 */
static struct
{
	const_workload_tt workload; /**< Workload.   */
	array_tt threads;           /**< Threads.    */
	thread_tt *taskmap;         /**< Scheduling. */
} scheddata = { NULL, NULL, NULL };

/*
 * Exchange two numbers.
 */
#define exch(a, b, t) \
	{ (t) = (a); (a) = (b); (b) = (t); }

/*
 * Insertion sort.
 */
static void insertion(int *map, int *a, int n)
{
	int t;    /* Temporary value. */
	
	/* Sort. */
	for (int i = 0; i < (n - 1); i++)
	{
		for (int j = i + 1; j < n; j++)
		{
			/* Swap. */
			if (a[j] < a[i])
			{
				exch(a[i], a[j], t);
				exch(map[i], map[j], t);
			}
		}
	}
}
 
/*
 * Sorts an array of numbers.
 */
static int *binlpt_chunk_sortmap(int *a, int n)
{
	int *map;

	map = smalloc(n*sizeof(int));

	/* Create map. */
	for (int i = 0; i < n; i++)
		map[i] = i;

	insertion(map, a, n);

	return (map);
} 

/**
 * @brief Computes the cummulative sum of an array.
 *
 * @param a Target array.
 * @param n Size of target array.
 *
 * @returns Commulative sum.
 */
static int *binlpt_compute_commulative_sum(const int *a, int n)
{
	int *sum;

	sum = malloc(n*sizeof(int));
	assert(sum != NULL);

	/* Compute cummulative sum. */
	sum[0] = 0;
	for (int i = 1; i < n; i++)
		sum[i] = sum[i - 1] + a[i - 1];

	return (sum);
}

/**
 * @brief Computes chunk sizes.
 *
 * @param tasks   Target tasks.
 * @param ntasks  Number of tasks.
 * @param nchunks Number of chunks.
 *
 * @returns Chunk sizes.
 */
static int *binlpt_compute_chunksizes(const_workload_tt workload, int nchunks)
{
	int ntasks;      /* Number of tasks. */
	int chunkweight;
	int *chunksizes, *off;

	chunksizes = calloc(nchunks, sizeof(int));
	assert(chunksizes != NULL);

	ntasks = workload_ntasks(workload);

	/* Compute verage chunk weight. */
	off = workload_cummulative_sum(workload);
	chunkweight = off[ntasks]/nchunks;

	/* Compute chunksizes. */
	for (int k = 0, i = 0; i < ntasks; /* noop */)
	{
		int j = ntasks;

		/* Bundles as much iterations as we can. */
		if (k < (nchunks - 1))
		{
			for (j = i + 1; j < ntasks; j++)
			{
				if (off[j] - off[i] > chunkweight)
					break;
			}
		}


		chunksizes[k] = j - i;
		i = j;
		k++;
	}

	/* House keeping. */
	free(off);

	return (chunksizes);
}

/**
 * @brief Compute chunk weights.
 *
 * @param tasks     Target tasks.
 * @param ntasks    Number of tasks.
 * @param chunksize Size of chunks.
 * @param nchunks   Number of chunks.
 *
 * @returns Table of chunks.
 */
static int *binlpt_compute_chunkweights(const_workload_tt workload, const int *chunksizes, int nchunks)
{
	int *chunks; /* Chunk weights.   */
	int ntasks;  /* Number of tasks. */

	chunks = calloc(nchunks, sizeof(int));
	assert(chunks != NULL);

	ntasks = workload_ntasks(workload);

	/* Compute chunks. */
	for (int i = 0, k = 0; i < nchunks; i++)
	{
		for (int j = 0; j < chunksizes[i]; j++)
		{
			chunks[i] += workload_task(workload, k++);

			/* 
			 * Number of chunks should not
			 * exceed the number fot tasks.
			 */
			assert(k <= ntasks);
		}
	}

	return (chunks);
}

/**
 * @brief Initializes the binlpt scheduler.
 * 
 * @param workload  Target workload.
 * @param threads   Target threads.
 * @param chunksize Chunk size.
 */
void scheduler_binlpt_init(const_workload_tt workload, array_tt threads, int chunksize)
{
	int ntasks;      /* Number of tasks.              */
	int nthreads;    /* Number of threads.            */
	int *map;        /* Task sorting map.             */
	int *wsize;      /* Workload assigned to threads. */
	int *chunksizes; /* Chunks sizes.                 */
	int *chunks;     /* Chunks.                       */
	int *chunkoff;   /* Offset to chunks.             */
	int maxnchunks;  /* Number of chunks.             */
	
	/* Sanity check. */
	assert(workload != NULL);
	assert(threads != NULL);
	assert(chunksize > 0);

	/* Already initialized. */
	if (scheddata.taskmap != NULL)
		return;
	
	ntasks = workload_ntasks(workload);
	maxnchunks = chunksize;
	nthreads = array_size(threads);

	/* Initialize scheduler data. */
	scheddata.workload = workload;
	scheddata.threads = threads;
	scheddata.taskmap = smalloc(ntasks*sizeof(thread_tt));

	chunksizes = binlpt_compute_chunksizes(workload, maxnchunks);
	chunks = binlpt_compute_chunkweights(workload, chunksizes, maxnchunks);
	chunkoff = binlpt_compute_commulative_sum(chunksizes, maxnchunks);

	map = binlpt_chunk_sortmap(chunks, maxnchunks);
	wsize = smalloc(nthreads*sizeof(int));
	memset(wsize, 0, nthreads*sizeof(int));

	/* Assign tasks to threads. */
	for (int i = maxnchunks; i > 0; i--)
	{
		int k;    /* Alias for current chunk. */
		int tidx; /* Least overloaded thread. */

		if (chunks[i - 1] == 0)
			continue;

		nchunks++;

		/* Search for least overloaded thread. */
		tidx = 0;
		for (int j = 1; j < array_size(threads); j++)
		{
			if (wsize[j] < wsize[tidx])
				tidx = j;
		}

		k = map[i - 1];
		for (int j = 0; j < chunksizes[k]; j++)
			scheddata.taskmap[chunkoff[k] + j] = array_get(threads, tidx);
		wsize[tidx] += chunks[i - 1];
	}
	
	/* House keeping. */
	free(wsize);
	free(map);
	free(chunkoff);
	free(chunks);
	free(chunksizes);
}

/**
 * @brief Finalizes the binlpt scheduler.
 */
void scheduler_binlpt_end(void)
{
	free(scheddata.taskmap);
	scheddata.taskmap = NULL;
}

/**
 * @brief BinLPT scheduler.
 * 
 * @param running Target queue of running threads.
 * @param t       Target thread
 * 
 * @returns Number scheduled tasks,
 */
int scheduler_binlpt_sched(dqueue_tt running, thread_tt t)
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
 * @brief BinLPT scheduler.
 */
static struct scheduler _sched_binlpt = {
	false,
	scheduler_binlpt_init,
	scheduler_binlpt_sched,
	scheduler_binlpt_end
};

const struct scheduler *sched_binlpt = &_sched_binlpt;

