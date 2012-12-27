/* future.c - provides a type whose value will be known at a future time
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

#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <errno.h>

#include "tpool.h"
#include "tpool-private.h"

/* Initializes a future, assuming that it has already been allocated.  Only
 * meant to be called internally.  External applications should get a future
 * only through calls to tpool_submit(). */
int
future_init(FUTURE *future)
{
	int errcode;

	if ((errcode = pthread_mutex_init(&future->f_mutex, NULL)) != 0) {
		goto exit0;
	}

	if ((errcode = pthread_cond_init(&future->f_cond, NULL)) != 0) {
		goto fail1;
	}

	errcode = 0;
	goto exit0;

fail1:
	pthread_mutex_destroy(&future->f_mutex);

exit0:
	return errcode;
}

/* Sets the value of the future.  Should only be called from the worker threads.
 */
void
future_set(FUTURE *future, void *value)
{
	/* if this fails, setting of f_ready should be "atomic" anyways, so
	 * we're probably all right */
	pthread_mutex_lock(&future->f_mutex);

	future->f_value = value;
	future->f_ready = 1;
	pthread_mutex_unlock(&future->f_mutex);
	pthread_cond_signal(&future->f_cond);
}

/* Returns a future value.  Returns NULL if there was an error; otherwise
 * returns the value held within the future.  If TPOOL_WAIT is set in flags,
 * this function will block until the value is ready.  If TPOOL_WAIT is not set
 * in flags, this function will return NULL and set errno to EAGAIN if the value
 * is not ready. */
void *
future_get(FUTURE *future, int flags)
{
	void *retval;
	int errcode;

	if ((errcode = pthread_mutex_lock(&future->f_mutex)) != 0) {
		errno = errcode;
		return NULL;
	}

	/* If we don't have the future value, don't block unless TPOOL_WAIT is
	 * set in flags. */
	if (!(flags & TPOOL_WAIT) && !future->f_ready) {
		pthread_mutex_unlock(&future->f_mutex);
		errno = EAGAIN;
		return NULL;
	}

	/* We either have the future value or TPOOL_WAIT is set in flags. */
	while (!future->f_ready) {
		pthread_cond_wait(&future->f_cond, &future->f_mutex);
	}

	retval = future->f_value;
	pthread_mutex_unlock(&future->f_mutex);

	return retval;
}

/* Destroys a future object.  Should only be called after the value is ready and
 * has been retrieved.  After destroying the future, no attempt should be made
 * to use it again.  On success, returns 0.  On failure, returns the error code.
 * Will return EBUSY if the future value is not yet ready. */
int
future_destroy(FUTURE *future)
{
	int errcode;
	int retval;

	pthread_mutex_lock(&future->f_mutex);
	if (!future->f_ready) {
		retval = EBUSY;
		goto fail1;
	}

	retval = 0;
	if ((errcode = pthread_cond_destroy(&future->f_cond)) != 0) {
		retval = errcode;
	}
	pthread_mutex_unlock(&future->f_mutex);
	if ((errcode = pthread_mutex_destroy(&future->f_mutex)) != 0) {
		retval = errcode;
	}
	free(future);
	goto exit;

fail1:
	assert(retval != 0);
	pthread_mutex_unlock(&future->f_mutex);

exit:
	return retval;
}

/* vim: set shiftwidth=8 tabstop=8 noexpandtab : */
