#include "../include/lib/fibers.h"
#include "../util/log.h"

static int __fibers_file = -1;

static void __attribute__ ((constructor)) __file_opener();

/*
* In case of a fork() `struct file`s of open files of the parent
* are shared by the child (with maybe different file descriptor number).
* We must close the fibers file descriptor in the child by registering
* this routine at fork to ensure that two different processes do not share
* the fiber pool contained in `private_data` of the `struct file` associated
* to the /dev/fibers file.
*
* More on file descriptors inheritance:
* https://stackoverflow.com/a/11734354
*/

static void __fork_handler()
{
	int err = close(__fibers_file);
	__fibers_file = open("/dev/" DEVICE_NAME, 0);
	if (__fibers_file < 0) {
		log_fatal("Fiber file is not open, exiting...");
	}
}

static void __file_opener()
{
	__fibers_file = open("/dev/" DEVICE_NAME, 0);
	if (__fibers_file < 0) {
		log_fatal("Fiber file is not open, exiting...");
	}
	int err = pthread_atfork(NULL, NULL, __fork_handler);
	if (err) {
		log_fatal("Couldn't register atfork handler");
	}
}

// https://stackoverflow.com/a/48218706
// The dispatcher will take the fast path for the ioctl system call
// not saving the registers below for performance reasons (callee-save).
//
// In case of switching from one fiber to another we might loose the
// state of those registers.
// Consider two fibers f1, f2 with the following interleaving:
//
//         f1                     f2
// %rbx := 3
// switch_fiber(f2)  -----> %rbx := 2
//                   <----- switch_fiber(f1)
// %rbx = ?

long fib_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg)
{
	long res = 0;
	if (cmd == IOCTL_SWITCH_FIB) {
		asm volatile ("push %%rbx \n\t"
			      "push %%rbp \n\t"
			      "push %%r12 \n\t"
			      "push %%r13 \n\t"
			      "push %%r14 \n\t"
			      "push %%r15 \n\t"
			      "syscall \n\t"
			      "pop %%r15 \n\t"
			      "pop %%r14 \n\t"
			      "pop %%r13 \n\t"
			      "pop %%r12 \n\t"
			      "pop %%rbp \n\t" "pop %%rbx \n\t":"=a" (res)
			      :"0"(SYS_ioctl), "D"(fd), "S"(cmd), "d"(arg):
			      "memory");
	} else {
		asm volatile ("syscall \n\t":"=a" (res)
			      :"0"(SYS_ioctl), "D"(fd), "S"(cmd), "d"(arg):
			      "memory");

	}

	return res;
}

void *to_fiber(void)
{
	long ret = fib_ioctl(__fibers_file, IOCTL_TO_FIB, 0);
	if (ret < 0) {
		log_fatal("Couldn't convert thread to fiber");
	}
	return (void *)ret;
}

void *create_fiber(size_t stack_size, void (*entry_point) (void *), void *param)
{
	void *stack = mmap(NULL, stack_size, PROT_WRITE | PROT_READ,
			   MAP_PRIVATE | MAP_ANON, -1, 0);

	if (stack == NULL) {
		log_fatal("Couldn't allocate stack");
	}

	struct create_data data = {
		// x86-64 System-V ABI requires stack to be aligned at 16 byte before
		// issuing a `call` and compilers assume this when compiling the
		// entry points of fibers. Therefore in order to emulate a call we
		// need to remove 8 bytes as if there was the return address to the
		// caller.
		.stack = (void *)(((unsigned long)stack) + stack_size - 8),
		.entry_point = entry_point,
		.param = param
	};

	long ret =
	    fib_ioctl(__fibers_file, IOCTL_CREATE_FIB, (unsigned long)&data);
	if (ret < 0) {
		log_fatal("Couldn't create fiber");
	}
	return (void *)ret;
}

void switch_fiber(void *fid)
{
	long ret =
	    fib_ioctl(__fibers_file, IOCTL_SWITCH_FIB, (unsigned long)fid);
	if (ret < 0) {
		// log_warn("Switch failed");
	}
}

long fls_alloc(void)
{
	long ret = fib_ioctl(__fibers_file, IOCTL_FLS_ALLOC, 0);
	if (ret < 0) {
		log_warn("Allocation failed");
	}
	return ret;
}

bool fls_free(long index)
{
	long ret = fib_ioctl(__fibers_file, IOCTL_FLS_FREE, index);
	if (ret < 0) {
		log_warn("Free failed");
		return false;
	}
	return true;
}

void fls_set(long index, long long value)
{
	struct fls_data data = {
		.index = index,
		.value = value
	};

	long ret =
	    fib_ioctl(__fibers_file, IOCTL_FLS_SET, (unsigned long)&data);
	if (ret < 0) {
		log_warn("Setting failed");
	}
}

long long fls_get(long index)
{
	struct fls_data data = {
		.index = index
	};

	long ret =
	    fib_ioctl(__fibers_file, IOCTL_FLS_GET, (unsigned long)&data);
	if (ret < 0) {
		log_warn("Get failed");
		return 0;
	}
	return data.value;
}
