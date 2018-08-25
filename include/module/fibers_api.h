#ifndef FIBERS_MODULE_API_H
#define FIBERS_MODULE_API_H

void to_fiber(void);
void create_fiber(size_t stack_size, void (*entry_point)(void*), void* param);
void switch_fiber(void* fid);
long fls_alloc(void);
bool fls_free(long index);
void fls_set(long index, long long value);
long long fls_get(long index);

#endif