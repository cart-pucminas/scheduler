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

#ifndef WORKLOAD_H_
#define WORKLOAD_H_

	#include <stdio.h>

	#include "statistics.h"

	/**
	 * @brief Opaque pointer to a synthetic workload.
	 */
	typedef struct workload * workload_tt;

	/**
	 * @brief Constant opaque pointer to a synthetic workload.
	 */
	typedef const struct workload * const_workload_tt;

	/**
	 * @brief Workload sorting types.
	 */
	enum workload_sorting
	{
		WORKLOAD_ASCENDING,  /**< Ascending order.  */
		WORKLOAD_DESCENDING, /**< Descending order. */
		WORKLOAD_SHUFFLE     /**< Shuffle.          */
	};

	/**
	 * @name Operations on Workload
	 */
	/**@{*/
	extern workload_tt workload_create(histogram_tt, int);
	extern void workload_destroy(workload_tt);
	extern int workload_ntasks(const_workload_tt);
	extern int workload_task(const_workload_tt, int);
	extern void workload_sort(workload_tt, enum workload_sorting);
	extern int *workload_sortmap(const_workload_tt);
	extern void workload_write(FILE *, const_workload_tt);
	extern workload_tt workload_read(FILE *);
	/**@}*/

#endif /* WORKLOAD_H_ */
