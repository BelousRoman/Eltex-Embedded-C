/*
* Eltex's academy homework #3 for lecture 11 "Structures"
*
* The server() function.
*/

#ifndef HOMEWORK3_H
#define HOMEWORK3_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define VALUE               0b11111111
#define BIT_MASK            0b10101010

struct byte
{
    unsigned char b1:1;
    unsigned char b2:1;
    unsigned char b3:1;
    unsigned char b4:1;
    unsigned char b5:1;
    unsigned char b6:1;
    unsigned char b7:1;
    unsigned char b8:1;
};

struct mask
{
    union
    {
        struct byte byte;
        unsigned char value;
    };
};

/**
 * @brief       0
 * @return      0 on success, 1 on errors
 */
int first_task(int, char **);

#endif // HOMEWORK3_H
