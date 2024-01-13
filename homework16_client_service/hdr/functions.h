/*
* Eltex's academy homework #16 for lecture 39 "Client Service Schemes"
*/

#ifndef HOMEWORK16_H
#define HOMEWORK16_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// #include <time.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

#define MSG_SIZE                        256

#define SERVER_ADDR                     "127.0.0.1"
#define SERVER_TCP_PORT                 9875
#define SERVER_UDP_PORT                 9876
#define SERVER_LISTEN_BACKLOG           100
#define SERVER_DEF_ALLOC                10
#define SERVER_MSG                      "Server_msg"

#define CLIENT_DEF_ALLOC                100
#define CLIENT_MSG                      "Client_msg"

/**
 * @brief       Server, using serial scheme, processing one client at the time
 * @return      0
 */
int first_task(void);

/**
 * @brief       Server, using classic scheme, sending client fd to new thread
 * @return      0
 */
int second_task(void);

/**
 * @brief       Client, creating new threads, connecting to server until one of
 *              them is refused to connect the server
 * @return      0
 */
int client(void);

#endif // HOMEWORK16_H
