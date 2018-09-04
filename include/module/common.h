#ifndef FIBERS_MODULE_COMMON_H
#define FIBERS_MODULE_COMMON_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/idr.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/sched/task_stack.h>
#include <linux/types.h>
#include <linux/spinlock.h>	// TODO: Move to linux/xarray.h

#include "../const.h"

struct fiber_struct {
	unsigned long flags;	// RUNNING-STOPPED
	struct pt_regs exec_context;
	struct fpu fpuregs;
};

#endif
