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
#include <mqueue.h>

#define MSG_SIZE                            10

#define SERVER_ADDR                         "127.0.0.1"
#define SERVER_TCP_PORT                     9875
#define SERVER_UDP_PORT                     9876
#define SERVER_LISTEN_BACKLOG               5
#define SERVER_DEF_ALLOC                    5
#define SERVER_Q_MAXMSG                     5
#define SERVER_TCP_Q_MSGSIZE                sizeof(int)
#define SERVER_UDP_Q_MSGSIZE                sizeof(struct sockaddr_in)
#define SERVER_MSG_SIZE                     MSG_SIZE
#define SERVER_MSG                          "Message"
#define SERVER_TCP_QUEUE_NAME               "/tcp_server_fds"
#define SERVER_UDP_QUEUE_NAME               "/udp_server_endpoints"
#define SERVER_TCP_FREE_THREADS_SEM_NAME    "/free_tcp_threads"
#define SERVER_UDP_FREE_THREADS_SEM_NAME    "/free_udp_threads"
#define SERVER_TCP_BUSY_THREADS_SEM_NAME    "/busy_tcp_threads"
#define SERVER_UDP_BUSY_THREADS_SEM_NAME    "/busy_udp_threads"
#define SERVER_SERVED_TCP_CLIENTS_SEM_NAME  "/served_tcp_clients"
#define SERVER_SERVED_UDP_CLIENTS_SEM_NAME  "/served_udp_clients"

#define CLIENT_DEF_ALLOC                    5
#define CLIENT_MSG_SIZE                     MSG_SIZE
#define CLIENT_MSG                          "Message"
#define CLIENT_TCP_COUNTER_SEM_NAME         "/client_tcp_counter"
#define CLIENT_UDP_COUNTER_SEM_NAME         "/client_udp_counter"

#define CLIENT_MODE                         TERMINATING

enum client_modes
{
    TERMINATING = 0,
    CONTINIOUS = 1
};

struct tcp_client_t {
    int client_fd;
    pthread_mutex_t client_mutex;
};

struct tcp_server_thread_t
{
    pthread_t tid;
    int client_fd;
    pthread_mutex_t mutex;
};

struct udp_server_thread_t
{
    pthread_t tid;
    struct sockaddr_in client_endp;
    pthread_mutex_t mutex;
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
