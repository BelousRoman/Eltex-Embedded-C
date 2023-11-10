#include "../hdr/functions.h"

struct store_t stores[STORES_NUM];

void * _loader_thread(void * args)
{
	/* get id of loader from args, declare index */
	int id = (int *)args;
	int index;
	
	/*
	* the loop runs infinitely, until it's canceled in the main thread, which
	* is happening when every customer has fullfilled it's need.
	* Loader is continuously trying to lock the mutex for particular store and
	* then increment it's stock by LOADER_INCREMENT.
	* Then unlock the mutex and go to sleep for LOADER_SLEEP_TIME seconds.
	*/
	while(1)
	{
		for (index = 0; index < STORES_NUM; ++index)
		{
			if (pthread_mutex_trylock(&stores[index].mutex) == 0)
			{
				stores[index].store += LOADER_INCREMENT;
				printf("Loader #%d has loaded %d into store #%d(has %d)\n",
						id, LOADER_INCREMENT, index, stores[index].store);
				pthread_mutex_unlock(&stores[index].mutex);
				sleep(LOADER_SLEEP_TIME);
			}
			
		}
	}

	return EXIT_SUCCESS;
}

void * _buyer_thread(void *args)
{
	/* 
	* get id of loader from args,
	* set customer's need to define-constant,
	* declare index
	*/
	int id = (int *)args;
	int need = CUSTOMER_NEED;
	int index = 0;

	/*
	* the loop runs until customer's need is fullfilled, continuously trying
	* to lock the mutex for particular store and then check if it has anything
	* in stock to decrement need.
	* Then unlock the mutex and go to sleep for CUSTOMER_SLEEP_TIME seconds.
	*/
	while (0 < need)
	{
		for (index = 0; index < STORES_NUM; ++index)
		{
			if (pthread_mutex_lock(&stores[index].mutex) == 0)
			{
				if (stores[index].store == 0)
				{
					pthread_mutex_unlock(&stores[index].mutex);
					continue;
				}

				if (need > stores[index].store)
				{
					need -= stores[index].store;
					stores[index].store = 0;
				}
				else
				{
					stores[index].store -= need;
					need = 0;
				}
				printf("Customer #%d has bought everything from "
						"store #%d(has %d), need %d more\n", 
						id, index, stores[index].store, need);
				pthread_mutex_unlock(&stores[index].mutex);
				sleep(CUSTOMER_SLEEP_TIME);
			}
		}
	}
	
	printf("Customer #%d has fulfilled his need\n", id);
	
	return EXIT_SUCCESS;
}

int first_task(){
	puts("First task");

	/* Declare pids for loader's and customer's threads and index variable */
	pthread_t loaders_pid[LOADER_THREAD_NUM];
	pthread_t customers_pid[CUSTOMER_THREAD_NUM];
	int index;
	
	/* Init global array of store's stocks and mutexes */
	for (index = 0; index < STORES_NUM; ++index)
	{
		stores[index].store = DEF_STORE_CAP;
		pthread_mutex_init(&stores[index].mutex, NULL);
	}

	/* Create threads for customers and loaders */
	for (index = 0; index < LOADER_THREAD_NUM; ++index)
	{
		if (pthread_create(&loaders_pid[index], NULL, _loader_thread,
							(void *)(index+1)) != 0)
		{
			perror("Couldn't create loader thread\n");
			exit(EXIT_FAILURE);
		}
	}
	for (index = 0; index < CUSTOMER_THREAD_NUM; ++index)
	{
		if (pthread_create(&customers_pid[index], NULL, _buyer_thread,
							(void *)(index+1)) != 0)
		{
			perror("Couldn't create customer thread\n");
			exit(EXIT_FAILURE);
		}
	}

	/* Wait for customer's threads to finish, then cancel loader's threads */
	for (index = 0; index < CUSTOMER_THREAD_NUM; ++index)
	{
		pthread_join(customers_pid[index], NULL);
	}

	puts("All the customers are done with shopping, "
			"canceling loader's threads");
	for (index = 0; index < LOADER_THREAD_NUM; ++index)
	{
		pthread_cancel(loaders_pid[index]);
	}
	
	return EXIT_SUCCESS;
}