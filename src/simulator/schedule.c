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
#include <stdlib.h>

#include <mylib/util.h>

#include "simulator.h"

/**
 * @brief Number of threads.
 */
static unsigned nthreads;

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
scheduler_init_t schedulers_init[5] = {
	NULL,                             /* SCHEDULER_NONE           */
	&scheduler_static_init,           /* SCHEDULER_STATIC         */
	&scheduler_dynamic_init,          /* SCHEDULER_DYNAMIC        */
	&scheduler_workload_aware_init,   /* SCHEDULER_WORKLOAD_AWARE */
	&scheduler_smart_round_robin_init /* SCHEDULER_SMART_ROUND_ROBIN */
};

/**
 * @brief Schedulers sched() table.
 */
scheduler_sched_t schedulers_sched[5] = {
	NULL,                              /* SCHEDULER_NONE              */
	&scheduler_static_sched,           /* SCHEDULER_STATIC            */
	&scheduler_dynamic_sched,          /* SCHEDULER_DYNAMIC           */
	&scheduler_workload_aware_sched,   /* SCHEDULER_WORKLOAD_AWARE    */
	&scheduler_smart_round_robin_sched /* SCHEDULER_SMART_ROUND_ROBIN */
};

/**
 * @brief Schedulers end() table.
 */
scheduler_end_t schedulers_end[5] = {
	NULL,                            /* SCHEDULER_NONE              */
	&scheduler_static_end,           /* SCHEDULER_STATIC            */
	&scheduler_dynamic_end,          /* SCHEDULER_DYNAMIC           */
	&scheduler_workload_aware_end,   /* SCHEDULER_WORKLOAD_AWARE    */
	&scheduler_smart_round_robin_end /* SCHEDULER_SMART_ROUND_ROBIN */
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
	unsigned i;   /* Working index.       */
	unsigned tid; /* ID of chosen thread. */
		
	/* Choose thread. */
	i = randnum()%nthreads;
	while (ready[i] == NULL)
		i = (i + 1)%nthreads;
	
	/* Remove thread from ready pool. */
	nready--;
	tid = ready[i]->tid;
	ready[i] = NULL;
	
	return (tid);
}

/**
 * @brief Loop scheduler.
 * 
 * @details Simulates a loop scheduler.
 */
void schedule
(const unsigned *tasks, unsigned ntasks, unsigned _nthreads, unsigned scheduler)
{
	unsigned n;
	
	nthreads = _nthreads;
	dqueue_create();
	
	/* Create pool of ready threads. */
	nready = nthreads;
	ready = smalloc(nthreads*sizeof(struct thread *));
	for (unsigned i = 0; i < nthreads; i++)
		ready[i] = &threads[i];
	
		
	/* Schedule. */
	schedulers_init[scheduler](tasks, ntasks, nthreads);
	for (n = ntasks; n > 0; /* loop*/ )
	{
		unsigned tid;
		
		/* Pick a thread to run. */
		while (nready > 0)
		{	
			tid = choose_thread();
			n -= schedulers_sched[scheduler](tid);
		}
		
		/* Put threads back into ready pool. */
		do
		{
			tid = dqueue_remove();
			ready[nready++] = &threads[tid];
		} while ((!dqueue_empty()) && (dqueue_next_timestamp() == 0));
	}
	schedulers_end[scheduler]();
	
	/* House keeping. */
	free(ready);
	dqueue_destroy();
}
