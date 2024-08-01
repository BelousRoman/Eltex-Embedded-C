#include "../hdr/functions.h"

int client(void)
{
    puts("Client");

    /*
    * Declare:
    * - server_fd - fd of server socket;
    * - server - server's endpoint;
    * - endpoint_size - endpoint length, passed to recvfrom as an argument;
    * - src_mac & dest_mac - char buffers to store client's and server's MACs;
    * - packet - message buffer for sending and receiving packets
    *   correspondently;
    * - eth_header - link layer header, used in sending and receiving a packet;
    * - ip_header - network layer header, used in sending and receiving a
    *   packet;
    * - udp_header - transport layer header, used in sending and receiving a
    *   packet;
    * - msg - straight pointer to payload in a packet.
    * - ifs - pointer to dynamically allocated array of network interfaces,
    *   used in sending a packet to server;
    * - if_path - dynamically allocated array of chars, used to store path to a
    *   file, containing IF MAC address;
    * - if_file - pointer to above-mentioned file with IF MAC;
    * - temp_csum - temporary variable, to store sum of ip header fields;
    * - csum_ptr - pointer to header's checksum;
    * - index - index variable, used in for loops.
    */
    int server_fd;
    struct sockaddr_ll server;
    socklen_t endpoint_size;
    char src_mac[ETH_ALEN];
    char dest_mac[ETH_ALEN];
    char packet[PACKET_LEN+1];
    struct ether_header *eth_header;
    struct iphdr *ip_header;
    struct udphdr *udp_header;
    char *msg;
    struct if_nameindex *ifs = NULL;
    char *if_path = NULL;
    FILE *if_file = NULL;
    int temp_csum = 0;
    short *csum_ptr = NULL;
    int index;

    /* Fill 'server', 'packet' and packet' with 0's */
	memset(&server, 0, sizeof(server));
    memset(&packet, 0, PACKET_LEN);

    /* Create socket */
	server_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (server_fd == -1)
    {
        printf("socket: %s (%d)\n", strerror(errno), errno);
        return EXIT_FAILURE;
    }

    /* Set server's endpoint */
    server.sll_family = AF_PACKET;
    server.sll_protocol = ETH_P_IP;
    /*
    * Search for IF index by name, provided by header constant, if IF with such
    * name could not be found - get array of all interfaces and go through all
    * elements, searching for first not local IF, then allocate and set 
    * 'if_name' to later get this interface MAC address.
    * */
    if ((server.sll_ifindex = if_nametoindex(ETHHDR_IF_NAME)) == 0)
    {
        perror("if_nametoindex");

        ifs = if_nameindex();
        if (ifs == NULL)
        {
            perror("if_nameindex");
            return EXIT_FAILURE;
        }
        for (index = 0; ifs[index].if_index != 0; ++index)
        {
            if (strncmp(ifs[index].if_name, ETHHDR_LOCAL_IF_NAME,
                strlen(ifs[index].if_name)) != 0)
            {
                server.sll_ifindex = ifs[index].if_index;
                if_path = malloc(IF_PATH_TEMPLATE_SIZE+
                                    strlen(ifs[index].if_name)+1);
                if (if_path == NULL)
                {
                    perror("malloc");
                    return EXIT_FAILURE;
                }
                snprintf(if_path, IF_PATH_TEMPLATE_SIZE+
                            strlen(ifs[index].if_name), IF_PATH_TEMPLATE,
                            ifs[index].if_name);
                break;
            }
        }
        for (index = 0; ifs[index].if_index != 0; ++index)
            free(ifs[index].if_name);
        
        free(ifs);
    }
    else
    {
        if_path = malloc(IF_PATH_TEMPLATE_SIZE+strlen(ETHHDR_IF_NAME)+1);
        if (if_path == NULL)
        {
            perror("malloc");
            return EXIT_FAILURE;
        }
        snprintf(if_path, IF_PATH_TEMPLATE_SIZE+strlen(ETHHDR_IF_NAME),
                IF_PATH_TEMPLATE, ETHHDR_IF_NAME);
    }

    /* Open file, containing this IF MAC address */
    if_file = fopen(if_path, "r");
    if (if_file == NULL)
    {
        printf("fopen %s: %s (%d)\n", if_path, strerror(errno), errno);
        return EXIT_FAILURE;
    }

    /* Parse file contents to fill 'src_mac' array */
    if (fscanf(if_file, "%x:%x:%x:%x:%x:%x", &src_mac[0], &src_mac[1], 
        &src_mac[2], &src_mac[3], &src_mac[4], &src_mac[5]) != ETH_ALEN)
    {
        perror("source MAC fscanf");
        return EXIT_FAILURE;
    }

    /* Close file, free memory */
    fclose(if_file);
    free(if_path);

    /* Fill 'dest_mac' with destination MAC address by header constant */
    if (sscanf(ETHHDR_DEST_MAC, "%x:%x:%x:%x:%x:%x", &dest_mac[0], 
        &dest_mac[1], &dest_mac[2], &dest_mac[3], &dest_mac[4], &dest_mac[5]) 
        != ETH_ALEN)
    {
        perror("destination MAC sscanf");
        return EXIT_FAILURE;
    }

    memcpy(&server.sll_addr, dest_mac, ETH_ALEN);

    server.sll_halen = ETH_ALEN;

    /*
    * Assign address to 'eth_header', 'ip_header', 'udp_header' and 'msg'
    * pointers, fill these structures with data.
    */
    eth_header = (struct ether_header *)&packet;
    ip_header = (struct iphdr *)(packet+ETHHDR_SIZE);
    udp_header = (struct udphdr *)(packet+ETHHDR_SIZE+IPHDR_SIZE);
    msg = packet+ETHHDR_SIZE+IPHDR_SIZE+UDPHDR_SIZE;

    /* 6 bytes long destination MAC */
    memcpy(&eth_header->ether_dhost, dest_mac, ETH_ALEN);
    /* 6 bytes long source MAC */
    memcpy(&eth_header->ether_shost, src_mac, ETH_ALEN);
    /* 2 bytes long Ethernet protocol */
    eth_header->ether_type = htons(ETH_P_IP);

    /* 4 bits long IP protocol version */
    ip_header->version = IPHDR_IP_VERSION; 
    /*
    * 4 bits long internet header length, multiplies by 4, i.e. 5*4 = 20 bytes.
    */
    ip_header->ihl = IPHDR_IHL;
    /* 1 byte long type of service*/
    ip_header->tos = IPHDR_TOS;
    /* 2 bytes long total length, not filled in while AF_PACKET is set */
    ip_header->tot_len = htons((ip_header->ihl*4)+UDPHDR_SIZE+MSG_SIZE);
    /* 2 bytes long ID, not filled in while AF_PACKET is set */
    ip_header->id = htons(IPHDR_ID);
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
    ip_header->frag_off = htons(0x0000 | IPHDR_DONT_FRAGMENT_FLAG);
    /* 1 byte long time-to-live */
    ip_header->ttl = IPHDR_TTL;
    /* 1 byte long transport layer protocol */
    ip_header->protocol = IPPROTO_UDP;
    /* 2 bytes long checksum, not filled in while AF_PACKET is set */
    ip_header->check = 0;
    /* 4 bytes long source address, not filled in while AF_PACKET is set */
    if (inet_pton(AF_INET, CLIENT_ADDR, &ip_header->saddr) == -1)
	{
		perror("inet_pton");
        return EXIT_FAILURE;
	}
    /* 4 bytes long destination address */
    if (inet_pton(AF_INET, SERVER_ADDR, &ip_header->daddr) == -1)
	{
		perror("inet_pton");
        return EXIT_FAILURE;
	}

    /* Count IP header's checksum */
    csum_ptr = (short *)ip_header;
    for (int i = 0; i < 10;++i)
    {
        temp_csum = temp_csum + *csum_ptr;
        csum_ptr++;
    }
    ip_header->check = (temp_csum & 0xFFFF) + (short)(temp_csum >> 16);
    ip_header->check = ~ip_header->check;

    /* 2 bytes long source port */
    udp_header->source = htons(CLIENT_PORT);
    /* 2 bytes long destination port */
    udp_header->dest = htons(SERVER_PORT);
    /* 2 bytes long UDP header + payload length */
    udp_header->len = htons(UDPHDR_SIZE+MSG_SIZE);
    /* 2 bytes long checksum, can be set to 0 for UDP protocol */
    udp_header->check = 0;

    strncpy(msg, CLIENT_MSG, MSG_SIZE);

    /* Send packet to a server */
    if (sendto(server_fd, packet, PACKET_LEN, 0, 
        (struct sockaddr*)&server, sizeof(server)) == -1)
    {
        printf("sendto: %s (%d)\n", strerror(errno), errno);
        return EXIT_FAILURE;
    }

    printf("Sent message <%s> from client (%x:%x:%x:%x:%x:%x) " \
            "to server %s:%d (%x:%x:%x:%x:%x:%x)\n", msg, src_mac[0],
            src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5],
            SERVER_ADDR, SERVER_PORT, dest_mac[0], dest_mac[1], dest_mac[2],
            dest_mac[3], dest_mac[4], dest_mac[5]);

    while (1)
    {
        /* Receive a packet from server*/
        endpoint_size = sizeof(server);
        recvfrom(server_fd, packet, PACKET_LEN, 0, 
                (struct sockaddr*)&server, &endpoint_size);

        /* Assign pointers to easily parse data */
        eth_header = (struct ether_header *)&packet;
        ip_header = (struct iphdr *)(packet+ETHHDR_SIZE);
        udp_header = (struct udphdr *)(packet+ETHHDR_SIZE+IPHDR_SIZE);
        msg = packet+ETHHDR_SIZE+IPHDR_SIZE+UDPHDR_SIZE;

        /*
        * If destination and source MAC matches client's and server's MACs or
        * destination port in packet is equal to defined client's port - print
        * message and end loop.
        */
        if ((memcmp(&eth_header->ether_dhost, src_mac, ETH_ALEN) == 0 && 
            memcmp(&eth_header->ether_shost, dest_mac, ETH_ALEN) == 0) || 
            ntohs(udp_header->dest) == CLIENT_PORT)
        {
            char addr[INET6_ADDRSTRLEN];
            if (inet_ntop(AF_INET, &ip_header->saddr, addr, INET6_ADDRSTRLEN) 
                == NULL)
            {
                perror("inet_ntop");
                return EXIT_FAILURE;
            }
            printf("Client (%x:%x:%x:%x:%x:%x) has received a message <%s> " \
                    "from server %s:%d (%x:%x:%x:%x:%x:%x)\n", 
                    eth_header->ether_dhost[0], eth_header->ether_dhost[1], 
                    eth_header->ether_dhost[2], eth_header->ether_dhost[3], 
                    eth_header->ether_dhost[4], eth_header->ether_dhost[5], 
                    msg, addr, ntohs(udp_header->source), 
                    eth_header->ether_shost[0], eth_header->ether_shost[1], 
                    eth_header->ether_shost[2], eth_header->ether_shost[3], 
                    eth_header->ether_shost[4], eth_header->ether_shost[5]);
            break;
        }
    }

	return EXIT_SUCCESS;
}
