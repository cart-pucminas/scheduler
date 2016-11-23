/*
 * Copyright(C) 2015-2016 Pedro H. Penna <pedrohenriquepenna@gmail.com>
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

#include <stdlib.h>

/**
 * @brief Shuffling seed.
 */
static unsigned seed = 0;

/**
 * @brief Greater than.
 * 
 * @param a1 First element.
 * @param a2 Second element.
 * 
 * @returns One if @p a1 is greater than @p a2 and minus one otherwise.
 */
static int greater(const void *a1, const void *a2)
{
	return ((*((unsigned *)a1) > *((unsigned *)a2)) ? 1 : -1);
}

/**
 * @brief Less than.
 * 
 * @param a1 First element.
 * @param a2 Second element.
 * 
 * @returns One if @p a1 is less than @p a2 and minus one otherwise.
 */
static int less(const void *a1, const void *a2)
{
	return ((*((unsigned *)a1) < *((unsigned *)a2)) ? 1 : -1);
}

/**
 * @brief Shuffles an array.
 *
 * @details Shuffles the target array pointed to by @p array. The
 * shuffling seed may be changed by array_shuffle_seed().
 * 
 * @param array Target array.
 * @param n     Size of the target array.
 */
static void array_shuffle(unsigned *array, unsigned n)
{
	srand(seed);
	
	/* Shuffle array. */
	for (unsigned i = 0; i < n - 1; i++)
	{
		unsigned j;   /* Shuffle index.  */
		unsigned tmp; /* Temporary data. */
		
		j = i + rand()/(RAND_MAX/(n - i) + 1);
			
		tmp = array[i];
		array[i] = array[j];
		array[j] = tmp;
	}
}

/**
 * @brief Sorts an array.
 *
 * @details Sorts the target array pointed to by @p array. The array
 * may be sorted in descending order, shuffled, or sorted in ascending
 * order depending if @p type is less than, equal to, or greater than
 * zero, respectively.
 *
 * @param array Target array.
 * @param n     Size of the target array.
 * @param type  Sorting type.
 */
void array_sort(unsigned *array, unsigned n, int type)
{
	/* Descending. */
	if (type < 0)
		qsort(array, n, sizeof(unsigned), less);

	/* Random. */
	else if (type == 0)
		array_shuffle(array, n);

	/* Ascending. */
	else
		qsort(array, n, sizeof(unsigned), greater);
}

/**
 * @brief Sets the shuffling seed.
 *
   @details Sets the shuffling seed for array_sort().
 *
 * @param newseed New shuffling seed.
 */
void array_shuffle_seed(unsigned newseed)
{
	seed = newseed;
}
