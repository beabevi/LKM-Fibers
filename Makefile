obj-m += fibers.o
fibers-objs := ./module/fibers.o ./module/fibers_api.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -g examples/simple.c lib/fibers.c util/log.c -lpthread
	make -C examples/2018-fibers

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	make -C examples/2018-fibers clean
	sudo rmmod fibers
	sudo dmesg -C

instest:
	sudo insmod fibers.ko
	./examples/2018-fibers/test 100

test:
	./examples/2018-fibers/test 100