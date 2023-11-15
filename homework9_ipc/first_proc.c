#include "hdr/functions.h"

int main(void)
{
    int ret = mkfifo(T2_FIFO_FTOS_PATH, 0666);
	if (ret == -1 && errno != EEXIST)
	{
		perror("mkfifo \"first to second\"");
	}

	ret = mkfifo(T2_FIFO_STOF_PATH, 0666);
	if (ret == -1 && errno != EEXIST)
	{
		perror("mkfifo \"second to first\"");
	}

	first_process();

	exit(EXIT_SUCCESS);
}