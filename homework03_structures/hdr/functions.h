/*
* Eltex's academy homework #3 for lecture 11 "Structures"
*
* The first_task function takes main function's argc and argv arguments in
* order to process getopt options, it then applies a 8 bit mask to a char
* value.
*/

#ifndef HOMEWORK3_H
#define HOMEWORK3_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define VALUE               0b11111111
#define BIT_MASK            0b10101010

/* Structure, where every element is a size of 1 bit */
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

/* Structure with union, to conveniently process 8 bit values */
struct char_val
{
    union
    {
        struct byte byte;
        unsigned char value;
    };
};

/**
 * @brief       Apply a mask to a char value
 * @param int argc - number of arguments.
 * @param char** argv - array of arguments.
 * @return      0 on success, 1 on errors
 */
int first_task(int, char **);

#endif // HOMEWORK3_H
