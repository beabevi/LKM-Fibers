#pragma once

#include <stdbool.h>
#include <pthread.h>

pthread_t os_tid;

extern __thread unsigned int tid;

struct _helper_thread {
	void *(*start_routine)(void*);
	void *arg;
};

#define new_thread(entry, arg)	pthread_create(&os_tid, NULL, entry, arg)

void create_threads(unsigned short int n, void *(*start_routine)(void*), void *arg);
