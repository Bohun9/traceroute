// Marcin Martowicz 321990

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <sys/select.h>
#include <assert.h>
#include <stdbool.h>

#include "traceroute.h"
#include "receive_packets.h"

static uint8_t buffer[IP_MAXPACKET];

static response_t response[NUM_PACKETS];

#ifdef DEBUG
static char *string_status(int s) {
   if (s == PACKET_RECEIVED) {
        return "PACKET RECEIVED";
    }
    if (s == PACKET_NOT_RECEIVED) {
        return "PACKET NOT RECEIVED";
    }
    if (s == PACKET_TIME_EXCEEDED) {
        return "PACKET TIME EXCEEDED";
    }
    assert(false);
}
#endif

static void process_packet(struct sockaddr_in *addr, struct timeval *time, int ttl) {
    struct ip *ip_header = (struct ip *) buffer;
    struct icmp *icmp_header = (struct icmp *) (buffer + 4 * ip_header->ip_hl);

    if (ip_header->ip_p != IPPROTO_ICMP) {
        debug_msg("[NOT ICMP PACKET]\n");
        return;
    }

    int packet_type = icmp_header->icmp_type;

    if (packet_type == ICMP_TIME_EXCEEDED) {
        struct ip *ip_inner = (struct ip *) (((uint8_t *) icmp_header) + 8);
        icmp_header = (struct icmp *) (((uint8_t *) ip_inner) + 4 * ip_inner->ip_hl);
    }

    int packet_id = icmp_header->icmp_id;
    int packet_ttl = icmp_header->icmp_seq / NUM_PACKETS + 1;
    int packet_seq = icmp_header->icmp_seq % NUM_PACKETS;

    if (packet_id != (getpid() & 0xFFFF)) {
        debug_msg("[FOREIGN PACKET] id:%d\n", packet_id);
        return;
    }

    if (packet_ttl != ttl) {
        debug_msg("[LATE PACKET] ttl:%d seq:%d\n", packet_ttl, packet_seq);
        return;
    }

    switch (packet_type) {
        case ICMP_ECHOREPLY:
            response[packet_seq].status = PACKET_RECEIVED;
            break;

        case ICMP_TIME_EXCEEDED:
            response[packet_seq].status = PACKET_TIME_EXCEEDED;
            break;

        default:
            debug_msg("[NOT SUPPORTED ICMP TYPE PACKET]\n");
            return;
    };

    response[packet_seq].ip = addr->sin_addr;
    response[packet_seq].us = 1000000 * (1 - time->tv_sec) - time->tv_usec;

    debug_msg("[%s] ttl:%d seq:%d\n", string_status(response[packet_seq].status), packet_ttl, packet_seq);
}

response_t *receive_packets(int socketfd, int ttl) {
    struct timeval time = {
        .tv_sec = 1,
        .tv_usec = 0
    };

    for (int i = 0; i < NUM_PACKETS; i++) {
        response[i].status = PACKET_NOT_RECEIVED;
    }

    for (;;) {
        fd_set descriptors;
        FD_ZERO (&descriptors);
        FD_SET (socketfd, &descriptors);

        int ready = select(socketfd + 1, &descriptors, NULL, NULL, &time);

        if (ready < 0) {
            fprintf(stderr, "select error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (ready == 0) {
            break;
        }

        for (;;) {
            struct sockaddr_in addr;
            socklen_t addr_len = sizeof(addr);

            ssize_t packet_len = recvfrom(socketfd, buffer, sizeof(buffer), MSG_DONTWAIT, (struct sockaddr *) &addr, &addr_len);

            if (packet_len < 0) {
                if (errno == EWOULDBLOCK) {
                    break;
                } else {
                    fprintf(stderr, "recvfrom error: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
            }

            process_packet(&addr, &time, ttl);
        }
    }

    return response;
}

