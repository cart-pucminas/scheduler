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

#include <assert.h>
#include <stdbool.h>

#include <mylib/util.h>
#include <mylib/dqueue.h>

/**
 * @brief Delta queue node.
 */
struct dqnode
{
	void *obj;           /**< Underlying object.             */
	int counter;         /**< Delta queue counter.           */
	struct dqnode *next; /**< Next objet in the delta queue. */
};

/**
 * @brief Queue.
 */
struct dqueue
{
	int size;            /**< Current delta queue size. */
	struct dqnode head;  /**< Dummy head node.          */
};

/*====================================================================*
 * QUEUE NODE                                                         *
 *====================================================================*/

/**
 * @brief Creates a delta queue node.
 * 
 * @param obj Object to store in the node.
 * 
 * @returns A delta queue node.
 */
static inline struct dqnode *dqnode_create(void *obj, int counter)
{
	struct dqnode *node;
	
	node = smalloc(sizeof(struct dqnode));
	
	/* Initialize delta queue node. */
	node->obj = obj;
	node->counter = counter;
	node->next = NULL;
	
	return (node);
}

/**
 * @brief Destroys a delta queue node.
 * 
 * @param node Target delta queue node.
 */
static inline void dqnode_destroy(struct dqnode *node)
{
	free(node);
}

/*====================================================================*
 * QUEUE                                                              *
 *====================================================================*/

/**
 * @brief Creates a delta queue.
 * 
 * @returns A delta queue.
 */
struct dqueue *dqueue_create(void)
{
	struct dqueue *q;
	
	q = smalloc(sizeof(struct dqueue));
	
	/* Initialize delta queue. */
	q->size = 0;
	q->head.next = NULL;
	
	return (q);
}

/**
 * @brief Destroys a delta queue.
 * 
 * @param q Target delta queue.
 */
void dqueue_destroy(struct dqueue *q)
{
	/* Sanity check. */
	assert(q != NULL);
	
	/* House keeping. */
	while (!dqueue_empty(q))
		dqueue_remove(q);
	free(q);
}

/**
 * @brief Returns the size of a delta queue.
 *
 * @param q Target delta queue.
 *
 * @returns The current size of the target delta queue,
 */
int dqueue_size(const struct dqueue *q)
{
	/* Sanity check. */
	assert(q != NULL);

	return (q->size);
}

/**
 * @brief Asserts if a delta queue is empty.
 *
 * @param q Target delta queue.
 *
 * @returns True if the target delta queue is empty and false otherwise.
 */
bool dqueue_empty(const struct dqueue *q)
{
	return (dqueue_size(q) == 0);
}

/**
 * @brief Returns the counter of the front object in a delta queue.
 *
 * @param Target delta queue.
 *
 * @returns The counter of the front object in the target delta queue.
 */
int dqueue_next_counter(const struct dqueue *q)
{
	/* Sanity check. */
	assert(q != NULL);

	/* Empty delta queue. */
	if (q->size == 0)
		return (-1);

	return (q->head.next->counter);
}

/**
 * @brief Inserts an object in a delta queue.
 * 
 * @param q       Target delta queue.
 * @param obj     Target object.
 * @param counter Object's counter.
 */
void dqueue_insert(struct dqueue *q, void *obj, int counter)
{
	struct dqnode *tmp;
	struct dqnode *node;

	/* Sanity check. */
	assert(q != NULL);
	assert(obj != NULL);
	assert(counter >= 0);

	node = &q->head;

	/* Search insertion point. */
	while ((node->next != NULL) && (counter > node->next->counter))
	{
		counter -= node->next->counter;
		node = node->next;
	}

	/* Create new node. */
	tmp = smalloc(sizeof(struct dqueue));
	tmp->obj = obj;
	tmp->counter = counter;

	/* Insert object. */
	tmp->next = node->next;
	node->next = tmp;

	/* Update counter. */
	for (node = tmp->next; node != NULL; node = node->next)
	{
		assert(node->counter >= 0);

		if (node->counter == 0)
			continue;

		node->counter -= counter;
		break;
	}

	q->size++;
}

/**
 * @brief Removes an object from a delta queue.
 * 
 * @param q Target delta queue.
 * 
 * @returns The object in the front of the delta queue.
 */
void *dqueue_remove(struct dqueue *q)
{
	void *obj;          /* Object in the first node. */
	struct dqnode *node; /* First node.               */
	
	/* Sanity check. */
	assert(q != NULL);
	assert(q->size != 0);
	
	/* Unlink node. */
	node = q->head.next;
	q->head.next = node->next;
	q->size--;
	
	/* Get object. */
	obj = node->obj;
	dqnode_destroy(node);
	
	return (obj);
}

