#ifndef HOMEWORK6_H
#define HOMEWORK6_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

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
