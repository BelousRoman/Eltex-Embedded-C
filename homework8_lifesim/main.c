#include "hdr/functions.h"
// void func(int **var)
// {
// 	for(int i = 0; i < 4; i++)
// 	{
// 		printf("%d\n", var[i]);
// 		*var[i] += 1;
// 		printf("%d\n", var[i]);
// 	}
// }

pthread_mutex_t mmutex = PTHREAD_MUTEX_INITIALIZER;

int main(void){
	pthread_mutex_trylock(&mmutex);
	pthread_mutex_unlock(&mmutex);
	first_task();
	// int a=1,b=2,c=3,d=4;
	// printf("%d %d %d %d\n", a, b, c, d);
	// int *arr[4] = {&a, &b, &c, &d};
	// func(arr);
	// printf("%d %d %d %d\n", a, b, c, d);

	return 0;
}
