#include "../include/lib/fibers.h"

#define assert_fibers_open() \
        do{              \
            if (__fibers_file < 0){ \
                printf("fibers file is not open\n"); \
            } \
        }while(0);

static int __fibers_file = -1;

// https://stackoverflow.com/questions/42609267/ptregs-in-syscall-table
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
			      :"0"(SYS_ioctl), "D"(fd), "S"(cmd), "d"(arg)
		    );
	} else {
		asm volatile ("syscall \n\t":"=a" (res)
			      :"0"(SYS_ioctl), "D"(fd), "S"(cmd), "d"(arg)
		    );

	}

	return res;
}

void *to_fiber(void)
{
	if (__fibers_file < 0)
		__fibers_file = open("/dev/" DEVICE_NAME, O_NONBLOCK);
	assert_fibers_open();
	long ret = fib_ioctl(__fibers_file, IOCTL_TO_FIB, 0);
	return (void *)ret;
}

void *create_fiber(size_t stack_size, void (*entry_point) (void *), void *param)
{
	if (__fibers_file < 0)
		__fibers_file = open("/dev/" DEVICE_NAME, O_NONBLOCK);
	assert_fibers_open();
	//void * stack = mmap(NULL, stack_size, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_GROWSDOWN | MAP_ANONYMOUS, -1, 0);
	void *stack = NULL;
	posix_memalign(&stack, 16, stack_size);
	if (stack == NULL) {
		printf("Error in mmap\n");
		return (void *)-1;
	}

	struct create_data data = {
		.stack = (void *)(((unsigned long)stack) + stack_size),
		.entry_point = entry_point,
		.param = param
	};

	long ret =
	    fib_ioctl(__fibers_file, IOCTL_CREATE_FIB, (unsigned long)&data);
	return (void *)ret;
}

void switch_fiber(void *fid)
{
	assert_fibers_open();
	int ret =
	    fib_ioctl(__fibers_file, IOCTL_SWITCH_FIB, (unsigned long)fid);
	if (ret < 0) {
		printf("Error in ioctl\n");
	}
}

long fls_alloc(void)
{
	assert_fibers_open();
	int ret = fib_ioctl(__fibers_file, IOCTL_FLS_ALLOC, 0);
	if (ret < 0) {
		printf("Error in ioctl\n");
	}
}

bool fls_free(long index)
{
	assert_fibers_open();
	int ret = fib_ioctl(__fibers_file, IOCTL_FLS_FREE, index);
	if (ret < 0) {
		printf("Error in ioctl\n");
	}
}

void fls_set(long index, long long value)
{
	assert_fibers_open();
	struct fls_set_data data = {
		.index = index,
		.value = value
	};

	int ret = fib_ioctl(__fibers_file, IOCTL_FLS_SET, (unsigned long)&data);
	if (ret < 0) {
		printf("Error in ioctl\n");
	}
}

long long fls_get(long index)
{
	assert_fibers_open();
	int ret = fib_ioctl(__fibers_file, IOCTL_FLS_GET, index);
	if (ret < 0) {
		printf("Error in ioctl\n");
	}
}
