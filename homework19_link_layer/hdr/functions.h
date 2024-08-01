/*
* Eltex's academy homework #18 for lecture 43 "Raw Socket: Network Layer"
*
* The server() function is an UDP socket server, receiving a message from
* client, then sending back answer.
*
* The client() function is RAW socket client, forming a packet out of IP
* network layer, UDP transport layer headers and message and then sending it to
* a server. After that, client locks in a loop, reading incoming packets and
* parsing destination port in packet's header until verifying, that client is
* this message recipient.
*/

#ifndef HOMEWORK18_H
#define HOMEWORK18_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <linux/if_ether.h>
#include <string.h>
#include <errno.h>

#define ETHHDR_SIZE                         sizeof(struct ether_header)
#define IPHDR_SIZE                          sizeof(struct iphdr)
#define UDPHDR_SIZE                         sizeof(struct udphdr)
#define MSG_SIZE                            10

#define PACKET_LEN                          (ETHHDR_SIZE+IPHDR_SIZE+UDPHDR_SIZE+MSG_SIZE)

#define ETHHDR_DEST_MAC                     "08:00:27:72:e7:cf"
#define ETHHDR_IF_NAME                      "enp0s3"
#define ETHHDR_LOCAL_IF_NAME                "lo"
#define IF_PATH_TEMPLATE                    "/sys/class/net/%s/address"
#define IF_PATH_TEMPLATE_SIZE               (strlen(IF_PATH_TEMPLATE))
#define IPHDR_IP_VERSION                    0b0100
#define IPHDR_IHL                           0b0101
#define IPHDR_TOS                           0x00
#define IPHDR_ID                            0x7F00
#define IPHDR_DONT_FRAGMENT_FLAG            (1 << 14)
#define IPHDR_MORE_FRAGMENTS_FLAG           (1 << 13)
#define IPHDR_TTL                           64

#define CLIENT_MSG                          "Client"
#define SERVER_MSG                          "Server"

#define LOCAL_ADDR                          "127.0.0.1"
#define SERVER_ADDR                         "10.0.2.15"
#define SERVER_PORT                         6789
#define CLIENT_ADDR                         "10.0.2.15"
#define CLIENT_PORT                         9876

/**
 * @brief       UDP Server, receiving message and replying to client.
 * @return      0 on success, 1 on errors
 */
int server(void);

/**
 * @brief       Client, creating raw socket, filling link, network and
 *              transport layer headers, sending packet to UDP server and
 *              receiving an answer from server.
 * @return      0 on success, 1 on errors
 */
int client(void);

#endif // HOMEWORK18_H
