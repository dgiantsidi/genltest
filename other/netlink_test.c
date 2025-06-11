#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <net/sock.h>

#define NETLINK_TEST 17
static struct task_struct *kth_arr;
int thread_id = 0;

struct sock *nl_sock = NULL;

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
  // if (msg_size != 1024 && msg_size != 1023) {
  //   printk(KERN_ERR "netlink_test: Received message size is not 1024 bytes
  //   msg=%d\n", msg_size);
  // return;
  //}

  // printk(KERN_INFO "netlink_test: Received from pid %d: [thread_id:%d]\n",
  // pid, current->pid);

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

  // printk(KERN_INFO "netlink_test: Send %s\n", msg);

  res = nlmsg_unicast(nl_sock, skb_out, pid);
  if (res < 0)
    printk(KERN_INFO "netlink_test: Error while sending skb to user\n");
}

// long running function to be executed inside a thread, this will run for 30
// secs.
static int thread_function(void *idx) {
  printk(KERN_INFO "thread %d started [thread_id=%d]\n", thread_id,
         (kth_arr)->pid);

  // kthread_should_stop call is important.
  while (!kthread_should_stop()) {
    // printk(KERN_INFO "Thread %d Still running...! %d secs\n", t_id, i);
    // i++;

    msleep(1000);
  }

  printk(KERN_INFO "thread %d stopped\n", thread_id);
  netlink_kernel_release(nl_sock);

  return 100;
}

// initialize one thread at a time.
static int initialize_thread(void) {

  char th_name[20];
  sprintf(th_name, "kthread_%d", thread_id);
  kth_arr = kthread_create(thread_function, NULL, (const char *)th_name);
  if ((kth_arr) != NULL) {
    wake_up_process(kth_arr);
    printk(KERN_INFO "%s is running\n", th_name);
  } else {
    printk(KERN_INFO "kthread %s could not be created\n", th_name);
    return -1;
  }

  return 0;
}

static int __init netlink_test_init(void) {
  printk(KERN_INFO "netlink_test: Init module %d\n", current->pid);
  if (initialize_thread() == -1) {
    return -1;
  }

  struct netlink_kernel_cfg cfg = {
      .input = netlink_test_recv_msg,
  };

  nl_sock = netlink_kernel_create(&init_net, NETLINK_TEST, &cfg);
  if (!nl_sock) {
    printk(KERN_ALERT "netlink_test: Error creating socket.\n");
    return -10;
  }

  return 0;
}

static void __exit netlink_test_exit(void) {
  printk(KERN_INFO "netlink_test: Exit module\n");
  printk(KERN_INFO "exiting thread module\n");

  // stop all of the threads before removing the module.
  int ret = kthread_stop(kth_arr);

  printk("thread returns %d", ret);

  printk(KERN_INFO "stopped the threads\n");
}

module_init(netlink_test_init);
module_exit(netlink_test_exit);

MODULE_LICENSE("GPL");