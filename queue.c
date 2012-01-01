/* queue.c - a task queue for the tpool library
 *
 * Copyright (c) 2011 Patrick MacArthur
 *
 * This file is part of tpool.
 *
 * tpool is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * tpool is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with tpool.  If not, see * <http://www.gnu.org/licenses/>.
 */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tpool.h"
#include "tpool_private.h"

int
task_queue_init(struct task_queue *queue)
{
	int errcode;

	assert(queue != NULL);
	memset(queue, 0, sizeof(*queue));
	if ((errcode = pthread_mutex_init(&queue->q_mutex, NULL)) != 0) {
		goto exit;
	}

	errcode = 0;
exit:
	return errcode;
}

int
task_queue_destroy(struct task_queue *queue)
{
	int errcode;

	assert(queue != NULL);
	if (queue->q_head) {
		errcode = EBUSY;
		goto exit;
	}
	pthread_mutex_destroy(&queue->q_mutex);

	errcode = 0;
exit:
	return errcode;
}

/* Adds a task to the tail of the queue.  This will return 0 if successful, and
 * an appropriate error code if it is not succesful.  The error codes defined
 * are EINVAL if an argument is invalid or ENOMEM if a new task could not be
 * allocated. */
int
task_queue_add(struct task_queue *queue, struct tpool_task *task,
								FUTURE *future)
{
	struct task_node *node;
	struct tpool_task *copy;
	int errcode;
	assert(task != NULL);
	if (task->flags & TASK_WANT_FUTURE) {
		assert((task->flags & TASK_WANT_FUTURE) && future != NULL);
	}

	if (!(node = calloc(1, sizeof(*node)))) {
		return errno;
	}

	/* We need to take a copy of the user's structure since it might have
	 * been allocated from stack memory. */
	assert(sizeof(*task) == sizeof(*copy));
	if (!(copy = malloc(sizeof(*copy)))) {
		return errno;
	}
	memcpy(copy, task, sizeof(*copy));

	/* Set up the node.  Fields have already been initialized to 0 at this
	 * point. */
	node->task = copy;
	node->future = future;

	if ((errcode = pthread_mutex_lock(&queue->q_mutex)) != 0) {
		fprintf(stderr, "Error locking task queue (add): %s\n",
							strerror(errcode));
		return errcode;
	} else {
		/* Add the node to the tail of the queue structure */
		if (queue->q_head == NULL) {
			queue->q_head = queue->q_tail = node;
		} else {
			queue->q_tail->next = node;
			queue->q_tail = node;
		}
		pthread_mutex_unlock(&queue->q_mutex);
		return 0;
	}
}

/* Removes a task from the head of the queue.  Returns 0 on if there is no
 * error.  If there is a task in the queue, places the function in func and the
 * argument in taskarg; otherwise, sets func to NULL.  On error, prints a
 * message to stderr and returns an appropriate error code, leaving func and
 * taskarg undefined. */
int
task_queue_remove(struct task_queue *queue, struct tpool_task **task,
							FUTURE **pfuture)
{
	struct task_node *node;
	int errcode;
	assert(task != NULL && pfuture != NULL);

	if ((errcode = pthread_mutex_lock(&queue->q_mutex)) != 0) {
		fprintf(stderr, "Error locking task queue (remove): %s\n",
							strerror(errcode));
		return errcode;
	} else {
		if (queue->q_head == NULL) {
			node = NULL;
		} else {
			node = queue->q_head;
			queue->q_head = node->next;
		}
		pthread_mutex_unlock(&queue->q_mutex);

		if (node != NULL) {
			*task = node->task;
			*pfuture = node->future;
		} else {
			*task = NULL;
		}
		return 0;
	}
}

/* vim: set shiftwidth=8 tabstop=8 noexpandtab : */
