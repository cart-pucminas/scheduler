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
 * @brief Number of threads.
 */
static unsigned nthreads;

/**
 * @brief Threads.
 */
struct thread *threads = NULL;

/**
 * @brief Number of ready threads.
 */
static unsigned nready = 0;

/**
 * @brief Pool of ready tasks.
 */
static struct thread **ready = NULL;

/**
 * @brief Schedulers init() table.
 */
scheduler_init_t schedulers_init[4] = {
	NULL,                   /* SCHEDULER_NONE           */
	&scheduler_static_init, /* SCHEDULER_STATIC         */
	NULL,                   /* SCHEDULER_DYNAMIC        */
	NULL                    /* SCHEDULER_WORKLOAD_AWARE */
};

/**
 * @brief Schedulers sched() table.
 */
scheduler_sched_t schedulers_sched[4] = {
	NULL,                    /* SCHEDULER_NONE           */
	&scheduler_static_sched, /* SCHEDULER_STATIC         */
	NULL,                    /* SCHEDULER_DYNAMIC        */
	NULL                     /* SCHEDULER_WORKLOAD_AWARE */
};

/**
 * @brief Schedulers end() table.
 */
scheduler_end_t schedulers_end[4] = {
	NULL,                  /* SCHEDULER_NONE           */
	&scheduler_static_end, /* SCHEDULER_STATIC         */
	NULL,                  /* SCHEDULER_DYNAMIC        */
	NULL                   /* SCHEDULER_WORKLOAD_AWARE */
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
	while (ready[tid] == NULL)
	{
		tid = (tid + 1)%nthreads;
	}
	
	/* Remove thread from ready pool. */
	nready--;
	ready[tid] = NULL;
	
	return (tid);
}

/**
 * @brief Loop scheduler.
 * 
 * @details Simulates a loop scheduler.
 */
void schedule
(unsigned *tasks, unsigned ntasks, unsigned _nthreads, unsigned scheduler)
{
	unsigned n;
	
	nthreads = _nthreads;
	
	/* Create threads. */
	info("creating threads...", VERBOSE_INFO);
	threads = smalloc(nthreads*sizeof(struct thread));
	for (unsigned i = 0; i < nthreads; i++)
	{
		threads[i].tid = i;
		threads[i].workload = 0;
	}
	
	/* Create pool of ready threads. */
	info("creating pool of ready threads...", VERBOSE_INFO);
	nready = nthreads;
	ready = smalloc(nthreads*sizeof(struct thread *));
	for (unsigned i = 0; i < nthreads; i++)
		ready[i] = &threads[i];
	
	/* Schedule. */
	info("scheduling...", VERBOSE_INFO);
	schedulers_init[scheduler](tasks, ntasks, nthreads);
	for (n = ntasks; n > 0; /* loop*/ )
	{
		unsigned tid;       /* Threads ID.      */
		unsigned timestamp; /* Next time stamp. */
		
		/* Pick a thread to run. */
		while (nready > 0)
		{	
			tid = choose_thread();
			n -= schedulers_sched[scheduler](tid);
		}
		
		/* Put threads back into ready pool. */
		do
		{
			timestamp = dqueue_next_timestamp();
			tid = dqueue_remove();
			ready[nready++] = &threads[tid];
		} while (timestamp == dqueue_next_timestamp());
	}
	schedulers_end[scheduler]();
	
	/* Print statistics. */
	printf("thread ID;workload\n");
	for (unsigned i = 0; i < nthreads; i++)
		printf("%u;%u\n", threads[i].tid, threads[i].workload);
	
	/* House keeping. */
	free(ready);
	free(threads);
	dqueue_destroy();
}
