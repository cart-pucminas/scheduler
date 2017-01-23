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

#include <mylib/util.h>

#include <thread.h>

/**
 * @brief Thread.
 */
struct thread
{
	int tid;      /**< Identification number,   */
	int wtotal;   /**< Total assigned workload. */
	int capacity; /**< Processing capacity.     */
};

/**
 * @brief Next available thread identification number.
 */
static int next_tid = 0;

/**
 * @brief Creates a thread.
 *
 * @param capacity Processing capacity.
 *
 * @returns A thread.
 */
struct thread *thread_create(int capacity)
{
	struct thread *t;

	/* Sanity check. */
	assert((capacity >= 1) && (capacity <= 100));

	t = smalloc(sizeof(struct thread));

	t->tid = next_tid++;
	t->wtotal = 0;
	t->capacity = capacity;

	return (t);
}

/**
 * @brief Destroys a thread.
 *
 * @param t Target thread.
 */
void thread_destroy(struct thread *t)
{
	/* Sanity check. */
	assert(t != NULL);

	free(t);
}

/**
 * @brief Gets the ID of a thread.
 *
 * @param t Target thread.
 *
 * @returns The ID of the target thread.
 */
int thread_gettid(const struct thread *t)
{
	/* Sanity check. */
	assert(t != NULL);

	return (t->tid);
}

/**
 * @brief Returns the total workload assigned to a thread.
 *
 * @param t Target thread.
 *
 * @returns The total workload assigned to a thread.
 */
double thread_wtotal(const struct thread *t)
{
	/* Sanity check. */
	assert(t != NULL);

	return (((double)t->wtotal)*t->capacity);
}

/**
 * @brief Assigns a workload to a thread.
 *
 * @param t     Target thread.
 * @param wsize Work size.
 *
 * @returns Required processing time.
 */
int thread_assign(struct thread *t, int wsize)
{
	/* Sanity check. */
	assert(t != NULL);

	t->wtotal += wsize;

	return (t->capacity*wsize);
}

/**
 * @brief Returns the capacity of a thread.
 *
 * @param t target thread.
 *
 * @returns The capacity of the target thread.
 */
int thread_capacity(const struct thread *t)
{
	/* Sanity check. */
	assert(t != NULL);

	return (t->capacity);
}
