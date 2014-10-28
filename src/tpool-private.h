/* tpool_private.h
 *
 * Patrick MacArthur <contact@patrickmacarthur.net>
 */

#ifndef TPOOL_PRIVATE_H
#define TPOOL_PRIVATE_H

#define TPOOL_EXPORT __attribute__ ((visibility("default")))

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
	struct task_node  *q_head;
	struct task_node  *q_tail;
	pthread_mutex_t   q_mutex;
};

int
task_queue_init(struct task_queue *queue);

int
task_queue_destroy(struct task_queue *queue);

int
task_queue_add(struct task_queue *queue, struct tpool_task *task,
								FUTURE *future);

int
task_queue_remove(struct task_queue *queue, struct tpool_task **task,
							FUTURE **pfuture);

/* Represents a unit of work in the thread pool. */
struct task_node {
	struct tpool_task *task;
	FUTURE *future;
	struct task_node *next;
};

#endif
/* vim: set shiftwidth=8 tabstop=8 noexpandtab : */
