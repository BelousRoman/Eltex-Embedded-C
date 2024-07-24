#include "../hdr/functions.h"

#ifdef MUTEX
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

void * _counter(void * args)
{
	/*
	* Converts void * pointer to size_t type, 
	* sets iter_num a number of loop iterations to run and
	* declares index variable
	*/
	size_t * global_var = (size_t *)args;
	size_t iter_num = TARGET_VAL / THREAD_NUM;
	int index;

	/*
	* Simply locks mutex if MUTEX define-constant is defined in header file
	* and increments global variable
	*/
	for (index = 0; index < iter_num; index++)
	{
		#ifdef MUTEX
		pthread_mutex_lock(&mutex);
		#endif

		*global_var = *global_var + 1;

		#ifdef MUTEX
		pthread_mutex_unlock(&mutex);
		#endif
	}
	
	return 0;
}

int first_task(){
	puts("First task");

	/* 
	* Create THREAD_NUM array of pids, index variable 
	* and global_var to pass it to each thread and then increment
	*/
	pthread_t pid[THREAD_NUM];
	int index;
	size_t global_var = 0;

	/* Create THREAD_NUM amount of threads */
	for (index = 0; index < THREAD_NUM; ++index)
	{
		if (pthread_create(&pid[index], NULL, _counter, (void *)&global_var) != 0)
		{
			perror("Couldn't create thread\n");
			exit(EXIT_FAILURE);
		}
	}

	/* Wait for these threads to finish */
	for (index = 0; index < THREAD_NUM; ++index)
	{
		pthread_join(pid[index], NULL);
	}

	printf("Global_var value = %ld\n", global_var);
	
	return EXIT_SUCCESS;
}