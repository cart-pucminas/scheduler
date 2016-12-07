/*
 * Copyright(C) 2016 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This file is part of SimSched.
 *
 * SimSched is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * SimSched is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with SimSched; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <assert.h>
#include <limits.h>
#include <math.h>

#include <mylib/util.h>
#include <mylib/array.h>
#include <mylib/dqueue.h>
#include <mylib/queue.h>

#include <scheduler.h>
#include <workload.h>
#include <thread.h>

/**
 * @brief Working threads.
 */
static array_tt threads;

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
 * @param nthreads Number of threads.
 */
static void threads_spawn(int nthreads)
{
	threads = array_create(nthreads);
	ready = queue_create();	
	running = dqueue_create();

	for (int i = 0; i < nthreads; i++)
	{
		thread_tt t = thread_create();
		array_set(threads, i, t);
		queue_insert(ready, t);
	}
}

/**
 * @brief Joins threads.
 */
static void threads_join(void)
{
	for (int i = 0; i < array_size(threads); i++)
	{
		thread_tt t = array_get(threads, i);
		thread_destroy(t);
	}
	array_destroy(threads);
	dqueue_destroy(running);
	queue_destroy(ready);
}

/**
 * @brief Dumps simulation statistics.
 */
static void simsched_dump(void)
{
	int min, max, total;
	double mean, stddev;

	min = INT_MAX; max = INT_MIN;
	total = 0; mean = 0.0; stddev = 0.0;

	/* Compute min, max, total. */
	for (int i = 0; i < array_size(threads); i++)
	{
		thread_tt t; /* Working thread.         */
		int wtotal;  /* Workload assigned to t. */

		t = array_get(threads, i);
		wtotal = thread_wtotal(t);

		if (min > wtotal)
			min = wtotal;
		if (max < wtotal)
			max = wtotal;

		total += wtotal;
	}

	/* Compute mean. */
	mean = ((double) total)/array_size(threads);

	/* Compute stddev. */
	for (int i = 0; i < array_size(threads); i++)
	{
		thread_tt t;

		t = array_get(threads, i);

		stddev += pow(thread_wtotal(t) - mean, 2);
	}
	stddev = sqrt(stddev/(array_size(threads)));

	/* Print statistics. */
	printf("min: %d\n", min);
	printf("max: %d\n", max);
	printf("mean: %lf\n", mean);
	printf("stddev: %lf\n", 100*stddev/mean);
	printf("imbalance: %lf\n", 100*(max - min)/((double) total));
	printf("speeddown: %lf\n", max/((double) min));
}

/**
 * @brief Simulates a parallel loop.
 *
 * @param w        Workload.
 * @param nthreads Number of threads
 * @param strategy Scheduling strategy.
 */
void simshed
(const_workload_tt w, int nthreads, const struct scheduler *strategy)
{
	/* Sanity check. */
	assert(w != NULL);
	assert(nthreads > 0);
	assert(strategy != NULL);

	threads_spawn(nthreads);

	strategy->init(w, threads);

	/* Simulate. */
	for (int i = 0; i < workload_ntasks(w); /* noop */)
	{
		/* Schedule ready threads. */
		while (!queue_empty(ready))
		{
			thread_tt t;

			t = queue_remove(ready);
			i += strategy->sched(running, t);
		}

		/* Reschedule running threads. */
		while (!dqueue_empty(running))
		{
			if (dqueue_next_counter(running) != 0)
				break;

			queue_insert(ready, dqueue_remove(running));
		}
	}

	strategy->end();

	simsched_dump();

	threads_join();
}

