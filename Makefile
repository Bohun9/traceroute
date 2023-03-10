CC := gcc
CFLAGS := -std=gnu17 -Wall -Wextra

ifdef DEBUG
 CFLAGS := $(CFLAGS) -DDEBUG
endif

all: traceroute

# executable
traceroute: traceroute.o send_packets.o receive_packets.o

%:
	$(CC) $^ -o $@

# objects
traceroute.o: traceroute.c traceroute.h send_packets.h receive_packets.h
send_packets.o: send_packets.c send_packets.h traceroute.h 
receive_packets.o: receive_packets.c receive_packets.h traceroute.h 

%.o: 
	$(CC) $(CFLAGS) -c $<


# phony rules
.PHONY: clean distclean all

clean:
	rm -f *.o

distclean: clean
	rm -f traceroute

