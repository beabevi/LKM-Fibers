#include <linux/kernel.h>
#include <linux/module.h>

#include "../include/module/fibers_api.h"

void to_fiber(void)
{
	pr_debug("[fibers: %s]\n", __FUNCTION__);
}

void create_fiber(size_t stack_size, void (*entry_point) (void *), void *param)
{

}

void switch_fiber(void *fid)
{

}

long fls_alloc(void)
{
	return 0;
}

bool fls_free(long index)
{
	return 0;
}

void fls_set(long index, long long value)
{

}

long long fls_get(long index)
{
	return 0;
}
