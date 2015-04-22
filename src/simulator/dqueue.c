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

#include <mylib/util.h>

#include "simulator.h"

/**
 * @brief Global delta queue.
 */
static struct dqueue head;

/**
 * @brief Creates the delta queue.
 * 
 * @details Creates the global delta queue.
 */
void dqueue_create(void)
{
	head.tid = 0;
	head.next = NULL;
}

/**
 * @brief Destroys the delta queue.
 * 
 * @details Destroys the global delta queue.
 */
void dqueue_destroy(void)
{
	/* House keeping. */
	while (head.next != NULL)
	{
		struct dqueue *node;
		
		node = head.next;
		head.next = node->next;
		
		free(node);
	}
}

/**
 * @brief Inserts a thread in the delta queue.
 * 
 * @details Inserts the thread identified by @p tid in the global delta queue
 *          with remaining time @p time;
 */
void dqueue_insert(unsigned tid, unsigned time)
{
	struct dqueue *tmp;
	struct dqueue *node;
	
	node = &head;
	
	/* Search insertion point. */
	while ((node->next != NULL) && (time >= node->next->remaining))
	{
		time -= node->next->remaining;
		node = node->next;
	}
	
	/* Create new node. */
	tmp = smalloc(sizeof(struct dqueue));
	tmp->tid = tid;
	tmp->remaining = time;
	tmp->next = node->next;
	
	/* Insert node in the delta queue. */
	node->next = tmp;
	
	/* Update remaining time. */
	for (node = tmp->next; node != NULL; node = node->next)
	{
		if (node->remaining == 0)
			continue;
		
		node->remaining -= time;
	}
}

/**
 * @brief Removes the first thread from the delta queue.
 * 
 * @details Removes the first thread in the global delta queue and returns it.
 * 
 * @param The ID of the first thread in the global delta queue is returned. If
 *        the delta queue is empty, zero is returned.
 */
unsigned dqueue_remove(void)
{
	unsigned tid;        /* Thread ID.               */
	struct dqueue *node; /* First node in the queue. */
	
	node = head.next;
	
	/* Empty delta queue. */
	if (node == NULL)
		return (0);
	
	tid = node->tid;
	
	/* Remove node. */
	head.next = node->next;
	free(node);
	
	return (tid);
}

/**
 * @brief Returns the next remaining time.
 * 
 * @details Returns the remaining time for the first thread in the global delta
 *          queue.
 * 
 * @returns The remaining time for the first thread in the global delta queue. 
 *          If the delta queue is empty, zero is returned.
 */
unsigned dqueue_next_timestamp(void)
{
	return ((head.next == NULL) ? 0 : head.next->remaining);
}
