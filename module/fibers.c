#include "../include/module/fibers.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR
    ("Beatrice Bevilacqua <beatricebevilacqua1995@gmail.com> and Anxhelo Xhebraj <angelogebrai@gmail.com>");
MODULE_DESCRIPTION("");		// TODO: change module description

/*
 * Global variables are declared as static, so are global within the file.
 */

static struct file_operations fibers_fops = {
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

/*
 * This function is called when the module is loaded
 */
int init_module(void)
{
	// Device registration for userspace-driver communication
	int minor = misc_register(&fibers_dev);
	if (minor < 0) {
		pr_alert("[fibers: %s] Failed to register /dev/%s\n",
			 __FUNCTION__, DEVICE_NAME);
		return minor;
	}
	pr_debug("[fibers: %s] Device created on /dev/%s\n", __FUNCTION__,
		 DEVICE_NAME);

	return SUCCESS;
}

/*
 * This function is called when the module is unloaded
 */
void cleanup_module(void)
{
	pr_debug("[fibers: %s]\n", __FUNCTION__);
	/*
	 * Unregister the device
	 */
	misc_deregister(&fibers_dev);
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
	struct idr *fibers_pool;
	pr_debug("[fibers: %s]\n", __FUNCTION__);

	// Increments reference counter of the module to ensure that it is
	// not removed while some process is using it
	try_module_get(THIS_MODULE);

	fibers_pool = kmalloc(sizeof(struct idr), GFP_USER);
	if (!fibers_pool) {
		return -1;
	}
	// Initialization of fibers pool idr
	idr_init(fibers_pool);

	file->private_data = fibers_pool;

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
	pr_debug("[fibers: %s]\n", __FUNCTION__);

	idr_for_each(file->private_data, fib_free, NULL);

	// Destroy fibers pool idr
	idr_destroy(file->private_data);
	kfree(file->private_data);

	/*
	 * Decrement the usage count, or else once you opened the file, you'll
	 * never get get rid of the module.
	 */
	module_put(THIS_MODULE);

	return SUCCESS;
}

static long device_ioctl(struct file *filp,
			 unsigned int ioctl_num, unsigned long ioctl_param)
{
	switch (ioctl_num) {
	case IOCTL_TO_FIB:
		return to_fiber(filp->private_data);
		break;
	case IOCTL_CREATE_FIB:
		return create_fiber(filp->private_data,
				    (struct create_data *)ioctl_param);
		break;
	case IOCTL_SWITCH_FIB:
		switch_fiber(filp->private_data, ioctl_param);
		break;
	case IOCTL_FLS_ALLOC:
		return fls_alloc();
		break;
	case IOCTL_FLS_FREE:
		return fls_free((long)ioctl_param);
		break;
	case IOCTL_FLS_SET:
		fls_set((struct fls_set_data *)ioctl_param);
		break;
	case IOCTL_FLS_GET:
		return fls_get((long)ioctl_param);
		break;
	}
	return 0;
}
