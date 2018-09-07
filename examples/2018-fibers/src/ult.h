#pragma once

#define STACK_SIZE (4096*2)

#pragma GCC poison setjmp longjmp

#include <stdio.h>
#include <stdbool.h>
#include "jmp.h"

typedef struct {
	volatile bool running;
	exec_context_t *context;
	int id;
} ult_t;

/// Save machine context for userspace context switch. This is used only in initialization.
#define context_save(context) set_jmp(context)

/// Restore machine context for userspace context switch. This is used only in inizialitaion.
#define context_restore(context) long_jmp(context, 1)

/// Swicth machine context for userspace context switch. This is used to schedule a LP or return control to simulation kernel
#define context_switch(context_old, context_new) \
	if(set_jmp(context_old->context) == 0) {\
		ult_unlock(context_old);\
		long_jmp(context_new->context, (context_new)->context->rax);\
	}

/// Swicth machine context for userspace context switch. This is used to schedule a LP or return control to simulation kernel
#define context_switch_create(context_old, context_new) \
	ult_lock(context);\
	if(set_jmp(context_old->context) == 0) {\
		ult_unlock(context_old);\
		long_jmp(context_new->context, 1);\
	}


extern void *ult_convert(void);
extern void *ult_creat(size_t stack_size, void (*entry_point)(void *), void *param);
extern void ult_switch_to(void *ult);
// extern long long fls_get(long idx);
// extern bool fls_free(long idx);
// extern long fls_alloc(void);
// extern void fls_set(long idx, long long value);

