### traceroute
Utility for showing routers on the path to the destination host for Linux platform.

### Usage
`$ make && sudo ./traceroute <ip>`

### How does it work?

#### Setup a socket
Create a raw socket. This allows manipulation of IP header and creation of ICMP packet.
It requires root privileges to prevent sending spurious packets to the network.

#### Send packets
Send ICMP echo requests to the destination host with the ascending values of TTL (time to live) IP parameter starting from 1.
TTL is decreased each time packet is forwarded by the router. When it reaches zero we will get information about it from this router.

In ICMP header place process number to the `id` parameter and unique number for each packet send in `seq` parameter.
It will be send back in the reply and it enables matching received and send packets.

#### Receive packets
After sending a group of packets with the same TTL value, wait 1 second for responses.
We cannot use standard read function on socket, because it is blocking.
On the other hand, looping and calling this function with non-blocking flag waste CPU time.
`select` system call allows to wait for a file descriptor with a given timeout.

