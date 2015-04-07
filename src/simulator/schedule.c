/*
 * Copyright(C) 2015 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <assert.h>

#include <mylib/util.h>

#include "simulator.h"

/**
 * @brief Number of ready threads.
 */
static unsigned nready = 0;

/**
 * @brief Threads.
 */
struct thread *threads = NULL;

/**
 * @brief Pool of ready tasks.
 */
static struct thread **ready = NULL;

/**
 * @brief Loop iterations.
 */
unsigned *tasks;

/**
 * @brief Schedulers table.
 */
scheduler_t schedulers[4] = {
	NULL, /* SCHEDULER_NONE           */
	NULL, /* SCHEDULER_STATIC         */
	NULL, /* SCHEDULER_DYNAMIC        */
	NULL  /* SCHEDULER_WORKLOAD_AWARE */
};

/**
 * @brief Chooses a thread to run next.
 * 
 * @details Choses a thread to run from the ready pool.
 * 
 * @returns The ID of the chosen thread.
 */
static unsigned choose_thread(void)
{
	unsigned tid;
		
	/* Choose thread. */
	tid = randnum()%nthreads;
	while (ready[tid] != NULL)
		tid = (tid + 1)%nthreads;

	nready--;
	
	return (tid);
}

/**
 * @brief Loop scheduler.
 * 
 * @details Simulates a loop scheduler.
 */
void schedule(unsigned scheduler)
{
	/* Create threads. */
	threads = smalloc(nthreads*sizeof(struct thread));
	for (unsigned i = 0; i < nthreads; i++)
	{
		threads[i].tid = i;
		threads[i].workload = 0;
	}

	/* Generate loop iterations. */
	for (unsigned i = 0; i < ntasks; i++)
		tasks[i] = randnum();
	
	/* Create pool of ready threads. */
	nready = nthreads;
	ready = smalloc(nthreads*sizeof(struct thread *));
	for (unsigned i = 0; i < nthreads; i++)
		ready[i] = &threads[i];
	
	/* Schedule. */
	while (ntasks > 0)
	{
		unsigned tid;
		unsigned timestamp;
		
		/* Pick a thread to run. */
		while (nready > 0)
		{	
			tid = choose_thread();
			schedulers[scheduler](tid);
		}
		
		/* Put threads back into ready pool. */
		do
		{
			timestamp = dqueue_next_timestamp();
			tid = dqueue_remove();
			ready[nready++] = &threads[tid];
		} while (timestamp == dqueue_next_timestamp());
	}
	
	/* Print statistics. */
	for (unsigned i = 0; i < nthreads; i++)
		printf("%u;%u\n", threads[i].tid, threads[i].workload);
	
	/* House keeping. */
	free(ready);
	free(threads);
}
