/*
* Eltex's academy homework #5 for lecture 18 "Dynamic memory"
*
* The first_task function reads stdin filestream and fills dynamic array of
* dictionary with entries, each containing name, surname and phone number of a
* person.
*/

#ifndef HOMEWORK5_H
#define HOMEWORK5_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <signal.h>
#include <sys/select.h>
#include <errno.h>

#define NAME_LEN        20
#define SURNAME_LEN     20
#define PHONE_LEN       12
#define STDIN_STR_LEN   80

/* Structure, containing persons info */
struct dictionary
{
    char name[NAME_LEN+1];
    char surname[SURNAME_LEN+1];
    char phone[PHONE_LEN+1];
};

/**
 * @brief       Read stdin filestream and fill dynamic array of dictionary
 * @return      0 on success, 1 on errors
 */
int first_task();

#endif // HOMEWORK5_H
