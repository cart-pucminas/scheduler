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

#ifndef _SIMULATOR_H_
#define _SIMULATOR_H_

	#include <stdbool.h>

	/*========================================================================*
	 *                              Scheduler                                 *
	 *========================================================================*/

	/**
	 * @brief Thread.
	 */
	struct thread
	{
		unsigned tid;      /**< Thread ID.                  */
		unsigned workload; /**< Total assigned workload.    */
		unsigned ntasks;   /**< Number of assigned tasks.   */
		double avg;        /**< Average task size.          */
		unsigned max;      /**< Maximum assigned task size. */
		unsigned min;      /**< Minimum assigned task size. */
	};
	
	/**
	 * @brief Initializes the scheduling strategy.
	 */
	typedef void (*scheduler_init_t)(const unsigned *, unsigned, unsigned);
	
	/**
	 * @brief Scheduling strategy.
	 */
	typedef unsigned (*scheduler_sched_t)(unsigned);
	
	/**
	 * @brief Finalizes the scheduling strategy.
	 */
	typedef void (*scheduler_end_t)(void);
	
	/* Chunk size. */
	extern unsigned chunksize;
	
	/* Forward definitions. */
	extern struct thread *threads;
	extern void schedule(const unsigned *, unsigned, unsigned, unsigned);
	extern void scheduler_static_init(const unsigned *, unsigned, unsigned);
	extern unsigned scheduler_static_sched(unsigned);
	extern void scheduler_static_end(void);
	extern void scheduler_dynamic_init(const unsigned *, unsigned, unsigned);
	extern unsigned scheduler_dynamic_sched(unsigned);
	extern void scheduler_dynamic_end(void);
	extern void scheduler_workload_aware_init(const unsigned *, unsigned, unsigned);
	extern unsigned scheduler_workload_aware_sched(unsigned);
	extern void scheduler_workload_aware_end(void);
	extern void scheduler_smart_round_robin_init(const unsigned *, unsigned, unsigned);
	extern unsigned scheduler_smart_round_robin_sched(unsigned);
	extern void scheduler_smart_round_robin_end(void);
	extern void scheduler_best_init(const unsigned *, unsigned, unsigned);
	extern unsigned scheduler_best_sched(unsigned);
	extern void scheduler_best_end(void);

	/*========================================================================*
	 *                             Delta Queue                                *
	 *========================================================================*/

	/**
	 * @defgroup dqueue
	 * 
	 * @brief Delta Queue
	 */
	/**@{*/

	/**
	 * @brief Delta queue.
	 */
	struct dqueue
	{
		unsigned tid;        /**< Thread ID.                */
		unsigned remaining;    /**< Remaining time.           */
		struct dqueue *next; /**< Next thread in the queue. */
	};
	
	/**
	 * @name Delta Queue Operations
	 */
	/**@{*/
	extern void dqueue_create(void);
	extern void dqueue_destroy(void);
	extern void dqueue_insert(unsigned, unsigned);
	extern unsigned dqueue_remove(void);
	extern unsigned dqueue_next_timestamp(void);
	extern bool dqueue_empty(void);
	/**@}*/
	
	/**@}*/
	
#endif /* _SIMULATOR_H_ */
