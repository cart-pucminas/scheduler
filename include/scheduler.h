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

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

	#include <stdbool.h>

	#include <mylib/array.h>
	#include <mylib/dqueue.h>

	#include "workload.h"
	#include "thread.h"

	/**
	 * @brief Loop scheduling strategy.
	 */
	struct scheduler
	{
		bool pinthreads;                                /**< Pin threads? */
		void (*init)(const_workload_tt, array_tt, int); /**< Initialize.  */
		int (*sched)(dqueue_tt, thread_tt);             /**< Schedule.    */
		void (*end)(void);                              /**< End.         */
	};

	/**
	 * @brief Supported Loop Scheduling Strategies.
	 */
	/**@{*/
	extern const struct scheduler *sched_guided;
	extern const struct scheduler *sched_dynamic;
	extern const struct scheduler *sched_hss;
	extern const struct scheduler *sched_kass;
	extern const struct scheduler *sched_binlpt;
	extern const struct scheduler *sched_srr;
	extern const struct scheduler *sched_static;
	/**@}*/

	/* Fordward definitions. */
	extern int nchunks;

	extern void simshed(const_workload_tt, array_tt, const struct scheduler*, int);

#endif /* SCHEDULER_H_ */
