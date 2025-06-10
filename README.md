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
sudo insmod netlink_test.ko
```

```sh
./nl_recv
```

## Results (10/06/2025)

```sh
Elapsed time: 41999067380  nanoseconds (latency per operation = 4.199907 us)
```