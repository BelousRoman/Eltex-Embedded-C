/*
* Eltex's academy homework #16 for lecture 39 "Client Service Schemes"
*/

#ifndef HOMEWORK16_H
#define HOMEWORK16_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <malloc.h>
#include <sys/poll.h>

#define MSG_SIZE                            256

#define SERVER_ADDR                         "127.0.0.1"
#define SERVER_TCP_PORT                     9875
#define SERVER_UDP_PORT                     9876
#define SERVER_LISTEN_BACKLOG               100
#define SERVER_DEF_ALLOC                    1000
#define SERVER_MSG                          "Server_msg"
#define SERVER_TCP_COUNTER_SEM_NAME         "/server_tcp_counter"
#define SERVER_UDP_COUNTER_SEM_NAME         "/server_udp_counter"
#define SERVER_TCP_BUSY_THREADS_SEM_NAME    "/busy_tcp_threads"
#define SERVER_UDP_BUSY_THREADS_SEM_NAME    "/busy_udp_threads"
#define SERVER_SERVED_CLIENTS_SEM_NAME      "/served_clients"

#define CLIENT_DEF_ALLOC                    100
#define CLIENT_MSG                          "Client_msg"
#define CLIENT_COUNTER_SEM_NAME             "/client_counter"

struct tcp_client_t {
    int client_fd;
    pthread_mutex_t client_mutex;
};

struct udp_client_t {
    struct sockaddr_in client;
    pthread_mutex_t client_mutex;
};

/**
 * @brief       Server, using serial scheme, processing one client at the time
 * @return      0
 */
int serial_server(void);

/**
 * @brief       Server, using classic scheme, sending client fd to new thread
 * @return      0
 */
int classic_server(void);

/**
 * @brief       Client, creating new threads, connecting to server until one of
 *              them is refused to connect the server
 * @return      0
 */
int client(void);

/**
 * @brief       Server, using multiprotocol (TCP+UDP) scheme, sending client fd
 *              to a new thread.
 * @return      0
 */
int multiproto_server(void);

/**
 * @brief       Multiprotocol (TCP+UDP) client, creating new threads, 
 *              connecting to server until one of them is refused to connect
 *              to the server.
 * @return      0
 */
int multiproto_client(void);

#endif // HOMEWORK16_H
