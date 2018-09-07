#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include "thread.h"
#include "ult.h"
#include "model.h"
#include "timer.h"
#include "../fiber.h"

volatile unsigned int completed_fibers;
static unsigned int num_fibers;
static void **fibers;
static unsigned long long init_millis;
static unsigned long long exec_millis;
static volatile bool init_complete;

// Pick fibers randomly. This might return a fiber which is
// currently scheduled on another thread.
static int get_random_fiber(void) {
	return random() % num_fibers;
}

// Let's switch to another fiber!
void schedule(int sig) {
	(void)sig;
	unsigned int f;
	f = get_random_fiber();
	alarm(1);
	SwitchToFiber(fibers[f]);
}

// This is the code in which each processing fiber will leave.
// We'll never leave this function anyhow, at the end the application
// terminates here.
static void main_loop(void *args) __attribute__ ((noreturn));
static void main_loop(void *args) {
	int q_idx, state_idx;
	long long ret;
	unsigned int millis;
	msg_t *event;
	timer fiber_runtime;
	unsigned int id = (unsigned int)args;
	
	// Initialize the current fiber's work
	q_idx = FlsAlloc();
	state_idx = FlsAlloc();
	
	if(q_idx == -1 || state_idx == -1) {
		fprintf(stderr, "No more FLS storage available, aborting...\n");
		exit(EXIT_FAILURE);
	}
	
	FlsSetValue(q_idx, malloc(sizeof(calqueue)));
	calqueue_init((calqueue *)FlsGetValue(q_idx));
	event = malloc(sizeof(msg_t));
	event->type = INIT;
	event->receiver = id;
	event->sender = id;
	calqueue_put((calqueue *)FlsGetValue(q_idx), 0.0, event);
	FlsSetValue(state_idx, NULL);

	// Do the job!
	timer_start(fiber_runtime);
	while(true) {

		event = (msg_t *)calqueue_get((calqueue *)FlsGetValue(q_idx));
		if(event == NULL) {
			fprintf(stderr, "No events to process!\n");
			exit(EXIT_FAILURE);
		}
		
		if(event->receiver != id) {
			fprintf(stderr, "Something went wrong! Fiber %d is getting data from fiber %d, which is impossible. Aborting...\n", id, event->receiver);
			exit(EXIT_FAILURE);
		}

		ret = ProcessEvent(event, (lp_state_type *)FlsGetValue(state_idx), (calqueue *)FlsGetValue(q_idx));
	printf("%f\n", 9.38f);
	
		if(event->type == INIT) {
			FlsSetValue(state_idx, ret);
			ret = 0;
		}

		free(event);
		
		if(ret)
			break;
	}
	millis = timer_value_milli(fiber_runtime);

	// Notify other fibers that I'm done and reduce the total time
	__sync_fetch_and_add(&exec_millis, millis);
	__sync_fetch_and_sub(&completed_fibers, 1);
	
	if(id != 1) {
		while(true) {
			schedule(0);
		}
	}
		
	while(completed_fibers > 0);
	
	puts("All fibers are done!");
	printf("Time to initialize fibers: %f\n", (double)init_millis / 1000);
	printf("Time to run do the work (per-fiber): %f\n", (double)exec_millis / 1000 / num_fibers);
	exit(EXIT_SUCCESS);
	
}

// This function lives in an "abandoned" fiber: no-one will ever
// get back here!
static void *thread_initialization(void *args) __attribute__ ((noreturn));
static void *thread_initialization(void *args) {
	unsigned int f;
	(void)args;
	
	ConvertThreadToFiber();
	
	while(!init_complete);
	
	while(true) {
		f = get_random_fiber();
		SwitchToFiber(fibers[f]);		
	}
}


int main(int argc, char **argv) {
	char *eptr;
	unsigned int cpus;
	unsigned int i;
	timer fiber_initialization;
	
	// Initialize pseudorandom generator
	srandom(time(0));
	
	// Get number of online CPUs
	cpus = sysconf(_SC_NPROCESSORS_ONLN);
	if(cpus < 1) {
		fprintf(stderr, "Cannot determine the number of online CPUs\n");
		exit(EXIT_FAILURE);
	}
	
	// Check if the number of fibers has been passed
	if(argc < 2) {
		fprintf(stderr, "Usage: %s <num_fibers>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	// Convert to ULL the number of fibers
	num_fibers = strtoll(argv[1], &eptr, 10);
	if(num_fibers == 0) {
		if(errno == EINVAL) {
			fprintf(stderr, "Invalid number of fibers: %s\n", argv[1]);
			exit(EXIT_FAILURE);
		}
		if(errno == ERANGE) {
			fprintf(stderr, "Warning: number of fibers is out of range. Continuing with %d fibers...\n", num_fibers);
		}
	}
	
	// Do we have enough fibers?
	if(num_fibers <= cpus) {
		fprintf(stderr, "On %d cores you need at least %d fibers for this program to run\n", cpus, cpus + 1);
		exit(EXIT_FAILURE);
	}
	
	// Create required numebr of threads
	if(cpus > 1) {
		create_threads(cpus - 1, thread_initialization, NULL);
	}

	// Create required number of fibers
	completed_fibers = num_fibers;
	fibers = malloc(sizeof(void *) * num_fibers);
	timer_start(fiber_initialization);
	fibers[0] = ConvertThreadToFiber();
	for(i = 1; i < num_fibers; i++) {
		fibers[i] = CreateFiber(STACK_SIZE, main_loop, (void *)(i+1));
	}
	init_millis = timer_value_milli(fiber_initialization);
	
	signal(SIGALRM, schedule);
	alarm(1);
	
	init_complete = true;

	main_loop((void *)1);
	
	return 0;
}
