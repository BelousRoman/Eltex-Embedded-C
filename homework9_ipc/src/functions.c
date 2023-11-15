#include "../hdr/functions.h"

char ***cmds = NULL;
int cmds_lim = T3_DEF_CMDS_LIM;
int *cmds_params = NULL;

/* Function called on exit() from third_task() */
void _cleanup(void)
{
	int index;
	int sec_index;

	if (cmds != NULL)
	{
		/* Free allocated memory */
		for (index = 0; index < cmds_lim; ++index)
		{
			for (sec_index = cmds_params[index]-1; sec_index >= 0; --sec_index)
			{
				/* First free every argument */
				free(cmds[index][sec_index]);
			}
			/* Then argument's array */
			free(cmds[index]);
		}
		free(cmds);
		free(cmds_params);
	}
}

int first_task(){
	puts("First task");

	/*
	* Declare:
	* - pipe_ptoc & pipe_ctop - file descriptor for 2 channels, pipe_ptoc for
	* sending from parent to child and pipe_ctop for sending from child to
	* parent;
	* - pid - process id of child process, used with fork();
	* - buf - char buffer to read channels, it's later copied to each process
	* with COW;
	* ret - variable to content return values from functions, usable in
	* comparing result.
	*/
	int pipe_ptoc[2];
	int pipe_ctop[2];
	pid_t pid;
	char buf;
	int ret;

	/* Create "Parent To Child" channel */
	ret = pipe(pipe_ptoc);
	if (ret == -1)
	{
		perror("pipe \"parent to child\"");
		exit(EXIT_FAILURE);
	}

	/* Create "Child To Parent" channel */
	ret = pipe(pipe_ctop);
	if (ret == -1)
	{
		perror("pipe \"child to parent\"");
		exit(EXIT_FAILURE);
	}

	/* Fork child process, process errors */
	pid = fork();
	if (pid == -1)
	{
		perror("fork");
		exit(EXIT_FAILURE);
	}

	/*
	* In child process first read the pipe_ptoc channel, then write to
	* pipe_ctop.
	*/
	if (pid == 0)
	{
		close(pipe_ptoc[1]); /* Close unused write-end of pipe_ptoc channel */
		close(pipe_ctop[0]); /* Close unused read-end of pipe_ctop channel */
		/*
		* Read the read-end of pipe_ptoc channel until EOL is detected, write
		* result to the terminal.
		*/
		while (read(pipe_ptoc[0], &buf, 1) > 0)
		{
			write(STDOUT_FILENO, &buf, 1);
		}
		write(STDOUT_FILENO, "\n", 1);

		/* Write message to the write-end of pipe_ctop */
		write(pipe_ctop[1], "Hello!", 7);

		close(pipe_ptoc[0]); /* Close read-end of pipe_ptoc channel */
		close(pipe_ctop[1]); /* Close write-end of pipe_ctop channel */
		exit(EXIT_SUCCESS);
	}
	else 
	{
		close(pipe_ctop[1]); /* Close unused write-end of pipe_ctop channel */
		close(pipe_ptoc[0]); /* Close unused read-end of pipe_ptoc channel */
		
		/* Write message to the write-end of pipe_ptoc */
		write(pipe_ptoc[1], "Greetings!", 11);
		
		close(pipe_ptoc[1]); /* Close write-end of pipe_ptoc channel */

		/*
		* Read the read-end of pipe_ctop channel until EOL is detected, write
		* result to the terminal.
		*/
		while (read(pipe_ctop[0], &buf, 1) > 0)
		{
			write(STDOUT_FILENO, &buf, 1);
		}
		write(STDOUT_FILENO, "\n", 1);
		
		close(pipe_ctop[0]); /* Close read-end of pipe_ctop channel */
		wait(NULL);
		puts("");
		return EXIT_SUCCESS;
	}
}

