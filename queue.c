/* queue.c
 *
 * Patrick MacArthur <generalpenguin89@gmail.com>
 *
 * A task queue for a simple thread pool library.
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
	if ((errcode = pthread_mutex_destroy(&queue->q_mutex)) != 0) {
		goto exit;
	}

	errcode = 0;
exit:
	return errcode;
}

/* Adds a task to the tail of the queue.  This will return 0 if successful, and
 * an appropriate error code if it is not succesful.  The error codes defined
 * are EINVAL if an argument is invalid or ENOMEM if a new task could not be
 * allocated. */
int
task_queue_add(struct task_queue *queue,
		void *(*func)(void *), void *taskarg, int flags, FUTURE *future)
{
	struct tpool_task *task;
	int errcode;
	assert(func != NULL);
	if (flags & TASK_WANT_FUTURE) {
		assert((flags & TASK_WANT_FUTURE) && future != NULL);
	}

	if (!(task = calloc(1, sizeof(*task)))) {
		return errno;
	}

	task->func = func;
	task->taskarg = taskarg;
	task->flags = flags;
	task->future = future;

	if ((errcode = pthread_mutex_lock(&queue->q_mutex)) != 0) {
		fprintf(stderr, "Could not lock task queue: %s\n",
							strerror(errcode));
		return errcode;
	} else {
		if (queue->q_head == NULL) {
			queue->q_head = queue->q_tail = task;
		} else {
			queue->q_tail->next = task;
			queue->q_tail = task;
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
task_queue_remove(struct task_queue *queue,
			void *(**func)(void *), void **taskarg, int *flags,
							FUTURE **pfuture)
{
	struct tpool_task *task;
	int errcode;
	assert(func != NULL && taskarg != NULL);

	if ((errcode = pthread_mutex_lock(&queue->q_mutex)) != 0) {
		fprintf(stderr, "Could not lock task queue: %s\n",
							strerror(errcode));
		return errcode;
	} else {
		if (queue->q_head == NULL) {
			task = NULL;
		} else {
			task = queue->q_head;
			queue->q_head = task->next;
		}
		pthread_mutex_unlock(&queue->q_mutex);

		if (task != NULL) {
			*func = task->func;
			*taskarg = task->taskarg;
			*flags = task->flags;
			*pfuture = task->future;
		} else {
			*func = NULL;
			*taskarg = NULL;
		}
		return 0;
	}
}

/* vim: set shiftwidth=8 tabstop=8 noexpandtab : */
