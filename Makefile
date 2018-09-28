EXTRA_CFLAGS := -I$(src)/include/module

obj-m += fibers.o
fibers-objs := module/fibers.o module/fibers_api.o module/proc.o

pretty-files := lib/fibers.c                \
                module/fibers.c             \
                module/fibers_api.c         \
                module/proc.c               \
                include/module/common.h     \
                include/module/fibers_api.h \
                include/module/fibers.h     \
                include/module/proc.h       \
                include/module/klog.h

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -g examples/simple.c lib/fibers.c util/log.c -lpthread
	make -C examples/2018-fibers

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm examples/2018-fibers/test
	sudo rmmod fibers
	sudo dmesg -C

instest:
	sudo insmod fibers.ko
	./examples/2018-fibers/test 100

test:
	./examples/2018-fibers/test 100

pretty:
	indent -linux $(pretty-files)

pretty-clean:
	find . -type f -name '*~' -delete
