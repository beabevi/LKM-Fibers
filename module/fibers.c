#include <fibers.h>

#include <linux/string.h>
#include <linux/fs.h>

/*
 * Global variables are declared as static, so are global within the file.
 */

static struct file_operations fibers_fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_release,
	.unlocked_ioctl = device_ioctl
};

static struct miscdevice fibers_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &fibers_fops,
	.mode = S_IALLUGO
};

struct proc_dir_entry *fibers_dir;

void initialize_proc(void)
{
	fibers_dir = proc_mkdir(".fibers", NULL);
	if (fibers_dir == NULL) {
		warn("Failed proc\n");
		proc_remove(fibers_dir);
	}
	hook_symbols();
}

/*
 * This function is called when the module is loaded
 */
int init_module(void)
{
	// Device registration for userspace-driver communication
	int minor = misc_register(&fibers_dev);
	if (minor < 0) {
		alert("Failed to register /dev/%s\n", DEVICE_NAME);
		return minor;
	}

	initialize_proc();

	return SUCCESS;
}

/*
 * This function is called when the module is unloaded
 */
void cleanup_module(void)
{
	/*
	 * Unregister the device
	 */
	proc_remove(fibers_dir);
	misc_deregister(&fibers_dev);
	restore_symbols();
}

/*
 * Methods
 */

/*
 * Called when a process tries to open the device file, like
 * "cat /dev/fibers"
 */
static int device_open(struct inode *inode, struct file *file)
{
#define buf_len 16
	struct fibers_data *fibdata;
	char buf[buf_len];
	int ret;

	fibdata = kmalloc(sizeof(struct fibers_data), GFP_KERNEL);
	if (!fibdata) {
		return -1;
	}

	ret = snprintf(buf, buf_len, "%d", current->tgid);
	if (!ret) {
		warn("Could not read tgid\n");
		kfree(fibdata);
		return -1;
	}
	fibdata->base = proc_mkdir(buf, fibers_dir);
	if (!fibdata->base) {
		warn("Could not create proc file\n");
		kfree(fibdata);
		return -1;
	}
	// Initialization of fibers pool idr
	idr_init(&fibdata->fibers_pool);

	bitmap_clear(fibdata->bitmap, 0, FLS_BSIZE);

	file->private_data = fibdata;

	return SUCCESS;
}

static int fib_free(int id, void *f, void *data)
{
	kfree(f);
	return 0;
}

/*
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{

	idr_for_each(&((struct fibers_data *)file->private_data)->fibers_pool,
		     fib_free, NULL);

	// Destroy fibers pool idr
	idr_destroy(&((struct fibers_data *)file->private_data)->fibers_pool);
	proc_remove(((struct fibers_data *)file->private_data)->base);

	kfree(file->private_data);

	return SUCCESS;
}

static long device_ioctl(struct file *filp,
			 unsigned int ioctl_num, unsigned long ioctl_param)
{
	switch (ioctl_num) {
	case IOCTL_TO_FIB:
		return to_fiber((struct fibers_data *)filp->private_data);
	case IOCTL_CREATE_FIB:
		return create_fiber((struct fibers_data *)filp->private_data,
				    (struct create_data *)ioctl_param);
	case IOCTL_SWITCH_FIB:
		return switch_fiber((struct fibers_data *)filp->private_data,
				    ioctl_param);
	case IOCTL_FLS_ALLOC:
		return fls_alloc((struct fibers_data *)filp->private_data);
	case IOCTL_FLS_FREE:
		return fls_free((struct fibers_data *)filp->private_data,
				(long)ioctl_param);
	case IOCTL_FLS_SET:
		return fls_set((struct fibers_data *)filp->private_data,
			       (struct fls_data *)ioctl_param);
	case IOCTL_FLS_GET:
		return fls_get((struct fibers_data *)filp->private_data,
			       (struct fls_data *)ioctl_param);
	}
	return -1;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR
    ("Beatrice Bevilacqua <beatricebevilacqua1995@gmail.com> and Anxhelo Xhebraj <angelogebrai@gmail.com>");
MODULE_DESCRIPTION("");		// TODO: change module description
