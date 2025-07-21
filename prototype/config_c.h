#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <linux/netlink.h>
#include <sys/socket.h>
#include <errno.h>
#include "config.h"


__attribute__((__maybe_unused__))  static void randomized_sleeps() {
    // sleep for a random time between 0 and 100 microseconds
    int sleep_time = rand() % 100; // in microseconds
    usleep(sleep_time);
}

// ---- helpers ----

__attribute__((__maybe_unused__))  static const char* get_socket_type(const int socket_type) {
    switch (socket_type) {
        case NOTIFY_CMTS_SOCK:
            return "NOTIFY_CMTS_SOCK";
        case GET_CMTS_SOCK:
            return "GET_CMTS_SOCK";
        default:
            return "UNKNOWN_SOCKET_TYPE";
    }
}