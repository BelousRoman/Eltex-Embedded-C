#ifndef HOMEWORK1_H
#define HOMEWORK1_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define DEF_CMDS_LIM 3

struct command_t {
	char cmd[16];
	unsigned char param1;
	unsigned char param2;
	unsigned char param3;
	unsigned char param4;
};

struct comm_list_t {
	int key;
	char cmd[16];
};

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

void free_on_exit(void);
void command_help();
void command_exit();

#endif // HOMEWORK1_H
