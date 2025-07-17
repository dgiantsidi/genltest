# Netlink example

This repository contains an example of how to use the Generic Netlink
communication protocol of the Linux Kernel both in kernel space and user space.


## Dependencies

```sh
sudo apt-get update
sudo apt-get install make gcc
sudo apt-get install libnl-3-dev libnl-genl-3-dev
sudo apt install pkgconf
```


## Build and run

```sh
make 
```

```sh
sudo rmmod netlink_test
sudo insmod netlink_test.ko
```

```sh
./nl_recv
```

## Results (10/06/2025)

```sh
Elapsed time: 80698039659  nanoseconds (latency per operation = 8.069804 us), msg_size=1023
Elapsed time: 51167878468  nanoseconds (latency per operation = 5.116788 us), msg_size=255
Elapsed time: 49612394604  nanoseconds (latency per operation = 4.961239 us), msg_size=255
Elapsed time: 45159785744  nanoseconds (latency per operation = 4.515979 us), msg_size=127
Elapsed time: 46151852832  nanoseconds (latency per operation = 4.615185 us), msg_size=127
Elapsed time: 42880586188  nanoseconds (latency per operation = 4.288059 us), msg_size=63
Elapsed time: 28764463421  nanoseconds (latency per operation = 2.876446 us), msg_size=63
Elapsed time: 30412958487  nanoseconds (latency per operation = 3.041296 us), msg_size=127
```