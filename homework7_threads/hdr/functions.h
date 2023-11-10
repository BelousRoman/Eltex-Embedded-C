/*
* Eltex's academy homework #7 for lecture 26 "Threads"
*
* The first_task function is a multithread algorithm that increments a varible,
* which is shared between threads by 1 each iteration until it (supposedly)
* reaches the value of TARGET_VAL define-constant as supposed.
*
* Although, the value returned is non-determined and less that constant, to
* guarantee global variable to be equal TARGET_VAL add:
* #define MUTEX
*/

#ifndef HOMEWORK7_H
#define HOMEWORK7_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define THREAD_NUM 10
#define TARGET_VAL 100000000

/**
 * @brief      Multithread global variable counter
 * @return     Returns 0 on success, otherwise 1 is returned
 */
int first_task();

#endif // HOMEWORK7_H
