// Marcin Martowicz 321990

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>

#include "traceroute.h"
#include "send_packets.h"
#include "receive_packets.h"

void report(response_t *response, int ttl) {
    char msg[64 * NUM_PACKETS];
    sprintf(msg, "%d.", ttl);
    
    int cnt_received = 0;
    int total_us = 0;

    for (int i = 0; i < NUM_PACKETS; i++) {
        if (response[i].status == PACKET_NOT_RECEIVED) {
            continue;
        }

        cnt_received++;
        total_us += response[i].us;

        bool unique = true;
        for (int j = 0; j < i; j++) {
            if (response[i].status != PACKET_NOT_RECEIVED) {
                unique &= response[i].ip.s_addr != response[j].ip.s_addr;
            }
        }

        if (unique) {
            strcat(msg, " ");
            strcat(msg, inet_ntoa(response[i].ip));
        }
    }

    if (cnt_received == 0) {
        strcat(msg, " *");
    } else if (cnt_received < NUM_PACKETS) {
        strcat(msg, " ???");
    } else {
        char avg[64];
        sprintf(avg, " %d ms", total_us / NUM_PACKETS / 1000);
        strcat(msg, avg);
    }

    printf("%s\n", msg);
}

bool destination_reached(response_t *response) {
    bool dest = false;

    for (int i = 0; i < NUM_PACKETS; i++) {
        if (response[i].status == PACKET_TIME_EXCEEDED) {
            return false;
        }
        dest |= response[i].status == PACKET_RECEIVED;
    }

    return dest;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <ip>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;

    if (!inet_pton(AF_INET, argv[1], &(addr.sin_addr))) {
        fprintf(stderr, "ip argument is not valid IPv4 address\n");
        exit(EXIT_FAILURE);
    }

    int socketfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    if (socketfd < 0) {
        fprintf(stderr, "socket error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    debug_msg("[START]\n");
    debug_msg("[INFO] pid:[%d] sockfd:%d\n", getpid(), socketfd);

    for (int ttl = 1; ttl <= TTL_MAX; ttl++) {
        send_packets(socketfd, addr, ttl, NUM_PACKETS);
        response_t *response = receive_packets(socketfd, ttl);
        report(response, ttl);

        if (destination_reached(response)) {
            break;
        }
    }

    return EXIT_SUCCESS;
}

