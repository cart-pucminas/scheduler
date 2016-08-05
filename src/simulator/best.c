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

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <mylib/util.h>

#include <simulator.h>

/**
 * @brief Best scheduler data.
 */
static struct
{
	unsigned i0;           /**< Last scheduled iteration. */
	unsigned ntasks;       /**< Number of tasks.          */
	const unsigned *tasks; /**< Tasks.                    */
	unsigned nthreads;     /**< Number of threads.        */
	unsigned sum;          /**< Workload sum.             */
} scheduler_data = {
	0,
	0,
	NULL,
	0,
	0
};

/**
 * @brief Initializes the best scheduler.
 * 
 * @details Initializes the best scheduler.
 * 
 * @param tasks    Tasks to assign.
 * @param ntasks   Number of tasks.
 * @param nthreads Number of threads.
 */
void scheduler_best_init
(const unsigned *tasks, unsigned ntasks, unsigned nthreads)
{
	/* Initialize scheduler data. */
	scheduler_data.i0 = 0;
	scheduler_data.ntasks = ntasks;
	scheduler_data.tasks = tasks;
	scheduler_data.nthreads = nthreads;
	scheduler_data.sum = 0;
	
	for (unsigned i = 0; i < ntasks; i++)
		scheduler_data.sum += tasks[i];
}

/**
 * @brief Finalizes the best scheduler.
 * 
 * @details Finalizes the best scheduler.
 */
void scheduler_best_end(void)
{
	/* NOOP */
}

/**
 * @brief Best scheduler.
 * 
 * @details Schedule tasks to threads statically.
 * 
 * @param tid Thread to be scheduled.
 * 
 * @returns Number of tasks assigned to the thread @p tid.
 */
unsigned scheduler_best_sched(unsigned tid)
{
	unsigned n;        /* Number of tasks. */
	unsigned workload; /* Workload amount. */
	
	/* Get next tasks. */
	n = (scheduler_data.i0 < scheduler_data.ntasks) ?
		scheduler_data.ntasks/scheduler_data.nthreads : 0;
	workload = floor(((double)scheduler_data.sum)/scheduler_data.nthreads);
	
	/* Skip to next task. */
	scheduler_data.i0 += n;
	
	threads[tid].workload += workload;
	dqueue_insert(tid, workload);
	
	return (n);
}

