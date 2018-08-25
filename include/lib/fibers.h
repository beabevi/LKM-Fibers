#ifndef FIBERS_LIB_H
#define FIBERS_LIB_H

#include <stdio.h>
#include <stddef.h> // size_t
#include <stdbool.h> // bool
#include <sys/ioctl.h> // ioctl
#include <fcntl.h> // open()
#include "../const.h"

extern void to_fiber(void);
extern void create_fiber(size_t stack_size, void (*entry_point)(void*), void* param);
extern void switch_fiber(void* fid);
extern long fls_alloc(void);
extern bool fls_free(long index);
extern void fls_set(long index, long long value);
extern long long fls_get(long index);

#endif