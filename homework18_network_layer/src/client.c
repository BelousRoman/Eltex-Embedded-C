#include "../hdr/functions.h"

int client(void)
{
    puts("Client");

    /*
    * Declare:
    * - server_fd - fd of server socket;
    * - server - server's endpoint;
    * - endpoint_size - endpoint length, passed to recvfrom as an argument;
    * - packet & packet - message buffers for sending and receiving 
    *   packets correspondently;
    * - ip_header - network layer header, used in sending and receiving a
    *   packet;
    * - udp_header - transport layer header, used in sending and receiving a
    *   packet;
    * - msg - straight pointer to payload in a packet.
    */
    int server_fd;
    struct sockaddr_in server;
    socklen_t endpoint_size;
    char packet[PACKET_LEN+1];
    struct iphdr *ip_header;
    struct udphdr *udp_header;
    char *msg;

    /* Fill 'server', 'packet' and packet' with 0's */
	memset(&server, 0, sizeof(server));
    memset(&packet, 0, PACKET_LEN);

	/* Set server's endpoint */
	server.sin_family = AF_INET;
	if (inet_pton(AF_INET, SERVER_ADDR, &server.sin_addr) == -1)
	{
		perror("inet_pton");
        return EXIT_FAILURE;
	}
	server.sin_port = htons(SERVER_PORT);

    /* Create socket */
	server_fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (server_fd == -1)
    {
        printf("socket: %s (%d)\n", strerror(errno), errno);
        return EXIT_FAILURE;
    }

    /* Set socket option to fill network layer header manually */
    setsockopt(server_fd, IPPROTO_IP, IP_HDRINCL, &(int){1}, sizeof(int));

    /*
    * Assign address to 'ip_header', 'udp_header' and 'msg' pointers, fill
    * these structures with data.
    */
    ip_header = (struct iphdr *)&packet;
    udp_header = (struct udphdr *)(packet+IPHDR_SIZE);
    msg = packet+IPHDR_SIZE+UDPHDR_SIZE;

    /* 4 bits long IP protocol version */
    ip_header->version = 4; 
    /*
    * 4 bits long internet header length, multiplies by 4, i.e. 5*4 = 20 bytes.
    */
    ip_header->ihl = 5;
    /* 1 byte long type of service*/
    ip_header->tos = 0;
    /* 2 bytes long total length, always filled in automatically */
    ip_header->tot_len = 0;
    /* 2 bytes long ID, filled in, when zero */
    ip_header->id = 0;
    /*
    * 2 byte long flags + offset:
    * - 1st bit is reserved and should be set to 0;
    * - 2nd bit is dont fragment flag, set to 0 for packets fragmentation, in
    *   this case, manual, otherwise, set to 1;
    * - 3rd bit is more fragments flag, 0 if this is the last fragment of
    *   packet, 1 if not;
    * - 4-16 bits contains offset, received value multiplies by 8 to be able to
    *   set arbitrary offset up to 64Kb.
    */
    ip_header->frag_off = htons(0b0 | IPHDR_DONT_FRAGMENT_FLAG(0)
                                | IPHDR_MORE_FRAGMENTS_FLAG(0));
    /* 1 byte long time-to-live */
    ip_header->ttl = IPHDR_TTL;
    /* 1 byte long transport layer protocol */
    ip_header->protocol = IPPROTO_UDP;
    /* 2 bytes long checksum, always filled in automatically */
    ip_header->check = 0;
    /* 4 bytes long source address, filled in, when zero */
    ip_header->saddr = 0;
    /* 4 bytes long destination address */
    if (inet_pton(AF_INET, SERVER_ADDR, &ip_header->daddr) == -1)
	{
		perror("inet_pton");
        return EXIT_FAILURE;
	}

    /* 2 bytes long source port */
    udp_header->source = htons(CLIENT_PORT);
    /* 2 bytes long destination port */
    udp_header->dest = htons(SERVER_PORT);
    /* 2 bytes long UDP header + payload length */
    udp_header->len = htons(UDPHDR_SIZE+MSG_SIZE);
    /* 2 bytes long checksum, can be set 0 for UDP */
    udp_header->check = 0;

    strncpy(msg, CLIENT_MSG, MSG_SIZE);

    /* Send packet to a server */
    if (sendto(server_fd, packet, PACKET_LEN, 0, 
        (struct sockaddr*)&server, sizeof(server)) == -1)
    {
        printf("sendto: %s (%d)\n", strerror(errno), errno);
        return EXIT_FAILURE;
    }

    printf("Sent message <%s> to server %s:%d\n", msg, SERVER_ADDR, SERVER_PORT);

    while (1)
    {
        /* Receive a packet from server*/
        endpoint_size = sizeof(struct sockaddr_in);
        recvfrom(server_fd, packet, PACKET_LEN, 0, 
                (struct sockaddr*)&server, &endpoint_size);

        /* Assign pointers to easily parse data */
        ip_header = (struct iphdr *)&packet;
        udp_header = (struct udphdr *)(packet+IPHDR_SIZE);
        msg = packet+IPHDR_SIZE+UDPHDR_SIZE;

        /*
        * If destination port in packet is equal to defined client's port - 
        * print message and end loop.
        */
        if (ntohs(udp_header->dest) == CLIENT_PORT)
        {
            char addr[INET6_ADDRSTRLEN];
            if (inet_ntop(AF_INET, &ip_header->saddr, addr, INET6_ADDRSTRLEN) 
                == NULL)
            {
                perror("inet_ntop");
                return EXIT_FAILURE;
            }
            printf("Received message <%s> from server %s:%d\n", msg, addr, 
                    ntohs(udp_header->source));
            break;
        }
    }

	return EXIT_SUCCESS;
}
