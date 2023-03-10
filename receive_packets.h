#pragma once

#include <netinet/in.h>

enum {
	PACKET_RECEIVED,
	PACKET_NOT_RECEIVED,
	PACKET_TIME_EXCEEDED
};

typedef struct response {
	int status;
	struct in_addr ip;
	int us;
} response_t;

response_t *receive_packets(int socketfd, int ttl);

