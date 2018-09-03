#ifndef FIBERS_MODULE_API_H
#define FIBERS_MODULE_API_H

#include <linux/ptrace.h>
#include <linux/irq.h>
#include <linux/rcupdate.h>
#include <linux/uaccess.h>
#include <asm/fpu/internal.h>

#include "common.h"

long to_fiber(void *fibers_pool);
long create_fiber(void *fibers_pool, struct create_data __user * data);
void switch_fiber(void *fibers_pool, fid_t fid);
long fls_alloc(void);
bool fls_free(long index);
void fls_set(struct fls_set_data __user * data);
long long fls_get(long index);

#define current_fiber (*((struct fiber_struct **) current->stack + sizeof(struct thread_info)))
#endif
