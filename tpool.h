/* tpool.h
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

#ifndef TPOOL_H
#define TPOOL_H

#include <stdint.h>
#include <unistd.h>

#if !defined(_POSIX_VERSION) || _POSIX_VERSION < 200112L
#error tpool.h requires a system that conforms to IEEE POSIX.1-2001 or a \
later version.
#endif

#define TASK_WANT_FUTURE (1 << 0)

/* Represents a unit of work in the thread pool. */
struct tpool_task {
	void *(*func)(void *);
	void *arg;
	int flags;
};


/* Represents a value that will be known at some point in the future. */
typedef struct future FUTURE;

/* Represents a thread pool. */
typedef struct tpool TPOOL;

int
tpool_init(unsigned maxthreads, uint32_t flags, TPOOL **tpoolp);

int
tpool_destroy(TPOOL *tpool);

void
tpool_shutdown(TPOOL *tpool);

int
tpool_submit(TPOOL *tpool, void *(*func)(void *), void *taskarg, int flags,
							FUTURE **pfuture);

void *
future_get(FUTURE *future);

int
future_destroy(FUTURE *future);

#endif
/* vim: set shiftwidth=8 tabstop=8 noexpandtab : */
