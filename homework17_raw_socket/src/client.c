#include "../hdr/functions.h"
// #include <netinet/ip.h>
int client(void)
{
    puts("Client");

    /*
    * Declare:
    * - server_fd - fd of server socket;
    * - server - server's endpoint;
    * - endpoint_size - endpoint length, passed to recvfrom as an argument;
    * - send_packet & recv_packet - message buffers for sending and receiving 
    *   packets correspondently;
    * - ip_header - ip layer header, received from recvfrom function;
    * - udp_header - transport layer header, used in sending and receiving a 
    *   packet;
    * - msg - straight pointer to payload in a packet.
    */
    int server_fd;
    struct sockaddr_in server;
    socklen_t endpoint_size;
    char send_packet[SEND_PACKET_LEN+1];
    char recv_packet[RECV_PACKET_LEN+1];
    struct iphdr *ip_header;
    struct udphdr *udp_header;
    char *msg;

    /* Fill 'server', 'send_packet' and recv_packet' with nulls */
	memset(&server, NULL, sizeof(server));
    memset(&send_packet, NULL, SEND_PACKET_LEN);
    memset(&recv_packet, NULL, RECV_PACKET_LEN);

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

    /*
    * Assign address to 'udp_header' and 'msg' pointers, fill these structures 
    * with data.
    */
    udp_header = (struct udphdr *)&send_packet;
    msg = send_packet+sizeof(struct udphdr);

    udp_header->source = htons(CLIENT_PORT);
    udp_header->dest = htons(SERVER_PORT);
    udp_header->len = htons(SEND_PACKET_LEN);
    udp_header->check = 0;

    strncpy(msg, CLIENT_MSG, MSG_SIZE);

    /* Send packet to a server */
    if (sendto(server_fd, send_packet, SEND_PACKET_LEN, NULL, 
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
        recvfrom(server_fd, recv_packet, RECV_PACKET_LEN, 0, 
                (struct sockaddr*)&server, &endpoint_size);

        /* Assign pointers to easily parse data */
        ip_header = (struct iphdr *)&recv_packet;
        udp_header = (struct udphdr *)(recv_packet+sizeof(struct iphdr));
        msg = recv_packet+sizeof(struct iphdr)+sizeof(struct udphdr);

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
                exit(EXIT_FAILURE);
            }
            printf("Received message <%s> from server %s:%d\n", msg, addr, 
                    ntohs(udp_header->source));
            break;
        }
    }

	return EXIT_SUCCESS;
}
