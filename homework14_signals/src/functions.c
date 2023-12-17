#include "../hdr/functions.h"

/* Signal handler, assigned to SIGUSR1 in first_task() */
static void signal_handler(int sig)
{
	puts("Received SIGUSR1");

    exit(EXIT_SUCCESS);
}

int first_task()
{
	puts("First task");

	/*
	* Declare:
	* - pid - process id of child process.
	*/
	pid_t pid;

	/* Create child process */
	pid = fork();
	if (pid == 0)
	{
		/* Define sigaction, set signal handler */
		struct sigaction sa;
		sigfillset(&sa.sa_mask);
		sa.sa_flags = SA_RESTART;
		sa.sa_handler = signal_handler;
		if (sigaction(SIGUSR1, &sa, NULL) == -1)
		{
			perror("sigaction");
			exit(EXIT_FAILURE);
		}

		/*
		* Endless loop to ensure that this process is killed only when signal
		* arrives.
		*/
		while(1)
		{}

		exit(EXIT_SUCCESS);
	}
	else
	{
		printf("Put parent process to sleep for %d seconds\n", T1_SLEEP_TIME);

		/* Sleep for T1_SLEEP_TIME seconds */
		sleep(T1_SLEEP_TIME);

		/* Send signal to child process */
		kill(pid, SIGUSR1);

		wait(NULL);
	}

	return EXIT_SUCCESS;
}

int second_task()
{
	puts("Second task");

	/*
	* Declare:
	* - pid - process id of child process.
	*/
	pid_t pid;

	/* Create child process */
	pid = fork();
	if (pid == 0)
	{
		/*
		* Declare:
		* - sigset - signal mask;
		* - sign - signal, arrived to sigwait().
		*/
		sigset_t sigset;
		int sign;

		/* Set empty signal mask 'sigset', add only SIGUSR1 */
		sigemptyset(&sigset);
		sigaddset(&sigset, SIGUSR1);

		/* Block signals in 'sigset' mask */
		sigprocmask(SIG_BLOCK, &sigset, NULL);

		/* Wait for any signal in 'sigset' mask */
		sigwait(&sigset, &sign);

		printf("Child received signal #%d\n", sign);

		exit(EXIT_SUCCESS);
	}
	else
	{
		printf("Put parent process to sleep for %d seconds\n", T2_SLEEP_TIME);

		/* Sleep for T2_SLEEP_TIME seconds */
		sleep(T2_SLEEP_TIME);

		/* Send signal to child process */
		kill(pid, SIGUSR1);

		wait(NULL);
	}

	return EXIT_SUCCESS;
}
