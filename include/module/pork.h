#ifndef FIBERS_MODULE_PORK_H
#define FIBERS_MODULE_PORK_H

#include <linux/fs.h>
#include <linux/kallsyms.h>
#include <linux/namei.h>
#include <linux/sched/task.h>

void hijack_symbols(void);
void restore_symbols(void);

#endif