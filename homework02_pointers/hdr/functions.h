/*
* Eltex's academy homework #2 for lecture 10 "Pointers"
*
* The first_task function takes main function's argc and argv arguments in
* order to process getopt options, it then changes one specific byte in an int
* type value.
*/

#ifndef HOMEWORK2_H
#define HOMEWORK2_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define VALUE               0xAABBCCDD
#define BYTE_TO_CHANGE      2
#define NEW_VALUE           0xEE

/**
 * @brief       Change value in int variable's one specific byte
 * @param int argc - number of arguments.
 * @param char** argv - array of arguments.
 * @return      0 on success, 1 on errors
 */
int first_task(int, char **);

#endif // HOMEWORK2_H
