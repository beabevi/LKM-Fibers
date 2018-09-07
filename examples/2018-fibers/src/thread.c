#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "thread.h"


__thread unsigned int tid;

static unsigned int thread_counter = 0;

// Spawn a new thread
static void *__helper_create_thread(void *arg) {

	struct _helper_thread *real_arg = (struct _helper_thread *)arg;

	// Get a unique local thread id...
	unsigned int old_counter;

	do {
		old_counter = thread_counter;
		tid = old_counter + 1;
	} while(!__sync_bool_compare_and_swap(&thread_counter, old_counter, tid));

	real_arg->start_routine(real_arg->arg);
	free(arg);
	return NULL;
}

// Create an arbitrary number of threads starting from the same entry point
void create_threads(unsigned short int n, void *(*start_routine)(void*), void *arg) {

	int i;

	struct _helper_thread *new_arg = malloc(sizeof(struct _helper_thread));
	new_arg->start_routine = start_routine;
	new_arg->arg = arg;

	for(i = 0; i < n; i++) {
		new_thread(__helper_create_thread, (void *)new_arg);
	}
}

