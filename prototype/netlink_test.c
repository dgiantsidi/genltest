#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <net/sock.h>

#define NOTIFY_CMTS 31
#define NETLINK_TEST 17

//static struct task_struct *kth_arr;
int thread_id = 0;

struct sock *nl_sock = NULL;
struct sock *nl_sock_notify = NULL;


static void notify_cmts(struct sk_buff *skb) {
  struct nlmsghdr *nlh;
  int msg_size;
  char *msg;
  int pid;
 

  nlh = (struct nlmsghdr *)skb->data;
  pid = nlh->nlmsg_pid; /* pid of sending process */
  msg = (char *)nlmsg_data(nlh);
  msg_size = nlh->nlmsg_len;
  int blk_id = 0;
  memcpy(&blk_id, msg, sizeof(int));
  printk(KERN_INFO "notify_cmts: Received from pid %d: [thread_id:%d] about blk_id=%d\n", pid, current->pid, blk_id);

}

static void netlink_test_recv_msg(struct sk_buff *skb) {
  struct sk_buff *skb_out;
  struct nlmsghdr *nlh;
  int msg_size;
  char *msg;
  int pid;
  int res;

  nlh = (struct nlmsghdr *)skb->data;
  pid = nlh->nlmsg_pid; /* pid of sending process */
  msg = (char *)nlmsg_data(nlh);
  msg_size = strlen(msg);
 
  // create reply
  skb_out = nlmsg_new(msg_size, 0);
  if (!skb_out) {
    printk(KERN_ERR "netlink_test: Failed to allocate new skb\n");
    return;
  }

  // put received message into reply
  nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
  NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
  strncpy(nlmsg_data(nlh), msg, msg_size);

  printk(KERN_INFO "netlink_test_recv_msg: Send %s\n", msg);

  res = nlmsg_unicast(nl_sock, skb_out, pid);
  if (res < 0)
    printk(KERN_INFO "netlink_test: Error while sending skb to user\n");
}



static int __init netlink_test_init(void) {
  printk(KERN_INFO "netlink_test: Init module %d\n", current->pid);


  struct netlink_kernel_cfg cfg = {
      .input = netlink_test_recv_msg,
  };

  
  nl_sock = netlink_kernel_create(&init_net, NETLINK_TEST, &cfg);
  if (!nl_sock) {
    printk(KERN_ALERT "netlink_test: Error creating NETLINK_TEST socket.\n");
    return -10;
  }

  #if 1
  struct netlink_kernel_cfg cfg_notify = {
      .input = notify_cmts,
  };
  nl_sock_notify = netlink_kernel_create(&init_net, NOTIFY_CMTS, &cfg_notify);
  if (!nl_sock_notify) {
    printk(KERN_ALERT "netlink_test: Error creating NOTIFY_CMTS socket.\n");
    return -10;
  }
  #endif


  return 0;
}

static void __exit netlink_test_exit(void) {
  printk(KERN_INFO "netlink_test: Exit module\n");
  printk(KERN_INFO "exiting thread module\n");
  if (nl_sock)
    netlink_kernel_release(nl_sock);
  if (nl_sock_notify)
    netlink_kernel_release(nl_sock_notify);

  
  printk(KERN_INFO "stopped the threads\n");
}

module_init(netlink_test_init);
module_exit(netlink_test_exit);

MODULE_LICENSE("GPL");