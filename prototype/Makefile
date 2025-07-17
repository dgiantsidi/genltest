KERNEL_DIR=/usr/src/kernel-headers-$(shell uname -r)
obj-m += netlink_test.o
ccflags-y := -std=gnu99 -O3 -Wno-declaration-after-statement

all: 
	$(CC) notify_cmts.c -o notify_cmts
	$(CC) memcpy_test.c -o memcpy_test
	$(CC) get_cmts.c -o get_cmts
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	rm -f get_cmts
	rm -f notify_cmts
	rm -f memcpy_test
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean