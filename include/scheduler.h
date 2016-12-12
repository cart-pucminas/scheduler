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

	#include <mylib/array.h>
	#include <mylib/dqueue.h>

	#include "workload.h"
	#include "thread.h"

	/**
	 * @brief Loop scheduling strategy.
	 */
	struct scheduler
	{
		void (*init)(const_workload_tt, array_tt); /**< Initialize. */
		int (*sched)(dqueue_tt, thread_tt);        /**< Schedule.   */
		void (*end)(void);                         /**< End.        */
	};

	/**
	 * @brief Supported Loop Scheduling Strategies.
	 */
	/**@{*/
	extern const struct scheduler *sched_dynamic;
	extern const struct scheduler *sched_lpt;
	extern const struct scheduler *sched_srr;
	extern const struct scheduler *sched_static;
	/**@}*/

	extern void simshed(const_workload_tt, int, const struct scheduler*);

#endif /* SCHEDULER_H_ */