int second_task()
{
	puts("Second task");
	
	/*
	* Declare:
	* - pid - process ids;
	* - ret - variable, containing return-value of functions
	*/
	pid_t pid[2];
	int ret;
	
	/* Create fifo files */
	ret = mkfifo(T2_FIFO_FTOS_PATH, 0666);
	if (ret == -1 && errno != EEXIST)
	{
		perror("mkfifo \"first to second\"");
	}
	ret = mkfifo(T2_FIFO_STOF_PATH, 0666);
	if (ret == -1 && errno != EEXIST)
	{
		perror("mkfifo \"second to first\"");
	}

	/*
		* Fork first process and call execl to run other binary, calling
		* first_process() function.
		*/
	pid[0] = fork();
	if (pid[0] == -1)
	{
		perror("pid[0] fork");
	}

	if (pid[0] == 0)
	{
		execl("./build/first_proc", NULL);
	}
	else
	{
		/*
		* Fork second process and call execl to run other binary, calling
		* second_process() function.
		*/
		pid[1] = fork();
		if (pid[1] == -1)
		{
			perror("pid[1] fork");
			exit(EXIT_SUCCESS);
		}
		if (pid[1] == 0)
		{
			execl("./build/second_proc", NULL);
		}
		else
		{
			wait(NULL);
			wait(NULL);
		}
	}

	puts("");

	return EXIT_SUCCESS;
}

int third_task(){
	puts("Third task");
	/* 
	* Declare:
	* buf - array of 80 chars;
	* str_ptr - pointer on char to work with strtok();
	* index & sec_index - index variables used in 'for' loops;
	* cur_cmd - variable, containing index of current command for parsing.
	* cur_param - variable, containing index of current param to set.
	*
	* Allocate memory to:
	* cmds - triple pointer on char, used to store arrays of arguments,
	* used in execv() call;
	* cmds_params - pointer on int, used to store current amount of parameters
	* in particular element of cmds[index].
	*
	* Allocate memory in 'for' loop:
	* for every array of arguments - cmds[index];
	* for every string of argument - cmds[index][sec_index].
	*/
	char buf[T3_STDIN_READ_LIM];
	char * str_ptr;
	int index;
	int sec_index;
	int cur_cmd = 0;
	int cur_param = 1;

	cmds = malloc(cmds_lim);
	cmds_params = malloc(cmds_lim);

	for (index = 0; index < cmds_lim; ++index)
	{
		cmds_params[index] = T3_DEF_PARAMS_NUM;
		cmds[index] = malloc(cmds_params[index]);
		for (sec_index = 0; sec_index < cmds_params[index]; ++sec_index)
		{
			cmds[index][sec_index] = malloc(T3_DEF_STR_SIZE);
		}
	}

	int ret = atexit(_cleanup);
	if (ret != 0)
	{
		perror("atexit");
	}

	/* Read user input */
	printf("Print your input: ");
	fgets(buf, T3_STDIN_READ_LIM, stdin);

	/* Replace 10th symbol ('\n') with 0 */
	for (index = 0; index < T3_STDIN_READ_LIM; ++index)
	{
		if (buf[index] == '\n')
		{
			buf[index] = 0;
			break;
		}
	}

	/* Treat first sequence as command name for first arguments array */
	str_ptr = strtok(buf, " ");
	snprintf(cmds[cur_cmd][0], T3_DEF_STR_SIZE, "/bin/%s", str_ptr);

	/* Gradually parse char sequences and set arguments for current command */
	while ((str_ptr = strtok(NULL, " ")) != NULL)
	{
		/* 
		* If parsed "&&" treat next sequences as new command.
		* Set last argument of previous command to NULL and see if there's a
		* need to allocate more memory to cmds pointer, then write the first
		* parsed sequence into cmds[cur_cmds][0].
		*/
		if (str_ptr[0] == '&' && str_ptr[1] == '&')
		{
			cmds[cur_cmd][cur_param] = NULL;

			cur_cmd++;
			cur_param = 1;

			if (cur_cmd >= cmds_lim)
			{
				char * ptr = realloc(cmds_params, (cur_cmd+1));
				if (ptr != NULL)
				{
					cmds_params = ptr;

					cmds_params[cur_cmd] = T3_DEF_PARAMS_NUM;
				}

				ptr = realloc(cmds, (cur_cmd+1));
				if (ptr != NULL)
				{
					cmds = ptr;
					cmds[cur_cmd] = malloc(cmds_params[cur_cmd]);

					for (index = 0; index < cmds_params[cur_cmd]; ++index)
					{
						cmds[cur_cmd][index] = malloc(T3_DEF_STR_SIZE);
					}

					cmds_lim = cur_cmd+1;
				}
			}
			str_ptr = strtok(NULL, " ");
			snprintf(cmds[cur_cmd][0], T3_DEF_STR_SIZE, "/bin/%s", str_ptr);

			continue;
		}

		/* 
		* Allocate more memory if argument need to be placed into index, that
		* is reserved for NULL or cur_param is going out of the space of
		* reserved memory.
		*/
		if (cur_param >= (cmds_params[cur_cmd]-1))
		{
			cmds_params[cur_cmd] += 1;
			char * ptr = realloc(cmds[cur_cmd], cmds_params[cur_cmd]);
			if (ptr != NULL)
			{
				cmds[cur_cmd] = ptr;
				cmds[cur_cmd][cmds_params[cur_cmd]-1] = malloc(T3_DEF_STR_SIZE);
			}
		}

		/* Write write the parsed sequence into current argument of command */
		strncpy(cmds[cur_cmd][cur_param], str_ptr, T3_DEF_STR_SIZE);
		cur_param++;
	}

	/* Set the last argument in command as NULL */
	cmds[cur_cmd][cur_param] = NULL;

	/* 'for' loop to run parsed commands as child processes */
	for (index = 0; index <= cur_cmd; ++index)
	{
		pid_t pid;
		pid = fork();
		if (pid == 0)
		{
			if (execv(cmds[index][0], cmds[index]) < 0)
			{
				strerror(errno);
				perror("execv");
				
			}
		}
		wait(NULL);
	}

	puts("");
		
	exit(EXIT_SUCCESS);
}

