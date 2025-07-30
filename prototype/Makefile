KERNEL_DIR=/usr/src/kernel-headers-$(shell uname -r)
obj-m += netlink_module.o
ccflags-y := -std=gnu99 -O3 -Wno-declaration-after-statement
CXX_FLAGS := -O3 -Wno-declaration-after-statement -fsanitize=address -g -Wall -Wextra -pthread  

all: 
	$(CC)  $(CXX_FLAGS) notify_cmts.c -o notify_cmts
	$(CC)  $(CXX_FLAGS)  user_space_proc.c -o user_space_proc
	$(CXX) $(CXX_FLAGS) user_space_proc.cc -o user_space_proc_cpp -fsanitize=address
	$(CC) $(CXX_FLAGS) memcpy_test.c -o memcpy_test
	$(CC) $(CXX_FLAGS) get_cmts.c -o get_cmts
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	rm -f notify_cmts user_space_proc user_space_proc_cpp memcpy_test get_cmts output.txt
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean