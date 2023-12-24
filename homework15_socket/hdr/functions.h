/*
* Eltex's academy homework #15 for lecture 36 "Sockets"
*
* The first_task() function setups local connection for client and server,
* using TCP-protocol.
* The second_task() function setups local connection for client and server,
* using UDP-protocol.
* The third_task() function setups internet connection for client and server,
* using TCP-protocol.
* The fourth_task() function setups internet connection for client and server,
* using UDP-protocol.
*/

#ifndef HOMEWORK15_H
#define HOMEWORK15_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

#define T1_SUN_PATH                 "/tmp/hw15_first_task"
#define T1_LISTEN_BACKLOG           5
#define T1_CONN_RETRIES             5
#define T1_RETRY_DELAY_SEC          0
#define T1_RETRY_DELAY_NSEC         50000000
#define T1_MSG_SIZE                 256
#define T1_SERVER_MSG               "Hello"
#define T1_CLIENT_MSG               "Hi"

#define T2_SERVER_SUN_PATH          "/tmp/hw15_second_task_server"
#define T2_CLIENT_SUN_PATH          "/tmp/hw15_second_task_client"
#define T2_CONN_RETRIES             5
#define T2_RETRY_DELAY_SEC          0
#define T2_RETRY_DELAY_NSEC         50000000
#define T2_MSG_SIZE                 256
#define T2_SERVER_MSG               "Hello"
#define T2_CLIENT_MSG               "Hi"

#define T3_PROTO_FAMILY             AF_INET
#define T3_SERV_ADDR                "127.0.0.1"
#define T3_SERV_PORT                9871
#define T3_LISTEN_BACKLOG           5
#define T3_CONN_RETRIES             5
#define T3_RETRY_DELAY_SEC          0
#define T3_RETRY_DELAY_NSEC         50000000
#define T3_MSG_SIZE                 256
#define T3_SERVER_MSG               "Hello"
#define T3_CLIENT_MSG               "Hi"

#define T4_PROTO_FAMILY             AF_INET
#define T4_SERV_ADDR                "127.0.0.1"
#define T4_SERV_PORT                9872
#define T4_CONN_RETRIES             5
#define T4_RETRY_DELAY_SEC          0
#define T4_RETRY_DELAY_NSEC         50000000
#define T4_MSG_SIZE                 256
#define T4_SERVER_MSG               "Hello"
#define T4_CLIENT_MSG               "Hi"

/**
 * @brief      AF_LOCAL socket communication with TCP-protocol
 * @return     0
 */
int first_task(void);

/**
 * @brief      AF_LOCAL socket communication with UDP-protocol
 * @return     0
 */
int second_task(void);

/**
 * @brief      AF_INET socket communication with TCP-protocol
 * @return     Return 0
 */
int third_task(void);

/**
 * @brief      AF_INET socket communication with UDP-protocol
 * @return     0
 */
int fourth_task(void);

#endif // HOMEWORK15_H
