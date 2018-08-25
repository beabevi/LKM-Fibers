obj-m += fibers.o
fibers-objs := ./module/fibers.o ./module/fibers_api.o

CFLAGS_fibers.o := -DDEBUG
CFLAGS_fibers_api.o := -DDEBUG

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean