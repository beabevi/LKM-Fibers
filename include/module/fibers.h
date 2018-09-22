#ifndef FIBERS_MODULE_H
#define FIBERS_MODULE_H

#include <linux/miscdevice.h>
#include <linux/stat.h>

#include "common.h"
#include "fibers_api.h"
#include "pork.h"

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static long device_ioctl(struct file *filp,
			 unsigned int ioctl_num, unsigned long ioctl_param);

#define SUCCESS 0

#endif
