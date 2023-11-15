/*
* Eltex's academy homework #9 for lecture 30 "IPC"
*
* The first_task() function is an algorithm, that send message from parent 
* process to child and then reseive an answer, using pipe() function and
* 2 channels.
*
* The second_task() function operates the same as first task, but it has been
* realized, using mkfifo() and 2 channels.
*
* The third_task() function is a command interpreter, same as in the
* homework6_processes, the difference here is this version of interpreter can
* now redirect STDOUT_FILENO from one process to another. The following text
* provides info about algorithm operation, same as in the homework6_processes.
* Is a simple command interpreter, that reads user input, then parses it with
* strtok and puts parsed argument into array, that is later passed to child
* process, calling execv() function of selected command.
* The amount of commands available to write without reallocating memory is set
* by DEF_CMD_LIM, the default number of argument in every command is set by
* T3_DEF_PARAMS_NUM, size of every argument is set by DEF_STR_SIZE, while
* T3_STDIN_READ_LIM controlls how many symbols will be read from user input.
* Known bugs:
* 1) When passing 2 or more arguments of command, the free for 0th argument of
*   that commands throws an error:
*        free(): invalid pointer
*        Aborted (core dumped)
* 2) When passing 4 or more commands the free of cmds pointer throws:
*        free(): invalid next size (fast)
*        Aborted (core dumped)
*/

#ifndef HOMEWORK6_H
#define HOMEWORK6_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <wait.h>
#include <fcntl.h>

#define T2_FIFO_FTOS_PATH "./kbuffer/fifo_ftos"
#define T2_FIFO_STOF_PATH "./kbuffer/fifo_stof"

#define T3_DEF_CMDS_LIM 1
#define T3_DEF_PARAMS_NUM 2
#define T3_DEF_STR_SIZE 20
#define T3_STDIN_READ_LIM 80

/**
 * @brief      Interprocess interaction using pipe()
 * @return     none
 */
int first_task();

/**
 * @brief      Interprocess interaction using mkfifo()
 * @return     none
 */
int second_task();

/**
 * @brief      Command interpreter
 * @return     none
 */
int third_task();

/**
 * @brief      Function called in first_proc binary
 * @return     none
 */
int first_process(void);

/**
 * @brief      Function called in second_proc binary
 * @return     none
 */
int second_process(void);

#endif // HOMEWORK6_H

