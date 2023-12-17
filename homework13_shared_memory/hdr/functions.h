/*
* Eltex's academy homework #13 for lecture 34 "Shared memory, semaphores"
*
* The first_task() function is sending and receiving messages using System V
* shared memory and semaphores.
* The second_task() function is sending and receiving messages just as
* first_task(0 do, but using POSIX shared memory and semaphores.
* The third_task_server is a terminal window's server, using POSIX shared
* memory and semaphores to receive message from one user and transmit it to
* another.
* The third_task_client is a ncurses application, operating as chatroom client.
* Press F3 to shut client application.
*/

#ifndef HOMEWORK13_H
#define HOMEWORK13_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <curses.h>

/* File permissions, keys, msg size (in bytes) used in first_task() */
#define T1_PERMS                    0666
#define T1_SHM_KEY                  1234
#define T1_SEM_KEY                  5678
#define T1_MSG_SIZE                 256

/* File permissions, name, msg size (in bytes) used in second_task() */
#define T2_PERMS                    0666
#define T2_SHM_NAME                 "/hw13_shm"
#define T2_PTOC_SEM_NAME            "/hw13_sem_ptoc"
#define T2_CTOP_SEM_NAME            "/hw13_sem_ctop"
#define T2_MSG_SIZE                 256

/* Define-constants used in third_task_server() and third_task_client() */
#define T3_PERMS                    0666
#define T3_LOGIN_SHM_NAME           "/hw13_login_shm"
#define T3_LOGIN_STOC_SEM_NAME      "/hw13_login_stoc_sem"
#define T3_LOGIN_CTOS_SEM_NAME      "/hw13_login_ctos_sem"
#define T3_LOGIN_LOG_SRV_SEM_NAME   "/hw13_login_log_srv_sem"
#define T3_LOGIN_LOG_USR_SEM_NAME   "/hw13_login_log_usr_sem"
/* Templates to create unique shm and sems for every user */
#define T3_USER_SHM_NAME          "/hw13_client%d_shm"
#define T3_USER_STOC_SEM_NAME     "/hw13_client%d_stoc_sem"
#define T3_USER_CTOS_SEM_NAME     "/hw13_client%d_ctos_sem"
#define T3_USER_SRV_SEM_NAME      "/hw13_client%d_srv_sem"
#define T3_USER_USR_SEM_NAME      "/hw13_client%d_usr_sem"
/*
* Number of users allocated by default, also serves as the value added to
* current number of allocated users.
*/
#define T3_DEF_ALLOC_USRS           10
/* Server timeout, when waiting for user to unlock semaphore */
#define T3_SERVER_TIMEOUT_SEC        1
/* Message commands */
#define T3_MSGSND_COMM              1
#define T3_SET_USRNAME_COMM         2
#define T3_UPD_USRLIST_COMM         3
#define T3_REQ_USRLIST_COMM         4
#define T3_DISCONNECT_COMM          5
/* Size of messages */
#define T3_LOGIN_MSG_SIZE           80
#define T3_USER_MSG_SIZE          1024

/*
* Shared memory segment, separated into server-to-client and client-to-server
* messages.
*/
struct log_msg_t {
    char stoc[T3_LOGIN_MSG_SIZE];
    char ctos[T3_LOGIN_MSG_SIZE];
};

/* Structure, containing shm segment and semaphores, used in login sequnce */
struct login_t {
    int shm_fd;
    sem_t *stc_sem;
    sem_t *cts_sem;
    sem_t *srv_sem;
    sem_t *usr_sem;
    struct log_msg_t *shm_buf;
};

/*
* Message structure, used in usr_shm_t struct, contains command id and message
* itself.
*/
struct usr_msg_t {
    unsigned char comm;
    char msg[T3_USER_MSG_SIZE];
};

/*
* User's shared memory segment, separated into server-to-client and
* client-to-server messages.
*/
struct usr_shm_t {
    struct usr_msg_t stoc;
    struct usr_msg_t ctos;
};

/* Structure for user used in third_task_server() and third_task_client() */
struct user_t {
    int id;
    int usr_shm;
	sem_t *stc_sem;
	sem_t *cts_sem;
    sem_t *srv_sem;
    sem_t *usr_sem;
    char login[T3_LOGIN_MSG_SIZE];
    char shm_name[T3_LOGIN_MSG_SIZE];
    char stc_sem_name[T3_LOGIN_MSG_SIZE];
    char cts_sem_name[T3_LOGIN_MSG_SIZE];
    char srv_sem_name[T3_LOGIN_MSG_SIZE];
    char usr_sem_name[T3_LOGIN_MSG_SIZE];
    struct usr_shm_t *shm_buf;
};

/*
* Structure to contained all the windows used in third_task_client().
*/
struct windows_t {
    WINDOW *wnd;
    WINDOW *main_wnd;
    WINDOW *users_wnd;
    WINDOW *msg_wnd;
};

/**
 * @brief      System V shm & semaphore message transfer
 * @return     Return 0
 */
int first_task();

/**
 * @brief      POSIX shm & semaphore message transfer
 * @return     Return 0
 */
int second_task();

/**
 * @brief      POSIX shm's & semaphore's server for chatroom
 * @return     Return 0
 */
int third_task_server();

/**
 * @brief      POSIX shm's & semaphore's client for chatroom
 * @return     Return 0
 */
int third_task_client();

#endif // HOMEWORK13_H
