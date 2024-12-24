#include "stdio.h"
#include "unistd.h"

int main(void)
{
	puts("*** Init process started!\nPut to sleep for 10 seconds");
	sleep(10);
	puts("*** Init process ending, kernel panic expected");
	return 0;
}
