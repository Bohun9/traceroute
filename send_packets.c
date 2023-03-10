// Marcin Martowicz 321990

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#include "traceroute.h"
#include "send_packets.h"

static u_int16_t compute_icmp_checksum (const void *buff, int length) {
	u_int32_t sum;
	const u_int16_t* ptr = buff;
	assert (length % 2 == 0);
	for (sum = 0; length > 0; length -= 2)
		sum += *ptr++;
	sum = (sum >> 16) + (sum & 0xffff);
	return (u_int16_t)(~(sum + (sum >> 16)));
}

static void send_packet(int socketfd, struct sockaddr_in addr, int ttl, int seq) {
    if (setsockopt(socketfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int)) < 0) {
        fprintf(stderr, "setsockopt error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct icmp header;
    header.icmp_type = ICMP_ECHO;
    header.icmp_code = 0;
    header.icmp_hun.ih_idseq.icd_id = getpid() & 0xFFFF;
    header.icmp_hun.ih_idseq.icd_seq = NUM_PACKETS * (ttl - 1) + seq;
    header.icmp_cksum = 0;
    header.icmp_cksum = compute_icmp_checksum((u_int16_t *) &header, sizeof(header));

    if (sendto(socketfd, &header, sizeof(header), 0, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        fprintf(stderr, "sendto error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    debug_msg("[PACKET SEND] id:%d ttl:%d seq:%d\n", header.icmp_id, ttl, seq);
}

void send_packets(int socketfd, struct sockaddr_in addr, int ttl, int n) {
    for (int i = 0; i < n; i++) {
        send_packet(socketfd, addr, ttl, i);
    }
}

