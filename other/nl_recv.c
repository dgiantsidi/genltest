#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <time.h>

#include <linux/netlink.h>
#include <sys/socket.h>

#define MAX_PAYLOAD 1024 /* maximum payload size */
#define NETLINK_TEST 17
#define TOTAL_OPS 10000000

static char message[MAX_PAYLOAD];
static int counter = 0; // static to retain value between calls
static int current_msg_size = MAX_PAYLOAD;

static char *get_message(void) {
  char tmp_message[MAX_PAYLOAD];
  memset(tmp_message, '1', MAX_PAYLOAD);
  memcpy(message, tmp_message, current_msg_size);
  message[current_msg_size - 1] = '\0'; 
  //printf(message, MAX_PAYLOAD, "%d", counter);
  counter++;
  return message;
}

int main(int argc, char **argv) {
  struct sockaddr_nl src_addr;
  struct sockaddr_nl dest_addr;
  struct nlmsghdr *nlh;
  struct msghdr msg;
  struct iovec iov;
  int rc;

  if (argc > 1) {
    current_msg_size = atoi(argv[1]);
    if (current_msg_size <= 0 || current_msg_size > MAX_PAYLOAD) {
      fprintf(stderr, "Invalid message size. Using default %d bytes.\n", MAX_PAYLOAD);
      current_msg_size = MAX_PAYLOAD;
    }
  }
  
  int sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_TEST);
  if (sock_fd < 0) {
    printf("socket: %s\n", strerror(errno));
    return 1;
  }

  memset(&src_addr, 0, sizeof(src_addr));
  src_addr.nl_family = AF_NETLINK;
  src_addr.nl_pid = getpid(); /* self pid */
  src_addr.nl_groups = 0;     /* not in mcast groups */
  bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));
  
  struct timespec start, end;
  long long  elapsed_ns;

  // Get start time
  clock_gettime(CLOCK_MONOTONIC, &start);
  char *my_msg;
  for (;;) {
    if (counter == TOTAL_OPS) {
      break; 
    }
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;    /* For Linux Kernel */
    dest_addr.nl_groups = 0; /* unicast */

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));

    /* Fill the netlink message header */
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid(); /* self pid */
    nlh->nlmsg_flags = 0;

    my_msg = get_message();
    /* Fill in the netlink message payload */
    strcpy(NLMSG_DATA(nlh), my_msg);

    memset(&iov, 0, sizeof(iov));
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;

    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    //printf("Send to kernel: %s\n", my_msg);

    rc = sendmsg(sock_fd, &msg, 0);
    if (rc < 0) {
      printf("sendmsg(): %s\n", strerror(errno));
      close(sock_fd);
      return 1;
    }

    /* Read message from kernel */
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));

    rc = recvmsg(sock_fd, &msg, 0);
    if (rc < 0) {
      printf("sendmsg(): %s\n", strerror(errno));
      close(sock_fd);
      return 1;
    }
    if (memcmp(NLMSG_DATA(nlh), my_msg, strlen(my_msg)) != 0) {
      printf("Received message does not match sent message.\n");
      return 1;
    }
    // printf("Received from kernel: %s\n", NLMSG_DATA(nlh));
    free(nlh);
  }
  
  // Get end time
  clock_gettime(CLOCK_MONOTONIC, &end);

  // Calculate elapsed time in seconds
  elapsed_ns = (end.tv_sec - start.tv_sec)*1e9 + (end.tv_nsec - start.tv_nsec);
  double latency_us = (elapsed_ns/1e3) / TOTAL_OPS; // Convert to microseconds 
  printf("Elapsed time: %llu  nanoseconds (latency per operation = %f us), msg_size=%lu\n", elapsed_ns, latency_us, strlen(my_msg));

  /* Close Netlink Socket */
  close(sock_fd);

  return 0;
}