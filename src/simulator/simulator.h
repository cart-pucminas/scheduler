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

	/**
	 * @brief Thread.
	 */
	struct thread
	{
		unsigned tid;      /**< Thread ID.         */
		unsigned workload; /**< Assigned workload. */
	};
	
	/**
	 * @name Schedulers
	 */
	/**@{*/
	#define SCHEDULER_NONE           0 /**< Null scheduler.    */
	#define SCHEDULER_STATIC         1 /**< Static scheduler.  */
	#define SCHEDULER_DYNAMIC        2 /**< Dynamic scheduler. */
	#define SCHEDULER_WORKLOAD_AWARE 3 /**< Null scheduler.    */
	/**@}*/
	
	/**
	 * @brief Scheduling strategy.
	 */
	typedef void (*scheduler_t)(unsigned);
	
	/* Forward definitions. */
	extern unsigned ntasks;
	extern unsigned nthreads;

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
		unsigned remaining;  /**< Remaining time.           */
		struct dqueue *next; /**< Next thread in the queue. */
	};
	
	/**
	 * @name Delta Queue Operations
	 */
	/**@{*/
	extern void dqueue_create(void);
	extern void dqueue_detroy(void);
	extern void dqueue_insert(unsigned, unsigned);
	extern unsigned dqueue_remove(void);
	extern unsigned dqueue_next_timestamp(void);
	/**@}*/
	
	/**@}*/
	
#endif /* _SIMULATOR_H_ */
