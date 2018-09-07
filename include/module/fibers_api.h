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
long switch_fiber(void *fibers_pool, fid_t fid);
long fls_alloc(void);
bool fls_free(long index);
long fls_set(struct fls_data __user * data);
long fls_get(struct fls_data __user * data);

#define MAX_FLS 4096

#define current_fiber (*((struct fiber_struct **) (((unsigned long)current->stack) + sizeof(struct thread_info))))
#endif
