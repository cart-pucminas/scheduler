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

#ifndef _COMMON_H_
#define _COMMON_H_

	/**
	 * @brief Number of supported probability distributions.
	 */
	#define NDISTRIBUTIONS 3

	/**
	 * @name Probability Distributions.
	 */
	/**@{*/
	#define DISTRIBUTION_RANDOM  0 /**< Random distribution.  */
	#define DISTRIBUTION_NORMAL  1 /**< Normal distribution.  */
	#define DISTRIBUTION_POISSON 2 /**< Poisson distribution. */
	/**@}*/
	
	/* Forward definitions. */
	extern double *create_tasks(unsigned, unsigned);
	
	/* Forward definitions. */
	extern const char *distributions[NDISTRIBUTIONS];

#endif /* _COMMON_H_ */
