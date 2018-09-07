obj-m += fibers.o
fibers-objs := ./module/fibers.o ./module/fibers_api.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -g examples/simple.c lib/fibers.c -lpthread

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm a.out
	sudo rmmod fibers

test:
	sudo insmod fibers.ko
	./a.out
