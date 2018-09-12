#ifndef FIBERS_LIB_H
#define FIBERS_LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>		// size_t
#include <stdbool.h>		// bool
#include <sys/syscall.h>	// SYS_ioctl
#include <fcntl.h>		// open()
#include <sys/mman.h>		// mmap()
#include <unistd.h>		// close()
#include <pthread.h>	// pthread_atfork()
#include "../const.h"

void *to_fiber(void);
void *create_fiber(size_t stack_size, void (*entry_point) (void *),
		   void *param);
void switch_fiber(void *fid);
long fls_alloc(void);
bool fls_free(long index);
void fls_set(long index, long long value);
long long fls_get(long index);

#endif
