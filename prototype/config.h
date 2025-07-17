#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <time.h>

#include <linux/netlink.h>
#include <sys/socket.h>
#include <errno.h>


// netlink sockets definitions
#define NOTIFY_CMTS_SOCK 31
#define GET_CMTS_SOCK 17

// ---- messages definitions ----

#define ZFS_MAX_DATASET_NAME_LEN 256
#define COMMITMENT_SIZE 32 

// message sent to kernel for acked cmt/pool notification
typedef struct notify_cmt_msg {
  uint64_t acknowledged_blk_id; // block id
  char poolname[ZFS_MAX_DATASET_NAME_LEN];
} notify_cmt_msg_t;

// message sent to kernel for getting latest cmt for poolname
typedef struct get_cmt_msg {
  char poolname[ZFS_MAX_DATASET_NAME_LEN];
} get_cmt_msg_t;

// message received from kernel for new commitment
typedef struct recv_cmt_msg {
  uint64_t blk_id; // block id
  char poolname[ZFS_MAX_DATASET_NAME_LEN];
  char tail_commitment[COMMITMENT_SIZE]; 
  // todo: extend w/ uberblock and zil header commitments
} recv_cmt_msg_t;


// ---- helpers ----

static char* get_socket_type(const int socket_type) {
    switch (socket_type) {
        case NOTIFY_CMTS_SOCK:
            return "NOTIFY_CMTS_SOCK";
        case GET_CMTS_SOCK:
            return "GET_CMTS_SOCK";
        default:
            return "UNKNOWN_SOCKET_TYPE";
    }
}