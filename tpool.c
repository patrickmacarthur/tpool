/* tpool.c - a simple thread pool library built on POSIX threads
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
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with tpool.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>

#include "tpool.h"
#include "tpool_private.h"

/* A thread pool. */
struct tpool {
	int                     alive;
	struct task_queue       queue;
	pthread_mutex_t         tp_mutex;

	/* The set of threads in the pool. */
	unsigned                pool_size;
	unsigned                n_threads;
	uint32_t		flags;
};

/* Initializes a new thread pool at the address pointed to by tpool.  The
 * maxthreads argument specifies the maximum number of threads in this pool.
 * The flags argument specifies additional flags.  Currently no flags are
 * defined and the only acceptable value is UINT32_C(0).  Returns 0 on success;
 * on error, it returns a nonzero error number, and the contents of *tpool are
 * undefined.  This function may fail with EINVAL if an invalid value is given
 * for tpool, maxthreads, or flags. */
int
tpool_init(unsigned maxthreads, uint32_t flags, TPOOL **tpoolp)
{
	TPOOL *tpool;
	int errcode;

	if (!tpoolp || !maxthreads || flags) {
		errcode = EINVAL;
		goto exit;
	}

	if ((tpool = calloc(1, sizeof(*tpool))) == NULL) {
		errcode = errno;
		goto exit;
	}
	tpool->pool_size = maxthreads;
	tpool->alive = 1;
	tpool->flags = flags;
	if ((errcode = task_queue_init(&tpool->queue)) != 0) {
		goto fail0;
	}
	if ((errcode = pthread_mutex_init(&tpool->tp_mutex, NULL)) != 0) {
		goto fail1;
	}

	errcode = 0;
	*tpoolp = tpool;
	goto exit;
fail1:
	assert(errcode != 0);
	task_queue_destroy(&tpool->queue);
fail0:
	assert(errcode != 0);
	free(tpool);
exit:
	return errcode;
}

/* Destroys the thread pool at the address pointed to by *tpool.  The contents
 * of *tpool are undefined after calling this function.  Returns 0 on success;
 * on error, it returns a nonzero error number.  If EINVAL or EBUSY are
 * returned, the state of the thread pool will not be changed.  This function
 * may fail with EINVAL if the value specified by tpool is invalid.  This
 * function will fail with EBUSY if the thread pool has not been shut down, if
 * any thread in the thread pool is busy, or if there are any queued tasks. */
int
tpool_destroy(TPOOL *tpool)
{
	int errcode;

	if (!tpool) {
		errcode = EINVAL;
		goto exit;
	}

	if (tpool->alive || tpool->n_threads || tpool->queue.q_head) {
		errcode = EBUSY;
		goto exit;
	}

	errcode = 0;
exit:
	return errcode;
}


/* This is the main work function of a pool worker thread.  This thread will
 * loop as long as there are tasks in the queue, pulling tasks off the queue and
 * executing them. */
static void *
pool_worker(void *threadarg)
{
	void *(*func)(void *);
	void *taskarg;
	void *result;
	int flags;
	int errcode;
	TPOOL *tpool;
	FUTURE *future;
	pthread_detach(pthread_self());
	tpool = (TPOOL *)threadarg;

	for (;;) {
		if ((errcode = task_queue_remove(&tpool->queue, &func, &taskarg, &flags,
								&future)) == 0) {
			if (func == NULL) {
				pthread_mutex_lock(&tpool->tp_mutex);
				--tpool->n_threads;
				pthread_mutex_unlock(&tpool->tp_mutex);
				pthread_exit(NULL);
			}

			result = func(taskarg);
			if (flags & TASK_WANT_FUTURE) {
				future_set(future, result);
			}
		} else {
			pthread_mutex_lock(&tpool->tp_mutex);
			--tpool->n_threads;
			pthread_mutex_unlock(&tpool->tp_mutex);
			pthread_exit(NULL);
		}
	}

	return NULL;
}

/* This functions shuts down the thread pool gracefully.  After calling this
 * function, new tasks will be rejected, but all threads currently executing and
 * in the queue will continue.  When the queue is empty, each of the threads in
 * the pool will exit. */
void
tpool_shutdown(TPOOL *tpool)
{
	pthread_mutex_lock(&tpool->tp_mutex);
	tpool->alive = 0;
	pthread_mutex_unlock(&tpool->tp_mutex);
}

/* This function adds a task to the thread pool, starting a thread for it if
 * possible.  The task to add is defined as a function and an argument to that
 * function.  Currently, this function may return a value but its return value
 * is ignored.
 *
 * On success, this function returns 0.  On failure, it returns one
 * of the follwing error codes:
 *   ECANCELED: the thread pool has been shut down and is not accepting new
 * tasks
 *   EINVAL: the passed-in task func is NULL
 *   ENOMEM: memory could not be allocated for the new task
 */
int
tpool_submit(TPOOL *tpool, void *(*func)(void *), void *taskarg, int flags,
							FUTURE **pfuture)
{
	int errcode;
	pthread_t threadid;

	if (func == NULL || ((flags & TASK_WANT_FUTURE) && pfuture == NULL)) {
		return EINVAL;
	}

	if (!tpool->alive) {
		return ECANCELED;
	}

	if (flags & TASK_WANT_FUTURE) {
		if ((*pfuture = calloc(1, sizeof(**pfuture))) == NULL) {
			return errno;
		}
		if ((errcode = future_init(*pfuture)) != 0) {
			free(*pfuture);
			return errno;
		}
		if ((errcode = task_queue_add(&tpool->queue, func, taskarg, flags, *pfuture))
									!= 0) {
			return errcode;
		}
	} else {
		if ((errcode = task_queue_add(&tpool->queue, func, taskarg, flags, NULL))
									!= 0) {
			return errcode;
		}
	}

	pthread_mutex_lock(&tpool->tp_mutex);
	if (tpool->n_threads < tpool->pool_size) {
		errcode = pthread_create(&threadid, NULL, &pool_worker, tpool);
		if (errcode != 0 && tpool->n_threads == 0) {
			return errcode;
		} else {
			++tpool->n_threads;
		}
	}
	pthread_mutex_unlock(&tpool->tp_mutex);

	return 0;
}

/* vim: set shiftwidth=8 tabstop=8 noexpandtab : */
