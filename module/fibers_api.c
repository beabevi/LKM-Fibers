#include "../include/module/fibers_api.h"

static void __print_fiber_struct(struct fiber_struct *f)
{
	pr_debug("[fibers: %s] f->owner= %d\n", __FUNCTION__, f->owner);
}

void to_fiber(void)
{
	f1.owner = current->pid;
	id1 = idr_alloc(&fibers_pool, &f1, 0, -1, GFP_KERNEL);
	pr_debug("[fibers: %s] Created fiber1 with id1 = %lu\n", __FUNCTION__,
		 id1);

}

void create_fiber(size_t stack_size, void (*entry_point) (void *), void *param)
{

}

void switch_fiber(fid_t fid)
{
	/**
	int id = (int) fid;
	struct fiber_struct *f = idr_find(&fibers_pool, id);
	pr_debug("[fibers: %s] Got fid = %llx\n", __FUNCTION__, fid);
	__print_fiber_struct(f);
	**/

	struct pt_regs *regs1 = current_pt_regs();
	if (regs1 == NULL)
		return;
	pr_debug("[fibers: %s] regs1->orig_ax = %lu\n", __FUNCTION__,
		 regs1->orig_ax);
	pr_debug("[fibers: %s] regs1->ip = %lx\n", __FUNCTION__, regs1->ip);
	regs1->ip = fid;

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
