/*
* Eltex's academy homework #2 for lecture 10 "Pointers"
*
* The server() function.
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
 * @brief       0
 * @return      0 on success, 1 on errors
 */
int first_task(int, char **);

#endif // HOMEWORK2_H
