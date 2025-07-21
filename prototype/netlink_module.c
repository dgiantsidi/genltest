#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include "config.h"



int thread_id = 0;

struct sock *nl_sock_get_cmts = NULL;
struct sock *nl_sock_notify = NULL;


static void notify_cmts(struct sk_buff *skb) {
  struct nlmsghdr* nlh = (struct nlmsghdr *)skb->data;
  int pid = nlh->nlmsg_pid; /* pid of sending process */
  char* msg = (char*) nlmsg_data(nlh);
  int msg_size = nlh->nlmsg_len;
  uint64_t blk_id = 0;
  memcpy(&blk_id, msg, sizeof(uint64_t));
  printk(KERN_INFO "notify_cmts: received from pid=%d and \
    thread_id=%d notification about blk_id=%lld \
    (msg size=%d and payload_sz=%d)\n", \
    pid, current->pid, blk_id, msg_size, (msg_size-NLMSG_HDRLEN));
}

static void get_cmts(struct sk_buff *skb) {
  struct sk_buff *skb_out;
  
  struct nlmsghdr* nlh = (struct nlmsghdr *)skb->data;
  int pid = nlh->nlmsg_pid; /* pid of sending process */
  char* msg = (char*) nlmsg_data(nlh);
  int msg_size = nlh->nlmsg_len;
 
  // create reply
  skb_out = nlmsg_new(msg_size, 0);
  if (!skb_out) {
    printk(KERN_ERR "get_cmts: failed to allocate new skb\n");
    return;
  }

  // put received message into reply
  nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
  NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
  strncpy(nlmsg_data(nlh), msg, msg_size);

  printk(KERN_INFO "get_cmts: send %s\n", msg);

  int res = nlmsg_unicast(nl_sock_get_cmts, skb_out, pid);
  if (res < 0)
    printk(KERN_INFO "get_cmts: error while sending skb to user\n");
}



static int __init netlink_module_init(void) {
  printk(KERN_INFO "netlink_module: init module from pid=%d\n", current->pid);


  struct netlink_kernel_cfg cfg_get_cmts = {
      .input = get_cmts,
  };

  
  nl_sock_get_cmts = netlink_kernel_create(&init_net, GET_CMTS_SOCK, &cfg_get_cmts);
  if (!nl_sock_get_cmts) {
    printk(KERN_ALERT "netlink_module: error creating NETLINK_TEST socket.\n");
    return -10;
  }

  #if 1
  struct netlink_kernel_cfg cfg_notify_cmts = {
      .input = notify_cmts,
  };
  nl_sock_notify = netlink_kernel_create(&init_net, NOTIFY_CMTS_SOCK, &cfg_notify_cmts);
  if (!nl_sock_notify) {
    printk(KERN_ALERT "netlink_module: error creating NOTIFY_CMTS_SOCK socket.\n");
    return -10;
  }
  #endif


  return 0;
}

static void __exit netlink_module_exit(void) {
  printk(KERN_INFO "netlink_module: exit module\n");
  if (nl_sock_get_cmts)
    netlink_kernel_release(nl_sock_get_cmts);
  if (nl_sock_notify)
    netlink_kernel_release(nl_sock_notify);

  
  printk(KERN_INFO "netlink_module: sockets closed\n");
}

module_init(netlink_module_init);
module_exit(netlink_module_exit);

MODULE_LICENSE("GPL");