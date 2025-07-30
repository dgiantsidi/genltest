#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <linux/netlink.h>
#include <sys/socket.h>
#include <errno.h>
#include "config_c.h"
#include "msg_processing_functions.h"

static void *notify_cmts(void *arg_poolname) {
  struct sockaddr_nl src_addr, dest_addr;
  static uint64_t c_total_ops = 1e6;
  static uint64_t counter = 0;
  struct msghdr msg;
  struct iovec iov;
  const char *poolname = (const char *)arg_poolname;

  // create socket for sending notify messages
  int sock_fd = socket(PF_NETLINK, SOCK_RAW, NOTIFY_CMTS_SOCK);
  if (sock_fd < 0) {
    printf("error creating the socket of type=%s, errno: %s\n",
           get_socket_type(NOTIFY_CMTS_SOCK), strerror(errno));
    return NULL;
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

    struct nlmsghdr *nlh =
        (struct nlmsghdr *)malloc(NLMSG_SPACE(sizeof(notify_cmt_msg_t)));

    /* fill the netlink message header */
    nlh->nlmsg_len = NLMSG_SPACE(sizeof(notify_cmt_msg_t));
    nlh->nlmsg_pid = getpid(); /* self pid */
    nlh->nlmsg_flags = 0;

    char *tx_msg = serialize_notify_cmt_into_char(poolname, counter);

    /* fill in the netlink message payload */
    memcpy(NLMSG_DATA(nlh), tx_msg, sizeof(notify_cmt_msg_t));

    memset(&iov, 0, sizeof(iov));
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;

    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    uint64_t blk_id = 0;
    memcpy(&blk_id, tx_msg, sizeof(uint64_t));
    printf("send to kernel: {%ld, %dB}\n", blk_id, nlh->nlmsg_len);

    int rc = sendmsg(sock_fd, &msg, 0);
    if (rc < 0) {
      printf("error seding the message: %s\n", strerror(errno));
      close(sock_fd);
      return NULL;
    }

    free(nlh);
    free(tx_msg);
    counter++;
  }

  // get end time
  clock_gettime(CLOCK_MONOTONIC, &end);

  // calculate elapsed time in seconds
  long long elapsed_ns =
      (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
  double latency_us =
      (elapsed_ns / 1e3) / c_total_ops; // convert to microseconds
  printf("elapsed time: %llu  nanoseconds (latency per operation = %f us), "
         "msg_size=%lu\n",
         elapsed_ns, latency_us, sizeof(notify_cmt_msg_t));

  /* close Netlink Socket */
  close(sock_fd);
  return NULL;
}

static size_t max_buffer_size() {
  return (sizeof(get_cmt_msg_t) > sizeof(recv_cmt_msg_t))
             ? sizeof(get_cmt_msg_t)
             : sizeof(recv_cmt_msg_t);
}

static void *get_cmts(void *arg_poolname) {
  static uint64_t c_total_ops = 1e6;
  const char *poolname = (const char *)arg_poolname;
  static uint64_t expected_blk_id = 0; // static to retain value between calls
  struct sockaddr_nl src_addr, dest_addr;

  struct msghdr msg;
  struct iovec iov;

  int sock_fd = socket(PF_NETLINK, SOCK_RAW, GET_CMTS_SOCK);
  if (sock_fd < 0) {
    printf("error creating the socket of type=%s, errno: %s\n",
           get_socket_type(NOTIFY_CMTS_SOCK), strerror(errno));
    return NULL;
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
    if (expected_blk_id == c_total_ops) {
      break;
    }
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;    /* For Linux Kernel */
    dest_addr.nl_groups = 0; /* unicast */

    struct nlmsghdr *nlh =
        (struct nlmsghdr *)malloc(NLMSG_SPACE(max_buffer_size()));

    /* fill the netlink message header */
    nlh->nlmsg_len = NLMSG_SPACE(max_buffer_size());
    nlh->nlmsg_pid = getpid(); /* self pid */
    nlh->nlmsg_flags = 0;

    char *tx_msg = serialize_get_cmt_into_char(poolname);
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
    int rc = sendmsg(sock_fd, &msg, 0);
    if (rc < 0) {
      printf("error seding the message: %s\n", strerror(errno));
      close(sock_fd);
      return NULL;
    }

    /* read message from kernel */
    memset(nlh, 0, NLMSG_SPACE(max_buffer_size()));

    rc = recvmsg(sock_fd, &msg, 0);
    if (rc < 0) {
      printf("sendmsg(): %s\n", strerror(errno));
      close(sock_fd);
      return NULL;
    }

    recv_cmt_msg_t *recv_msg = deserialize_recv_cmt(NLMSG_DATA(nlh));
    if (expected_blk_id != recv_msg->blk_id) {
      printf("received unexpected blk_id=%ld, expected=%ld\n", recv_msg->blk_id,
             expected_blk_id);
      exit(0);
    }
    printf("received from kernel: {blk_id=%ld, %s, cmt=%s}\n", recv_msg->blk_id,
           recv_msg->poolname, recv_msg->tail_commitment);
    free(nlh);
    expected_blk_id++;
  }

  // get end time
  clock_gettime(CLOCK_MONOTONIC, &end);

  // calculate elapsed time in seconds
  long long elapsed_ns =
      (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
  double latency_us =
      (elapsed_ns / 1e3) / c_total_ops; // convert to microseconds
  printf("elapsed time: %llu  nanoseconds (latency per operation = %f us), "
         "msg_size=%lu\n",
         elapsed_ns, latency_us, max_buffer_size());

  /* close Netlink Socket */
  close(sock_fd);
  return NULL;
}

int main(int argc, char **argv) {
  pthread_t get_cmts_thread, notify_cmts_thread;
  ;
  if (argc == 1) {
    fprintf(stderr, "Usage: %s <poolname> [<total_ops>]\n", argv[0]);
    return 1;
  }
  char *poolname = argv[1];
  printf("poolname: %s\n", poolname);

  // create two threads
  if (pthread_create(&get_cmts_thread, NULL, get_cmts, poolname) != 0) {
    perror("failed to create get_cmts_thread");
    return 1;
  }

  if (pthread_create(&notify_cmts_thread, NULL, notify_cmts, poolname) != 0) {
    perror("failed to create notify_cmts_thread");
    return 1;
  }

  // wait for both threads to finish
  pthread_join(get_cmts_thread, NULL);
  pthread_join(notify_cmts_thread, NULL);

  printf("both threads have completed.\n");
  return 0;
}