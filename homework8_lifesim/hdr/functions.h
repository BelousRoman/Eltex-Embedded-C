/*
* Eltex's academy homework #8 for lecture 28 "Reentrability"
*
* The first_task function is a life simulator which creates
* a total of LOADER_THREAD_NUM + CUSTOMER_THREAD_NUM threads, where:
* Loader is a thread, which adds LOADER_INCREMENT units to store,
* which is not locked by mutex by other thread, then go to sleep for 
* LOADER_SLEEP_TIME seconds;
* Customer is a thread, which decrementing units from not mutex-locked store,
* it's goal is to fullfill 'customer's need' set by CUSTOMER_NEED
* define-constant, then go to sleep for CUSTOMER_SLEEP_TIME seconds.
*
* Total number of stores is set in STORES_NUM define-constant, each contains
* int, storing current capacity of store, which is set to DEF_STORE_CAP
* right after it's definition, and pthread_mutex_t for locking the store,
* to prevent access from more than 1 thread simeltaneously.
*/

#ifndef HOMEWORK8_H
#define HOMEWORK8_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define LOADER_THREAD_NUM 1
#define LOADER_INCREMENT 500
#define LOADER_SLEEP_TIME 2

#define CUSTOMER_THREAD_NUM 3
#define CUSTOMER_NEED 10000
#define CUSTOMER_SLEEP_TIME 1

#define STORES_NUM 5
#define DEF_STORE_CAP 1000

struct store_t {
    int store;
    pthread_mutex_t mutex;
};

/**
 * @brief      Multithread global variable counter
 * @return     Returns 0 on success, otherwise 1 is returned
 */
int first_task();

#endif // HOMEWORK1_H
