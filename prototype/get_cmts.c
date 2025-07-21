#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <linux/netlink.h>
#include <sys/socket.h>
#include "config_c.h"

#include "msg_processing_functions.h"


static uint64_t c_total_ops = 1e6;

static uint64_t counter = 0; // static to retain value between calls

static size_t max_buffer_size() {
  return (sizeof(get_cmt_msg_t) > sizeof(recv_cmt_msg_t))? \
    sizeof(get_cmt_msg_t) : sizeof(recv_cmt_msg_t);
}

int main(int argc, char **argv) {
  struct sockaddr_nl src_addr;
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh;
  struct msghdr msg;
  struct iovec iov;
  int rc;

 
  if (argc == 1) {
    fprintf(stderr, "Usage: %s <poolname> [<total_ops>]\n", argv[0]);
    return 1;
  }
  const char* poolname = argv[1];
  printf("poolname: %s\n", poolname);
  if (argc > 2) {
    c_total_ops = atoi(argv[1]);
    printf("total operations: %lu\n", c_total_ops);
  }

  int sock_fd = socket(PF_NETLINK, SOCK_RAW, GET_CMTS_SOCK);
  if (sock_fd < 0) {
    printf("error creating the socket of type=%s, errno: %s\n", \
    get_socket_type(NOTIFY_CMTS_SOCK), \
    strerror(errno));
    return 1;
  }

  memset(&src_addr, 0, sizeof(src_addr));
  src_addr.nl_family = AF_NETLINK;
  src_addr.nl_pid = getpid(); /* self pid */
  src_addr.nl_groups = 0;     /* not in mcast groups */
  bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));

  struct timespec start, end;

  // get start time
  clock_gettime(CLOCK_MONOTONIC, &start);
  for (;;) {
    if (counter == c_total_ops) {
      break;
    }
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;    /* For Linux Kernel */
    dest_addr.nl_groups = 0; /* unicast */

    nlh = (struct nlmsghdr*) malloc(NLMSG_SPACE(max_buffer_size()));

    /* fill the netlink message header */
    nlh->nlmsg_len = NLMSG_SPACE(max_buffer_size());
    nlh->nlmsg_pid = getpid(); /* self pid */
    nlh->nlmsg_flags = 0;

    char* tx_msg = serialize_get_cmt_into_char(poolname);
    /* fill in the netlink message payload */
    memcpy(NLMSG_DATA(nlh), tx_msg, sizeof(get_cmt_msg_t));

    memset(&iov, 0, sizeof(iov));
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;

    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    printf("send to kernel: {%s, %dB}\n", poolname, nlh->nlmsg_len);
    free(tx_msg);
    rc = sendmsg(sock_fd, &msg, 0);
    if (rc < 0) {
      printf("error seding the message: %s\n", strerror(errno));
      close(sock_fd);
      return 1;
    }

    /* read message from kernel */
    memset(nlh, 0, NLMSG_SPACE(max_buffer_size()));

    rc = recvmsg(sock_fd, &msg, 0);
    if (rc < 0) {
      printf("sendmsg(): %s\n", strerror(errno));
      close(sock_fd);
      return 1;
    }

    recv_cmt_msg_t* recv_msg = deserialze_recv_cmt(NLMSG_DATA(nlh));

    printf("received from kernel: {blk_id=%ld, %s, cmt=%s}\n", \
      recv_msg->blk_id, recv_msg->poolname, recv_msg->tail_commitment);
    free(nlh);
  }

  // get end time
  clock_gettime(CLOCK_MONOTONIC, &end);

  // calculate elapsed time in seconds
  long long elapsed_ns =
      (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
  double latency_us = (elapsed_ns / 1e3) / c_total_ops; // convert to microseconds
  printf("elapsed time: %llu  nanoseconds (latency per operation = %f us), "
         "msg_size=%lu\n",
         elapsed_ns, latency_us, max_buffer_size());

  /* close Netlink Socket */
  close(sock_fd);

  return 0;
}