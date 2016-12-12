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

#ifndef QUEUE_H_
#define QUEUE_H_

	#include <stdbool.h>

	/**
	 * @brief Opaque pointer to a queue.
	 */
	typedef struct queue * queue_tt;

	/**
	 * @brief Constant opaque pointer to a queue.
	 */
	typedef const struct queue * const_queue_tt;

	/**
	 * @name Operations on Queues
	 */
	/**@{*/
	extern queue_tt queue_create(void);
	extern void queue_destroy(queue_tt);
	extern int queue_size(const_queue_tt);
	extern bool queue_empty(const_queue_tt);
	extern void queue_insert(queue_tt, void *);
	extern void *queue_remove(queue_tt);
	/**@}*/

#endif /* QUEUE_H_ */
