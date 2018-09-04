#include "../include/module/fibers_api.h"

void fiber_init(struct fiber_struct *f, int fib_flags, struct create_data *data)
{
	f->exec_context = *current_pt_regs();
	f->flags = fib_flags;
	if (fib_flags == FIB_STOPPED) {
		// create_fiber called fiber_init
		f->exec_context.sp = (unsigned long)data->stack;
		f->exec_context.ip = (unsigned long)data->entry_point;
		f->exec_context.di = (unsigned long)data->param;
	}
	f->fpuregs = current->thread.fpu;
}

fid_t fibers_pool_add(struct idr *pool, struct fiber_struct *f)
{
	//unsigned long flags;
	fid_t id;

	// xa_lock_irqsave(&fibers_pool.idr_rt, flags);
	id = idr_alloc(pool, f, 0, -1, GFP_KERNEL);
	// xa_unlock_irqrestore(&fibers_pool.idr_rt, flags);

	return id;
}

long to_fiber(void *fibers_pool)
{
	fid_t id;
	struct fiber_struct *f = kmalloc(sizeof(struct fiber_struct), GFP_USER);
	if (unlikely(!f)) {
		pr_warn("[fibers: %s] Failed create struct fiber (%d)\n",
			__FUNCTION__, current->pid);
		return -1;
	}

	fiber_init(f, FIB_RUNNING, NULL);

	id = fibers_pool_add(fibers_pool, f);
	if (unlikely(id < 0)) {
		kfree(f);
		pr_warn("[fibers: %s] Failed to convert thread (%d) to fiber\n",
			__FUNCTION__, current->pid);
		return -1;
	}
	// Save current running fiber at the bottom of the kernel stack
	// of the thread it is running on
	current_fiber = f;

	return id;
}

long create_fiber(void *fibers_pool, struct create_data __user * udata)
{
	unsigned long ret;
	struct create_data data;
	fid_t id;
	struct fiber_struct *f = kmalloc(sizeof(struct fiber_struct), GFP_USER);

	if (unlikely(!f)) {
		pr_warn("[fibers: %s] Failed create struct fiber (%d)\n",
			__FUNCTION__, current->pid);
		return -1;
	}

	ret = copy_from_user(&data, udata, sizeof(struct create_data));

	if (ret) {
		pr_warn("[fibers: %s] Could not copy create_data\n",
			__FUNCTION__);
		return -1;
	}

	fiber_init(f, FIB_STOPPED, &data);

	id = fibers_pool_add(fibers_pool, f);
	if (unlikely(id < 0)) {
		kfree(f);
		pr_warn("[fibers: %s] Failed to convert thread (%d) to fiber\n",
			__FUNCTION__, current->pid);
		return -1;
	}
	return id;
}

void switch_fiber(void *fibers_pool, fid_t fid)
{
	struct fiber_struct *next;
	bool old;
	struct pt_regs *regs;

	rcu_read_lock();
	next = idr_find(fibers_pool, fid);
	rcu_read_unlock();

	if (next == NULL) {
		pr_warn("[fibers: %s] Failed to switch to %lu\n", __FUNCTION__,
			fid);
	}

	old = test_and_set_bit(0, &(next->flags));
	if (unlikely(old == FIB_RUNNING)) {
		pr_warn("[fibers: %s] Switch attempted to running fiber %lu\n",
			__FUNCTION__, fid);
		return;
	}

	regs = current_pt_regs();
	current_fiber->exec_context = *regs;
	*regs = next->exec_context;

	kernel_fpu_begin();
	current_fiber->fpuregs = current->thread.fpu;
	current->thread.fpu = next->fpuregs;
	kernel_fpu_end();
	test_and_clear_bit(0, &(current_fiber->flags));

	current_fiber = next;
}

long fls_alloc(void)
{
	return 0;
}

bool fls_free(long index)
{
	return 0;
}

void fls_set(struct fls_set_data __user * data)
{

}

long long fls_get(long index)
{
	return 0;
}
