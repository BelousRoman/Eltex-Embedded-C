#ifndef HOMEWORK1_H
#define HOMEWORK1_H

#include <math.h>
#include <stdio.h>

#define FIRST_TASK_N 3

#define SECOND_TASK_ARR {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}
#define SECOND_TASK_N 10

#define THIRD_TASK_N 5

#define FOURTH_TASK_N 6

/**
 * @brief      Draws NxN matrix
 * @return     none
 */
void first_task();

/**
 * @brief      Inverts an array
 * @return     none
 */
void second_task();

/**
 * @brief      Draws square matrix where elements under left-to-right diagonal
 *             are filled with 1's, the rest - with 0's
 * @return     none
 */
void third_task_ltr();

/**
 * @brief      Draws square matrix where elements under right-to-left diagonal
 *             are filled with 1's, the rest - with 0's
 * @return     none
 */
void third_task_rtl();

/**
 * @brief      Draws snail-like square matrix
 * @return     none
 */
void fourth_task();

#endif // HOMEWORK1_H