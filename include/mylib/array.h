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

#ifndef ARRAY_H_
#define ARRAY_H_

	/**
	 * @brief Opaque pointer to an array.
	 */
	typedef struct array * array_tt;

	/**
	 * @brief Constant pointer to an array.
	 */
	typedef const struct array * const_array_tt;

	/**
	 * @name Operations on Array
	 */
	/**@{*/
	extern array_tt array_create(int);
	extern void array_destroy(array_tt);
	extern int array_size(const_array_tt);
	extern void array_set(array_tt, int, const void *);
	extern void *array_get(const_array_tt, int);
	extern void array_shuffle(array_tt);
	/**@}*/

#endif /* ARRAY_H_ */
