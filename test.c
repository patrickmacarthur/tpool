/* test.c
 *
 * Patrick MacArthur <pio3@wildcats.unh.edu>
 */

#define _POSIX_C_SOURCE 200809L


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "tpool.h"

void *
thread1(void *threadarg)
{
	(void)threadarg;
	printf("This is thread 1\n");
	fflush(stdout);
	return (void *)(1);
}

void *
thread2(void *threadarg)
{
	(void)threadarg;
	printf("This is thread 2\n");
	fflush(stdout);
	return (void *)(2);
}

int
main()
{
	int errcode;
	TPOOL *tpool;
	FUTURE *f1;
	FUTURE *f2;
	void *value;

	if ((errcode = tpool_init(2, UINT32_C(0), &tpool)) != 0) {
		fprintf(stderr, "tpool_init: %s\n", strerror(errcode));
	}
	if ((errcode = tpool_submit(tpool, &thread1, NULL, TASK_WANT_FUTURE, &f1)) != 0) {
		fprintf(stderr, "submit task 1: %s\n", strerror(errcode));
	}
	if ((errcode = tpool_submit(tpool, &thread2, NULL, TASK_WANT_FUTURE, &f2)) != 0) {
		fprintf(stderr, "submit task 2: %s\n", strerror(errcode));
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
	tpool_destroy(tpool);

	return 0;
}

/* vim: set shiftwidth=8 tabstop=8 noexpandtab : */
