/* test.c
 *
 * Patrick MacArthur <pio3@wildcats.unh.edu>
 */

#define _POSIX_C_SOURCE 200809L


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "tpool.h"

TPOOL	*tpool	= NULL;

struct param
{
	unsigned taskno;
	bool have_future;
};

void *
test_task(void *arg)
{
	struct param *p = (struct param *)arg;
	printf("This is thread %u%s\n", p->taskno,
			p->have_future ? "" : " (no future)");
	fflush(stdout);
	return (void *)(uintptr_t)(p->taskno);
}

FUTURE *
submit_task(void *(*func)(void *), int flags)
{
	static unsigned taskno_next = 1;
	FUTURE *future;
	struct tpool_task task;
	struct param *params;
	int errcode;

	if ((params = malloc(sizeof(*params))) == NULL) {
		return NULL;
	}
	params->taskno = taskno_next++;
	params->have_future = flags & TASK_WANT_FUTURE;

	task.func = func;
	task.arg = (void *)params;
	task.flags = flags;
	if ((errcode = tpool_submit(tpool, &task, &future)) != 0) {
		errno = errcode;
		return NULL;
	} else {
		return future;
	}
}

int
main()
{
	int errcode;
	FUTURE *f1;
	FUTURE *f2;
	void *value;

	if ((errcode = tpool_init(2, UINT32_C(0), &tpool)) != 0) {
		fprintf(stderr, "tpool_init: %s\n", strerror(errcode));
	}
	if ((f1 = submit_task(&test_task, TASK_WANT_FUTURE)) == NULL) {
		fprintf(stderr, "submit task: %s\n", strerror(errno));
	}
	if ((f2 = submit_task(&test_task, TASK_WANT_FUTURE)) == NULL) {
		fprintf(stderr, "submit task: %s\n", strerror(errno));
	}
	errno = 0;
	submit_task(&test_task, UINT32_C(0));
	if (errno != 0) {
		fprintf(stderr, "submit task: %s\n", strerror(errno));
	}
	value = future_get(f1);
	printf("Task 1 finished; returned value %p\n", value);
	fflush(stdout);
	future_destroy(f1);
	value = future_get(f2);
	printf("Task 2 finished; returned value %p\n", value);
	fflush(stdout);
	future_destroy(f2);
	tpool_shutdown(tpool);
	if (tpool_destroy(tpool) == 0) {
		printf("Thread pool destroyed\n");
	}

	pthread_exit(NULL);
	return 0; /* should never reach this */
}

/* vim: set shiftwidth=8 tabstop=8 noexpandtab : */
