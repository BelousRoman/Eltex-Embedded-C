/*
* Eltex's academy homework #17 for lecture 42 "Raw Socket: Transport Layer"
*
* The server() function is an UDP socket server, receiving a message from
* client, then sending back answer.
*
* The client() function is RAW socket client, forming a packet out of UDP
* transport layer header and message and then sending it to a server. After
* that, client locks in a loop, reading incoming packets and parsing
* destination port in packet's header until verifying, that client is this
* message recipient.
*/

#ifndef HOMEWORK17_H
#define HOMEWORK17_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>

#define MSG_SIZE                            10
#define CLIENT_MSG                          "Client"
#define SERVER_MSG                          "Server"

#define SEND_PACKET_LEN                     (sizeof(struct udphdr)+MSG_SIZE)
#define RECV_PACKET_LEN                     (sizeof(struct iphdr)+SEND_PACKET_LEN)

#define SERVER_ADDR                         "127.0.0.1"
#define SERVER_PORT                         6789
#define CLIENT_PORT                         9876

/**
 * @brief       UDP Server, receiving message and replying to client.
 * @return      0 on success, 1 on errors
 */
int server(void);

/**
 * @brief       Client, creating raw socket, filling transport layer header, 
 *              sending packet to UDP server and receiving an answer from 
 *              server.
 * @return      0 on success, 1 on errors
 */
int client(void);

#endif // HOMEWORK17_H
