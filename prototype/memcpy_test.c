// #include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <time.h>

#include <linux/netlink.h>
#include <sys/socket.h>
#include <errno.h>
#define MAX_PAYLOAD 1024 /* maximum payload size */
#define NOTIFY_CMTS 31
#define TOTAL_OPS 1000000

static char message[MAX_PAYLOAD];
static int counter = 0; // static to retain value between calls
static int current_msg_size = MAX_PAYLOAD;

static char *get_message(void) {

  printf("[DEBUG] counter = %d\n", counter);
#if 1
  char tmp_message[MAX_PAYLOAD];
  memset(tmp_message, '1', current_msg_size);
  memcpy(tmp_message, &counter, sizeof(int));
  memset(tmp_message + sizeof(int), '1', current_msg_size - sizeof(int));
  memcpy(message, tmp_message, current_msg_size);
  message[current_msg_size - 1] = '\0';
#endif
  printf(">> Send to kernel: %d, size=%ld counter=%d, current_msg_size=%d\n",
         counter, strlen(message), counter, current_msg_size);
  counter++;
  return message;
}

int main(int argc, char **argv) {

  if (argc > 1) {
    current_msg_size = atoi(argv[1]);
    if (current_msg_size <= 0 || current_msg_size > MAX_PAYLOAD) {
      fprintf(stderr, "Invalid message size, we use default %d bytes.\n",
              MAX_PAYLOAD);
      current_msg_size = MAX_PAYLOAD;
    }
  }

  char *my_msg;
  for (;;) {
    printf("[DEBUG] counter = %d\n", counter);
    if (counter == TOTAL_OPS) {
      break;
    }

    my_msg = get_message();
    // get_message();

    int blk_id = 0;
    // memcpy(&blk_id, my_msg, sizeof(int));
    printf("Send to kernel: %d, size=%ld counter=%d\n", blk_id, strlen(my_msg),
           counter);
  }

  return 0;
}