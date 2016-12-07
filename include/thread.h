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

#ifndef THREAD_H_
#define THREAD_H_

	/**
	 * @brief Opaque pointer to a thread.
	 */
	typedef struct thread * thread_tt;

	/**
	 * @brief Constant opaque pointer to a thread.
	 */
	typedef const struct thread * const_thread_tt;

	/**
	 * @name Operations on Thread
	 */
	/**@{*/
	extern thread_tt thread_create(void);
	extern void thread_destroy(thread_tt);
	extern int thread_gettid(const_thread_tt);
	extern int thread_wtotal(const_thread_tt);
	extern void thread_assign(thread_tt, int);
	/**@}*/

#endif /* THREAD_H_ */
