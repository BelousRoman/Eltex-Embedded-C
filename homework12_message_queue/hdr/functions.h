/*
* Eltex's academy homework #12 for lecture 33 "Messages queue"
*
* The first_task() function is sending and receiving messages using System V
* message queues.
* The second_task() function is sending and receiving messages just as
* first_task(0 do, but using POSIX message queues.
* The third_task_server is a terminal window's server, creating POSIX mqs to
* receive message from one user and transmit it to another.
* The third_task_client is a ncurses app, operating as chatroom client.
* Press F3 to shut client app.
*/

#ifndef HOMEWORK12_H
#define HOMEWORK12_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <mqueue.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <curses.h>

/* File permissions and size of message in first_task() */
#define T1_PERMS                0666
#define T1_MSG_SIZE             80

/* File permissions, max messages and size of message in second_task() */
#define T2_PERMS                0666
#define T2_MAX_MSGS             2
#define T2_MSG_SIZE             255

/* Define-constants used in third_task_server() and third_task_client() */
#define T3_PERMS                0666
#define T3_LOGIN_Q_NAME         "/hw12_login"
/* Templates to create unique queues for every user */
#define T3_STOC_CH_NAME         "/hw12_client%d_stoc"
#define T3_CTOS_CH_NAME         "/hw12_client%d_ctos"
/*
* Number of users allocated by default, also serves as the value added to
* current number of allocated users.
*/
#define T3_DEF_ALLOC_USRS       10

/* Message priorities to treat them as commands */
#define T3_USER_PRIO            1
#define T3_SERVER_PRIO          2
#define T3_MSGSND_PRIO          3
#define T3_SET_USRNAME_PRIO     4
#define T3_UPD_USRLIST_PRIO     5
#define T3_REQ_USRLIST_PRIO     6
#define T3_DISCONNECT_PRIO      7

/* Attributes of login and user queues*/
#define T3_LOGIN_Q_MAX_MSGS     5
#define T3_LOGIN_Q_MSG_SIZE     80
#define T3_USER_Q_MAX_MSGS      5
#define T3_USER_Q_MSG_SIZE      1024

/* Structure used in first_task() */
struct msgbuf {
    long mtype;
    char mtext[T1_MSG_SIZE];
};

/* Structure for user used in third_task_server() and third_task_client() */
struct user_t {
    int id;
    mqd_t rcv_q;
    mqd_t snd_q;
    char login[T3_LOGIN_Q_MSG_SIZE];
    char stc_ch[T3_LOGIN_Q_MSG_SIZE];
    char cts_ch[T3_LOGIN_Q_MSG_SIZE];
    char rcv_buf[T3_USER_Q_MSG_SIZE];
    char snd_buf[T3_USER_Q_MSG_SIZE];
};

/*
* Structure to handle messages and update windows used in third_task_client().
*/
struct q_handler_t {
    mqd_t rcv_q;
    WINDOW *wnd;
    WINDOW *main_wnd;
    WINDOW *users_wnd;
    WINDOW *msg_wnd;
    char buf[T3_USER_Q_MSG_SIZE];
};

/**
 * @brief      System V message transfer
 * @return     Return 0
 */
int first_task();

/**
 * @brief      POSIX MQ message transfer
 * @return     Return 0
 */
int second_task();

/**
 * @brief      POSIX MQ's server for chatroom
 * @return     Return 0
 */
int third_task_server();

/**
 * @brief      POSIX MQ's client for chatroom
 * @return     Return 0
 */
int third_task_client();

#endif // HOMEWORK12_H