int first_process(void)
{
	puts("First subprocess");

	int transmitfd;
	int receivefd;
	char buf;

	transmitfd = open(T2_FIFO_FTOS_PATH, O_WRONLY);
	if (transmitfd == -1)
	{
		perror("open transmitfd");
	}

	write(transmitfd, "hello!", 7);

	receivefd = open(T2_FIFO_STOF_PATH, O_RDONLY);
	if (receivefd == -1)
	{
		perror("open receivefd");
	}

	while(read(receivefd, &buf, 1) > 0 && buf != '\0')
	{
		write(STDOUT_FILENO, &buf, 1);
	}
	write(STDOUT_FILENO, "\n", 1);

	close(transmitfd);
	close(receivefd);
	exit(EXIT_SUCCESS);
}

int second_process(void)
{
	puts("Second subprocess");
	
	int receivefd;
	int transmitfd;
	char buf;

	receivefd = open(T2_FIFO_FTOS_PATH, O_RDONLY);
	if (receivefd == -1)
	{
		perror("open receivefd");
	}

	while(read(receivefd, &buf, 1) > 0 && buf != '\0')
	{
		write(STDOUT_FILENO, &buf, 1);
	}
	write(STDOUT_FILENO, "\n", 1);

	transmitfd = open(T2_FIFO_STOF_PATH, O_WRONLY);
	if (transmitfd == -1)
	{
		perror("open transmitfd");
	}

	write(transmitfd, "hi!", 4);

	close(receivefd);
	close(transmitfd);
	exit(EXIT_SUCCESS);
}
