/*
* Eltex's academy homework #6 for lecture 22-24 "Processes"
*
* The first_task function is a algorithm, that starts a total of 6 processes.
* In the following sequence:
* |----Grandfather (#1)
* |
* |---Parent (#2)
* |   |--Child (#4)
* |   |--Child (#5)
* |
* |---Parent (#3)
*     |--Child (#6)
*
* The second_task function is a simple command interpreter, that reads user 
* input, then parses it with strtok and puts parsed argument into array, that
* is later passed to child process, calling execv() function of selected 
* command.
* The amount of commands available to write without reallocating memory is set
* by DEF_CMD_LIM, the default number of argument in every command is set by
* DEF_PARAMS_NUM, size of every argument is set by DEF_STR_SIZE, while
* STDIN_READ_LIM controlls how many symbols will be read from user input.
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
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>

#define DEF_CMDS_LIM 1
#define DEF_PARAMS_NUM 2
#define DEF_STR_SIZE 20
#define STDIN_READ_LIM 80

/**
 * @brief      Creates 6 processes from 1 process
 * @return     none
 */
int first_task();

/**
 * @brief      Command interpreter
 * @return     none
 */
int second_task();

#endif // HOMEWORK6_H
