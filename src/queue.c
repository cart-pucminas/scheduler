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

#include <util.h>
#include <queue.h>

/**
 * @brief Queue node.
 */
struct qnode
{
	void *obj;          /** Underlying object.        */
	struct qnode *next; /**< Next objet in the queue. */
};

/**
 * @brief Queue.
 */
struct queue
{
	int size;           /**< Current queue size. */
	struct qnode head;  /**< Dummy head node.    */
	struct qnode *tail; /**< Tail node.          */
};

/*====================================================================*
 * QUEUE NODE                                                         *
 *====================================================================*/

/**
 * @brief Creates a queue node.
 * 
 * @param obj Object to store in the node.
 * 
 * @returns A queue node.
 */
static inline struct qnode *qnode_create(void *obj)
{
	struct qnode *node;
	
	node = smalloc(sizeof(struct qnode));
	
	/* Initialize queue node. */
	node->obj = obj;
	node->next = NULL;
	
	return (node);
}

/**
 * @brief Destroys a queue node.
 * 
 * @param node Target queue node.
 */
static inline void qnode_destroy(struct qnode *node)
{
	free(node);
}

/*====================================================================*
 * QUEUE                                                              *
 *====================================================================*/

/**
 * @brief Creates a queue.
 * 
 * @returns A queue.
 */
struct queue *queue_create(void)
{
	struct queue *q;
	
	q = smalloc(sizeof(struct queue));
	
	/* Initialize queue. */
	q->size = 0;
	q->head.next = NULL;
	q->tail = &q->head;
	
	return (q);
}

/**
 * @brief Destroys a queue.
 * 
 * @param q Target queue.
 */
void queue_destroy(struct queue *q)
{
	/* Sanity check. */
	assert(q != NULL);
	
	/* House keeping. */
	while (!queue_empty(q))
		queue_remove(q);
	free(q);
}

/**
 * @brief Returns the size of a queue.
 *
 * @param q Target queue.
 *
 * @returns The current size of the target queue,
 */
int queue_size(const struct queue *q)
{
	/* Sanity check. */
	assert(q != NULL);

	return (q->size);
}

/**
 * @brief Asserts if a queue is empty.
 *
 * @param q Target queue.
 *
 * @returns True if the target queue is empty and false otherwise.
 */
bool queue_empty(const struct queue *q)
{
	return (queue_size(q) == 0);
}

/**
 * @brief Inserts an object in a queue.
 * 
 * @param q   Target queue.
 * @param obj Target object.
 */
void queue_insert(struct queue *q, void *obj)
{
	struct qnode *node;
	
	/* Sanity check. */
	assert(q != NULL);
	assert(obj != NULL);
	
	/* Link object. */
	node = qnode_create(obj);
	q->tail->next = node;
	q->tail = node;
	q->size++;
}

/**
 * @brief Removes an object from a queue.
 * 
 * @param q Target queue.
 * 
 * @returns The object in the front of the queue.
 */
void *queue_remove(struct queue *q)
{
	void *obj;          /* Object in the first node. */
	struct qnode *node; /* First node.               */
	
	/* Sanity check. */
	assert(q != NULL);
	assert(q->size != 0);
	
	/* Unlink node. */
	node = q->head.next;
	q->head.next = node->next;
	q->size--;
	
	/* Update tail node. */
	if (q->size == 0)
		q->tail = &q->head;
	
	/* Get object. */
	obj = node->obj;
	qnode_destroy(node);
	
	return (obj);
}
