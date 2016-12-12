/*
 * Copyright(C) 2016 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This file is part of MyLib.
 *
 * MyLib is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * MyLib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with MyLib; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef DQUEUE_H_
#define DQUEUE_H_

	#include <stdbool.h>

	/**
	 * @brief Opaque pointer to a delta queue.
	 */
	typedef struct dqueue * dqueue_tt;

	/**
	 * @brief Constant opaque pointer to a delta queue.
	 */
	typedef const struct dqueue * const_dqueue_tt;

	/**
	 * @name Operations on Delta Queues
	 */
	/**@{*/
	extern dqueue_tt dqueue_create(void);
	extern void dqueue_destroy(dqueue_tt);
	extern int dqueue_size(const_dqueue_tt);
	extern bool dqueue_empty(const_dqueue_tt);
	extern int dqueue_next_counter(const_dqueue_tt);
	extern void dqueue_insert(dqueue_tt, void *, int);
	extern void *dqueue_remove(dqueue_tt);
	/**@}*/

#endif /* DQUEUE_H_ */
