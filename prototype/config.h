#pragma once

// netlink sockets definitions
#define NOTIFY_CMTS_SOCK 31
#define GET_CMTS_SOCK 17
#define GET_UBCMTS_SOCK 23
#define NOTIFY_UBCMTS_SOCK 24

// ---- messages definitions ----

#define ZFS_MAX_DATASET_NAME_LEN 256
#define COMMITMENT_SIZE 32
// do not change this (from uberblock_impl.h) as it is used for serialization/deserialization of uberblocks cmts
#define HEX_PER_UINT8 2
#define UBERBLOCK_DIGEST_BUF_SIZE SHA256_DIGEST_LENGTH * HEX_PER_UINT8 + 1


// message sent to kernel for acked cmt/pool notification
typedef struct notify_cmt_msg {
  uint64_t acknowledged_blk_id; // block id
  char poolname[ZFS_MAX_DATASET_NAME_LEN];
} notify_cmt_msg_t;

// message sent to kernel for getting latest cmt for poolname
typedef struct get_cmt_msg {
  char poolname[ZFS_MAX_DATASET_NAME_LEN];
} get_cmt_msg_t;

enum block_type {
  TAIL = 0,
  UB = 1,
  REGISTRATION = 2
};
// message received from kernel for new commitment
typedef struct recv_cmt_msg {
  uint64_t blk_id; // block id
  char poolname[ZFS_MAX_DATASET_NAME_LEN];
  char tail_commitment[COMMITMENT_SIZE];
  int blk_type;
  uint64_t zil_head_blk_id;
  char ub_digest[UBERBLOCK_DIGEST_BUF_SIZE];
} recv_cmt_msg_t;
