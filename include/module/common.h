#ifndef FIBERS_MODULE_COMMON_H
#define FIBERS_MODULE_COMMON_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/idr.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/sched/task_stack.h>

typedef unsigned long fid_t;

struct fiber_struct {
	int owner;
};

#endif
