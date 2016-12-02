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
#include <dqueue.h>
#include <queue.h>

#include <scheduler.h>
#include <workload.h>
#include <thread.h>

/**
 * @brief Simulates a parallel loop.
 */
void simshed
(const_workload_tt w, int nthreads, const struct scheduler *strategy)
{
	thread_tt *threads; /* Threads          */
	dqueue_tt running;  /* Running threads. */
	queue_tt ready;     /* Ready threads.   */

	/* Sanity check. */
	assert(w != NULL);
	assert(nthreads > 0);
	assert(strategy != NULL);

	ready = queue_create();	
	running = dqueue_create();

	/* Create threads. */
	threads = smalloc(nthreads*sizeof(thread_tt));
	for (int i = 0; i < nthreads; i++)
	{
		threads[i] = thread_create();
		queue_insert(ready, &threads[i]);
	}

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

	/* House keeping. */
	for (int i = 0; i < nthreads; i++)
		thread_destroy(threads[i]);
	free(threads);
	dqueue_destroy(running);
	queue_destroy(ready);
}


