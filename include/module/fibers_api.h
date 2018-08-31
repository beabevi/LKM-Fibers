#ifndef FIBERS_MODULE_API_H
#define FIBERS_MODULE_API_H

#include <linux/ptrace.h>
#include <linux/irq.h>
#include "common.h"

extern struct fiber_struct f1, f2;
extern struct idr fibers_pool;
extern fid_t id1, id2;

void to_fiber(void);
void create_fiber(size_t stack_size, void (*entry_point) (void *), void *param);
void switch_fiber(fid_t fid);
long fls_alloc(void);
bool fls_free(long index);
void fls_set(long index, long long value);
long long fls_get(long index);

#endif
