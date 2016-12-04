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

#include <util.h>
#include <array.h>
#include <dqueue.h>
#include <queue.h>

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
 * @brief Simulates a parallel loop.
 */
void simshed
(const_workload_tt w, int nthreads, const struct scheduler *strategy)
{
	/* Sanity check. */
	assert(w != NULL);
	assert(nthreads > 0);
	assert(strategy != NULL);

	threads_spawn(nthreads);

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

	threads_join();
}


