#ifndef FIBERS_MODULE_COMMON_H
#define FIBERS_MODULE_COMMON_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/idr.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/sched/task_stack.h>
#include <linux/types.h>
#include <linux/xarray.h>
#include <linux/proc_fs.h>

#include <klog.h>
#include "../const.h"

struct fiber_struct {
	unsigned long state;	// RUNNING-STOPPED
	unsigned long entry_point;
	pid_t pid;
	unsigned long activations;
	atomic64_t failed_activations;
	u64 laststart_utime;
	u64 laststart_stime;
	u64 utime;
	u64 stime;
	struct pt_regs exec_context;
	struct fpu fpuregs;
};

struct fibers_data {
	struct idr fibers_pool;
	struct proc_dir_entry *base;
};

#endif
