#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <errno.h>
#include <sys/mman.h>
#include "thread.h"
#include "ult.h"

#define MAX_FLS		4096

extern volatile unsigned int completed_fibers;

static __thread volatile sig_atomic_t	context_called;

static __thread void			(*context_creat_func)(void *);
static __thread void			*context_creat_arg;

static long long	fls[MAX_FLS];
static long		fls_idx;
static __thread ult_t *current;
static __thread ult_t *context_creat;
static int		ult_id = 0;

#define cpu_relax() asm volatile("pause\n": : :"memory")

// Trylock: used to check whether we can schedule a fiber
static bool ult_trylock(ult_t *ctx) {
	volatile bool *pFlag = &ctx->running;
	return !__atomic_test_and_set(pFlag, __ATOMIC_ACQUIRE);
}

// Lock: we are sure that the fiber is free, so we lock it
static void ult_lock(ult_t *ctx) {  
	volatile bool *pFlag = &ctx->running;
	for (;;) {
		if (!__atomic_test_and_set(pFlag, __ATOMIC_ACQUIRE)) {
			return;
		}
        
		while (__atomic_load_n(pFlag, __ATOMIC_RELAXED)) {
			cpu_relax();
		}
	}
}

// Releasing a fiber
void ult_unlock(ult_t *ctx) {
	volatile bool *pFlag = &ctx->running;
	__atomic_clear(pFlag, __ATOMIC_RELEASE);
}

// Allocate a new stack for the fiber to be created
static void *get_ult_stack(size_t size) {
	void *stack;
	size_t reminder;

	// Sanity check
	if (size <= 0) {
		size = SIGSTKSZ;
	}

	// Align the size to the page boundary
	reminder = size % getpagesize();
	if (reminder != 0) {
		size += getpagesize() - reminder;
	}

	posix_memalign(&stack, 16, STACK_SIZE);
	bzero(stack, size);

	return stack;
}

// This is the function which, upon reschedule, will activate the
// function associated with the fiber
static void context_create_boot(void) __attribute__ ((noreturn));
static void context_create_boot(void) {

	void (*context_start_func)(void *);
	void *context_start_arg;

	context_start_func = context_creat_func;
	context_start_arg = context_creat_arg;
	
	ult_lock(current); // Locking is demanded to outer code
	context_switch(context_creat, current);
	
	context_start_func(context_start_arg);

	// you should never reach this!
	abort();
}

// Trampoline to create fibers. This is executed in a signal handler
// in order to have a new stack for the fiber, then the handler returns,
// but we have saved the context in the new stack. In this way we have
// a fresh stack out of the signal context.
static void context_create_trampoline(int sig) {
	(void)sig;

	if(context_save(context_creat->context) == 0)
		return;

	context_create_boot();
}

// Create a new fiber. Some magic with signals and sigaltstack() is done here
void context_create(ult_t *context, void (*entry_point)(void *), void *args, void *stack, size_t stack_size) {
	struct sigaction sa;
	stack_t ss;
	stack_t oss;

	bzero((void *)&sa, sizeof(struct sigaction));
	sa.sa_handler = context_create_trampoline;
	sa.sa_flags = SA_ONSTACK;
	sigfillset(&sa.sa_mask);
	sigdelset(&sa.sa_mask, SIGUSR1);
	sigaction(SIGUSR1, &sa, NULL);

	ss.ss_sp = stack;
	ss.ss_size = stack_size;
	ss.ss_flags = 0;
	if(sigaltstack(&ss, &oss) == -1) {
		fprintf(stderr, "Critical error: cannot initialize a separate stack\n");
	}

	context_creat = context;
	context_creat_func = entry_point;
	context_creat_arg = args;
	context_called = false;
	raise(SIGUSR1);
	if(sigaltstack(&oss, NULL) == -1)  {
		fprintf(stderr, "Critical error: cannot initialize a separate stack\n");
	}

	context_switch_create(current, context);
}	

// Simplistic allocation for FLS
/* long fls_alloc(void) { */
	/* long ret = __sync_fetch_and_add (&fls_idx, 1); */
	/* if(ret >= MAX_FLS) */
		/* return -1; */
	/* return ret; */
/* } */

/* // Get a FLS value */
/* long long fls_get(long idx) { */
	/* return fls[idx]; */
/* } */

/* // Dummy: we don't actually free FLS here... */
/* bool fls_free(long idx) { */
	/* (void)idx; */
	/* return true; */
/* } */

/* // Store a value in FLS storage */
/* void fls_set(long idx, long long value) { */
	/* fls[idx] = value; */
/* } */

// A thread is migrated to a fiber (i.e., we create a running context for it)
// This must be called before creating any other fiber, or current will
// be null and the application will fail badly. There's no sanity check on this.
void *ult_convert(void) {
	ult_t *ctx;

	ctx = malloc(sizeof(ult_t));
	bzero(ctx, sizeof(ult_t));
	ctx->id = __sync_fetch_and_add(&ult_id, 1);
	ult_lock(ctx);
	ctx->context = malloc(sizeof(exec_context_t));
	bzero(ctx->context, sizeof(exec_context_t));
	context_save(ctx->context);
	
	current = ctx;
	return ctx;
}

// Create a new fiber. To be called after an invocation of ult_convert()
// on each new thread.
void *ult_creat(size_t stack_size, void (*entry_point)(void *), void *args) {
	ult_t *ctx;
	void *stack = NULL;

	stack = get_ult_stack(stack_size);
	
	ctx = malloc(sizeof(ult_t));
	bzero(ctx, sizeof(ult_t));

	ctx->id = __sync_fetch_and_add(&ult_id, 1);
	printf("Created fiber with id %d\n", ctx->id);
	
	ctx->context = malloc(sizeof(exec_context_t));
	bzero(ctx->context, sizeof(exec_context_t));
	
	context_create(ctx, entry_point, args, stack, stack_size);
	return ctx;
}

// Change execution context in a thread.
void ult_switch_to(void *ult) {
	ult_t *from = current;
	ult_t *to = (ult_t *)ult;
	
	if(!to || !ult_trylock(to))
		return;
	
	if(completed_fibers > 0)
		printf("Thread %d switching to fiber %d...", tid, to->id);
	current = to;
	if(completed_fibers > 0)
		puts("done.");
	
	context_switch(from, to);
}
