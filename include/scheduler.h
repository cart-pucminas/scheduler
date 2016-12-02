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

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

	#include "dqueue.h"

	#include "workload.h"
	#include "thread.h"

	/**
	 * @brief Loop scheduling strategy.
	 */
	struct scheduler
	{
		void (*init)(const_workload_tt, thread_tt *, int); /**< Initialize. */
		int (*sched)(dqueue_tt, thread_tt);                /**< Schedule.   */
		void (*end)(void);                                 /**< End.        */
	};

	/**
	 * @brief Supported Loop Scheduling Strategies.
	 */
	/**@{*/
	extern const struct scheduler *sched_static;
	extern const struct shceduler *sched_lpt;
	extern const struct scheduler *sched_dynamic;
	extern const struct scheduler *srr;
	/**@}*/

#endif /* SCHEDULER_H_ */
