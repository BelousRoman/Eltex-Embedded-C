/*
* Eltex's academy homework #14 for lecture 35 "Signals"
*
* The first_task() function changes signal action, then locks in 'while' loop,
* waiting for SIGUSR1 signal to arrive.
* The second_task() function has the same concept, but it instead blocks
* SIGUSR1 and then locks in sigwait(), until SIGUSR1 arrives.
*/

#ifndef HOMEWORK14_H
#define HOMEWORK14_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

/*
* Time that parent process sleeps, before sending signal to child process
* (in seconds).
*/
#define T1_SLEEP_TIME 5
#define T2_SLEEP_TIME 5

/**
 * @brief      Signal handling
 * @return     Return 0
 */
int first_task();

/**
 * @brief      Signal handling
 * @return     Return 0
 */
int second_task();

#endif // HOMEWORK14_H
