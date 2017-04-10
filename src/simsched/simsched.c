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
#include <limits.h>
#include <math.h>
#include <stdbool.h>

#include <mylib/util.h>
#include <mylib/array.h>
#include <mylib/dqueue.h>
#include <mylib/queue.h>

#include <scheduler.h>
#include <workload.h>
#include <thread.h>

/**
 * @brief Number of chunks.
 */
int nchunks = 0;

/**
 * @brief Ready threads.
 */
static queue_tt ready;

/**
 * @brief Running threads.
 */
static dqueue_tt running;

/**
 * @brief Spawns threads.
 *
 * @param threads Working threads.
 * @param pin     Pin threads?
 */
static void threads_spawn(array_tt threads, bool pinthreads)
{
	ready = queue_create();	
	running = dqueue_create();

	if (!pinthreads)
		array_shuffle(threads);

	for (int i = 0; i < array_size(threads); i++)
	{
		thread_tt t = array_get(threads, i);
		queue_insert(ready, t);
	}
}

/**
 * @brief Joins threads.
 */
static void threads_join(void)
{
	dqueue_destroy(running);
	queue_destroy(ready);
}

/**
 * @brief Dumps simulation statistics.
 *
 * @param threads Working threads.
 */
static void simsched_dump(array_tt threads)
{
	double min, max, total;
	double mean, stddev;
	int nthreads;

	nthreads = array_size(threads);

	min = INT_MAX; max = INT_MIN;
	total = 0; mean = 0.0; stddev = 0.0;

	/* Compute min, max, total. */
	for (int i = 0; i < nthreads; i++)
	{
		thread_tt t;   /* Working thread.         */
		double wtotal; /* Workload assigned to t. */

		t = array_get(threads, i);
		wtotal = thread_wtotal(t);

		if (min > wtotal)
			min = wtotal;
		if (max < wtotal)
			max = wtotal;

		total += wtotal;
	}

	/* Compute mean. */
	mean = ((double) total)/nthreads;

	/* Compute stddev. */
	for (int i = 0; i < nthreads; i++)
	{
		thread_tt t;

		t = array_get(threads, i);

		stddev += pow(thread_wtotal(t) - mean, 2);
	}
	stddev = sqrt(stddev/(nthreads));

	/* Print statistics. */
	printf("nchunks: %d\n", nchunks);
	printf("time: %lf\n", max);
	printf("cost: %lf\n", max*nthreads);
	printf("performance: %lf\n", total/max);
	printf("total: %lf\n", total);
	printf("cov: %lf\n", stddev/mean);
	printf("slowdown: %lf\n", max/((double) min));
}

/**
 * @brief Chooses a thread to run next.
 *
 * @param Target thread queue.
 *
 * @returns The next thread to run.
 */
static thread_tt choose_thread(queue_tt q)
{
	thread_tt t;

	/* Sanity check. */
	assert(q != NULL);
	assert(!queue_empty(q));

	do
	{
		t = queue_remove(q);

		if (rand()%2)
			break;

		queue_insert(q, t);
	} while (!queue_empty(q));

	return (t);
}

/**
 * @brief Simulates a parallel loop.
 *
 * @param w         Workload.
 * @param threads   Working threads.
 * @param strategy  Scheduling strategy.
 * @param chunksize Chunksize;
 */
void simshed(const_workload_tt w, array_tt threads, const struct scheduler *strategy, int chunksize)
{
	/* Sanity check. */
	assert(w != NULL);
	assert(threads != NULL);
	assert(strategy != NULL);

	threads_spawn(threads, strategy->pinthreads);

	strategy->init(w, threads, chunksize);

	/* Simulate. */
	for (int i = 0; i < workload_ntasks(w); /* noop */)
	{
		/* Schedule ready threads. */
		while (!queue_empty(ready))
		{
			thread_tt t;

			t = choose_thread(ready);
			i += strategy->sched(running, t);
		}

		/* Reschedule running threads. */
		while (!dqueue_empty(running))
		{
			queue_insert(ready, dqueue_remove(running));

			if (dqueue_next_counter(running) != 0)
				break;
		}
	}

	strategy->end();

	simsched_dump(threads);

	threads_join();
}

