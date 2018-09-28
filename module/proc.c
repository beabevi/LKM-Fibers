#include <proc.h>

typedef struct dentry *(*proc_pident_instantiate_t) (struct dentry *,
						     struct task_struct *,
						     const void *);
typedef bool(*proc_fill_cache_t) (struct file *, struct dir_context *,
				  const char *, unsigned int,
				  proc_pident_instantiate_t,
				  struct task_struct *, const void *);

typedef int (*proc_tgid_base_readdir_t) (struct file * file,
					 struct dir_context * ctx);
typedef struct dentry *(*proc_tgid_base_lookup_t) (struct inode * dir,
						   struct dentry * dentry,
						   unsigned int flags);

union proc_op {
	int (*proc_get_link) (struct dentry *, struct path *);
	int (*proc_show) (struct seq_file * m, struct pid_namespace * ns,
			  struct pid * pid, struct task_struct * task);
};

struct pid_entry {
	const char *name;
	unsigned int len;
	umode_t mode;
	const struct inode_operations *iop;
	const struct file_operations *fop;
	union proc_op op;
};

static proc_fill_cache_t proc_fill_cache;
static proc_pident_instantiate_t proc_pident_instantiate;

static proc_tgid_base_lookup_t orig_proc_tgid_base_lookup;
static proc_tgid_base_readdir_t orig_proc_tgid_base_readdir;

static struct file_operations *proc_tgid_base_operations;
static struct inode_operations *proc_tgid_base_inode_operations;

static struct pid_entry fibers_link;

enum hooked_syms {
	PROC_PID_LINK_INODE_OPERATIONS,
	PROC_FILL_CACHE,
	PROC_PIDENT_INSTANTIATE,
	PROC_TGID_BASE_OPERATIONS,
	PROC_TGID_BASE_INODE_OPERATIONS,
	HOOKED_SYMS_MAX
};

static const char *hooked_syms_names[] = {
	"proc_pid_link_inode_operations",
	"proc_fill_cache",
	"proc_pident_instantiate",
	"proc_tgid_base_operations",
	"proc_tgid_base_inode_operations"
};

static void *syms[HOOKED_SYMS_MAX];

static unsigned long cr0;

static void LNK(struct pid_entry *pe, const char *name, int name_len,
		void *get_link)
{
	pe->name = name;
	pe->len = name_len;
	pe->mode = (S_IFLNK | S_IRWXUGO);
	pe->iop = syms[PROC_PID_LINK_INODE_OPERATIONS];
	pe->fop = NULL;
	pe->op.proc_get_link = get_link;
}

static int proc_fibers_link(struct dentry *dentry, struct path *path)
{
	char buf[64];
	int ret;
	struct dentry *parent = dentry->d_parent;
	snprintf(buf, 64, "/proc/.fibers/%s", parent->d_iname);
	ret = kern_path(buf, LOOKUP_FOLLOW, path);
	path_get(path);
	if (ret) {
		warn("Couldn't find the directory %s\n", buf);
		return 0;
	}

	return 0;
}

static int wrap_proc_tgid_base_readdir(struct file *file,
				       struct dir_context *ctx)
{
	int orig_ret;
	int ret;
	char buf[64];
	struct task_struct *task;
	pid_t pid;
	struct path path;

	// readdir is called infinitely often until ctx->pos is unchanged
	// Here we ensure that we don't instantiate fibers again
	if (ctx->pos > 0)
		return 0;

	if ((orig_ret = orig_proc_tgid_base_readdir(file, ctx))) {
		return orig_ret;
	}

	snprintf(buf, 64, "/proc/.fibers/%s", file->f_path.dentry->d_iname);
	ret = kern_path(buf, LOOKUP_FOLLOW, &path);
	if (ret) {
		return orig_ret;
	}

	ret = kstrtouint(file->f_path.dentry->d_iname, 10, &pid);
	if (ret) {
		warn("Couldn't convert string to pid\n");
		return orig_ret;
	}

	task = get_pid_task(find_vpid(pid), PIDTYPE_PID);

	if (!task) {
		warn("No task found\n");
		return orig_ret;
	}

	if (proc_fill_cache(file, ctx, fibers_link.name, fibers_link.len,
			    proc_pident_instantiate, task, &fibers_link)) {
		ctx->pos++;
	}
	put_task_struct(task);
	return 0;
}

static struct dentry *wrap_proc_tgid_base_lookup(struct inode *dir,
						 struct dentry *dentry,
						 unsigned int flags)
{
	int ret;
	char buf[64];
	struct path path;
	struct task_struct *task;
	pid_t pid;
	struct dentry *res;

	if (memcmp(dentry->d_name.name, "fibers", 6)) {
		return orig_proc_tgid_base_lookup(dir, dentry, flags);
	}

	snprintf(buf, 64, "/proc/.fibers/%s", dentry->d_parent->d_name.name);
	ret = kern_path(buf, LOOKUP_FOLLOW, &path);
	if (ret) {
		return ERR_PTR(-ENOENT);
	}

	ret = kstrtouint(dentry->d_parent->d_name.name, 10, &pid);
	if (ret) {
		warn("Couldn't convert string to pid\n");
		return ERR_PTR(-ENOENT);
	}
	task = get_pid_task(find_vpid(pid), PIDTYPE_PID);

	if (!task) {
		warn("No task found\n");
		return ERR_PTR(-ENOENT);
	}

	res = proc_pident_instantiate(dentry, task, &fibers_link);
	put_task_struct(task);
	return res;
}

static inline void protect_memory(void)
{
	write_cr0(cr0);
}

static inline void unprotect_memory(void)
{
	write_cr0(cr0 & ~0x00010000);
}

void hook_symbols(void)
{
	int i = 0;

	for (i = 0; i < HOOKED_SYMS_MAX; i++) {
		syms[i] = (void *)kallsyms_lookup_name(hooked_syms_names[i]);
		if (!syms[i]) {
			warn("Failed retrieving symbol %s",
			     hooked_syms_names[i]);
		}
	}

	LNK(&fibers_link, "fibers", 6, proc_fibers_link);

	proc_fill_cache = syms[PROC_FILL_CACHE];

	proc_pident_instantiate = syms[PROC_PIDENT_INSTANTIATE];

	proc_tgid_base_operations = syms[PROC_TGID_BASE_OPERATIONS];

	proc_tgid_base_inode_operations = syms[PROC_TGID_BASE_INODE_OPERATIONS];

	cr0 = read_cr0();
	unprotect_memory();
	orig_proc_tgid_base_readdir = proc_tgid_base_operations->iterate_shared;
	proc_tgid_base_operations->iterate_shared = wrap_proc_tgid_base_readdir;

	orig_proc_tgid_base_lookup = proc_tgid_base_inode_operations->lookup;
	proc_tgid_base_inode_operations->lookup = wrap_proc_tgid_base_lookup;
	protect_memory();
}

void restore_symbols(void)
{
	unprotect_memory();
	proc_tgid_base_operations->iterate_shared = orig_proc_tgid_base_readdir;
	proc_tgid_base_inode_operations->lookup = orig_proc_tgid_base_lookup;
	protect_memory();
}
