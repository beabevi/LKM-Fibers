#include <fibers_api.h>

static long long _fls[MAX_FLS];
static long fls_idx = 0;

char *fib_format = "state: %lu\n"\
    "entry point: %lx\n"\
    "creator pid: %d\n"\
    "# activations: %lu\n"\
	"# failed activations: %ld\n"\
	"time:\n";

ssize_t fiber_read_info(struct file *filp, char *buffer, size_t buff_len,
			loff_t * off)
{
	char out[256];
	int n_bytes, ret;
	struct inode *inode = filp->f_inode;
	struct fiber_struct *f = PDE_DATA(inode);

	if (*off > 0) {
		return 0;
	}

	snprintf(out, 256, fib_format, f->state, f->entry_point, f->pid,
		 f->activations, f->failed_activations.counter);
	n_bytes = strlen(out);
	ret = copy_to_user(buffer, out, n_bytes);
	if (ret) {
		pr_warn("[fibers: %s] Could not copy fiber's data\n",
			__FUNCTION__);
		return ret;
	}

	*off += n_bytes;
	return n_bytes;
}

static struct file_operations fops = {
	.read = fiber_read_info
};

static inline void fiber_setup_stats(struct fiber_struct *f,
				     unsigned long entry)
{
	f->entry_point = entry;
	f->pid = current->pid;
	f->activations = (f->state == FIB_RUNNING) ? 1 : 0;
	f->failed_activations.counter = 0;
	// TODO: initialize execution time
}

static inline void fiber_init_stopped(struct fiber_struct *f,
				      struct create_data *data)
{
	f->state = FIB_STOPPED;
	f->exec_context = *current_pt_regs();
	f->exec_context.sp = (unsigned long)data->stack;
	f->exec_context.ip = (unsigned long)data->entry_point;
	f->exec_context.di = (unsigned long)data->param;
	f->exec_context.flags = 0L;
	fiber_setup_stats(f, f->exec_context.ip);
}

static fid_t fibers_pool_add(struct fibers_data *fibdata,
			     struct fiber_struct *f)
{
#define buf_len 32
	unsigned long flags;
	fid_t id;
	int ret;
	char buf[buf_len];
	struct proc_dir_entry *proc_file;

	xa_lock_irqsave(&fibdata->fibers_pool.idr_rt, flags);
	id = idr_alloc(&fibdata->fibers_pool, f, 0, -1, GFP_KERNEL);
	xa_unlock_irqrestore(&fibdata->fibers_pool.idr_rt, flags);

	ret = snprintf(buf, buf_len, "%lu", id);
	if (!ret) {
		pr_warn("[fibers: %s] Could not read fid\n", __FUNCTION__);
		return -1;
	}
	proc_file = proc_create_data(buf, S_IALLUGO, fibdata->base, &fops, f);
	if (!proc_file) {
		pr_warn("[fibers: %s] Could not create proc file\n",
			__FUNCTION__);
		return -1;
	}

	return id;
}

long to_fiber(struct fibers_data *fibdata)
{
	fid_t id;
	struct fiber_struct *f =
	    kmalloc(sizeof(struct fiber_struct), GFP_KERNEL);

	if (unlikely(!f)) {
		pr_warn("[fibers: %s] Failed create struct fiber (%d)\n",
			__FUNCTION__, current->pid);
		return -1;
	}

	f->state = FIB_RUNNING;
	fiber_setup_stats(f, current_pt_regs()->ip);

	id = fibers_pool_add(fibdata, f);
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

long create_fiber(struct fibers_data *fibdata,
		  struct create_data __user * udata)
{
	unsigned long ret;
	struct create_data data;
	fid_t id;
	struct fiber_struct *f =
	    kzalloc(sizeof(struct fiber_struct), GFP_KERNEL);

	if (unlikely(!f)) {
		pr_warn("[fibers: %s] Failed create struct fiber (%d)\n",
			__FUNCTION__, current->pid);
		return -1;
	}

	ret = copy_from_user(&data, udata, sizeof(struct create_data));

	if (unlikely(ret)) {
		pr_warn("[fibers: %s] Could not copy create_data\n",
			__FUNCTION__);
		kfree(f);
		return -1;
	}

	fiber_init_stopped(f, &data);

	id = fibers_pool_add(fibdata, f);
	if (unlikely(id < 0)) {
		kfree(f);
		pr_warn("[fibers: %s] Failed to convert thread (%d) to fiber\n",
			__FUNCTION__, current->pid);
		return -1;
	}
	return id;
}

long switch_fiber(struct fibers_data *fibdata, fid_t fid)
{
	struct fiber_struct *next, *prev;
	bool old;
	struct pt_regs *regs;

	prev = current_fiber;

	if (unlikely(!prev)) {
		pr_warn
		    ("[fibers: %s] Attempt to switch not from fiber context\n",
		     __FUNCTION__);
		return -1;
	}

	rcu_read_lock();
	next = idr_find(&fibdata->fibers_pool, fid);
	rcu_read_unlock();

	if (unlikely(next == NULL)) {
		pr_warn("[fibers: %s] Failed to switch to %lu\n", __FUNCTION__,
			fid);
		return -1;
	}

	old = test_and_set_bit(0, &(next->state));
	if (unlikely(old == FIB_RUNNING)) {
		atomic64_inc(&next->failed_activations);
		return -1;
	}

	next->activations++;
	// TODO: update exec time

	regs = current_pt_regs();
	prev->exec_context = *regs;
	*regs = next->exec_context;

	fpu__save(&prev->fpuregs);

	preempt_disable();
	fpu__restore(&next->fpuregs);
	preempt_enable();

	test_and_clear_bit(0, &(prev->state));

	current_fiber = next;

	return 0;
}

long fls_alloc(void)
{
	long ret = __sync_fetch_and_add(&fls_idx, 1);
	if (ret >= MAX_FLS)
		return -1;
	return ret;
}

// Get a FLS value
long fls_get(struct fls_data __user * data)
{
	struct fls_data fsd;
	unsigned long ret =
	    copy_from_user(&fsd.index, &data->index, sizeof(long));
	if (ret) {
		pr_warn("[fibers: %s] Could not copy fls_data\n", __FUNCTION__);
		return -1;
	}
	fsd.value = _fls[fsd.index];

	ret = copy_to_user(&data->value, &fsd.value, sizeof(long long));

	if (ret) {
		pr_warn("[fibers: %s] Could not copy fls_data\n", __FUNCTION__);
		return -1;
	}
	return 0;
}

// Dummy: we don't actually free FLS here...
bool fls_free(long idx)
{
	(void)idx;
	return true;
}

// Store a value in FLS storage
long fls_set(struct fls_data __user * data)
{
	struct fls_data fsd;
	unsigned long ret = copy_from_user(&fsd, data, sizeof(struct fls_data));
	if (ret) {
		pr_warn("[fibers: %s] Could not copy fls_data\n", __FUNCTION__);
		return -1;
	}
	_fls[fsd.index] = fsd.value;

	return 0;
}
