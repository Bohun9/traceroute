#pragma once

#define TTL_MAX 30
#define NUM_PACKETS 3

#ifdef DEBUG
#define debug_msg(...) fprintf(stderr, __VA_ARGS__)
#else
#define debug_msg(...) ((void) 42)
#endif

