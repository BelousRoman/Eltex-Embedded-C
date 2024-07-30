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
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

#define IPHDR_SIZE                          sizeof(struct iphdr)
#define UDPHDR_SIZE                         sizeof(struct udphdr)
#define MSG_SIZE                            10
#define CLIENT_MSG                          "Client"
#define SERVER_MSG                          "Server"

#define PACKET_LEN                          (IPHDR_SIZE+UDPHDR_SIZE+MSG_SIZE)

#define SERVER_ADDR                         "127.0.0.1"
#define SERVER_PORT                         6789
#define CLIENT_PORT                         9876

#define IPHDR_DONT_FRAGMENT_FLAG(x)         x << 1
#define IPHDR_MORE_FRAGMENTS_FLAG(x)        x << 2
#define IPHDR_TTL                           64

/**
 * @brief       UDP Server, receiving message and replying to client.
 * @return      0 on success, 1 on errors
 */
int server(void);

/**
 * @brief       Client, creating raw socket, filling network and transport
 *              layer headers, sending packet to UDP server and receiving an
 *              answer from server.
 * @return      0 on success, 1 on errors
 */
int client(void);

#endif // HOMEWORK18_H
