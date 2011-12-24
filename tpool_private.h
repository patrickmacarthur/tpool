/* tpool_private.h
 *
 * Patrick MacArthur <generalpenguin89@gmail.com>
 */

#ifndef TPOOL_PRIVATE_H
#define TPOOL_PRIVATE_H

struct future {
	int             f_ready;
	void            *f_value;
	pthread_mutex_t f_mutex;
	pthread_cond_t  f_cond;
};

int
future_init(FUTURE *future);

void
future_set(FUTURE *future, void *value);

/* Represents a FIFO queue of tasks for the thread pool.  Any thread may
 * add a task to the tail.  Worker threads will pull a task off of the
 * head when they become available. */
struct task_queue {
	struct tpool_task *q_head;
	struct tpool_task *q_tail;
	pthread_mutex_t   q_mutex;
};

int
task_queue_init(struct task_queue *queue);

int
task_queue_destroy(struct task_queue *queue);

int
task_queue_add(struct task_queue *queue,
				void *(*func)(void *), void *taskarg, int flags,
								FUTURE *future);

int
task_queue_remove(struct task_queue *queue,
			void *(**func)(void *), void **taskarg, int *flags,
							FUTURE **pfuture);
/* Represents a unit of work in the thread pool. */
struct tpool_task {
	void *(*func)(void *);
	void *taskarg;
	FUTURE *future;
	int flags;
	struct tpool_task *next;
};

#endif
/* vim: set shiftwidth=8 tabstop=8 noexpandtab : */
