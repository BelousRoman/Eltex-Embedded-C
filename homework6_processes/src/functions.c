#include "../hdr/functions.h"
#include <errno.h>


int first_task(){
	printf("First task\n");

	pid_t frst_child_pid, scnd_child_pid,
	      frst_grandchild_pid, scnd_grandchild_pid, thrd_grandchild_pid;

	int frst_child_res, scnd_child_res;

	frst_child_pid = fork();
	if (frst_child_pid == 0){
		int frst_wstatus, scnd_wstatus;
		frst_grandchild_pid = fork();
		if (frst_grandchild_pid == 0){
			printf("4 First grandchild pid is equal to %d, it's ppid = %d\n", getpid(), getppid());
			exit(EXIT_SUCCESS);
		}
		else {
			scnd_grandchild_pid = fork();
			if (scnd_grandchild_pid == 0){
				printf("5 Second grandchild pid is equal to %d, it's ppid = %d\n", getpid(), getppid());
				exit(EXIT_SUCCESS);
			}
			else {
				printf("2 First child pid is equal to %d, it's ppid = %d\n", getpid(), getppid());
				waitpid(frst_grandchild_pid, &frst_wstatus, 0);
				waitpid(scnd_grandchild_pid, &scnd_wstatus, 0);
				exit(EXIT_SUCCESS);
			}
		}
	}
	else {
		scnd_child_pid = fork();
		if (scnd_child_pid == 0){
			int thrd_wstatus;
			thrd_grandchild_pid = fork();
			if (thrd_grandchild_pid == 0){
				printf("6 Third grandchild pid is equal to %d, it's ppid = %d\n", getpid(), getppid());
				exit(EXIT_SUCCESS);
			}
			else {
				printf("3 Second child pid is equal to %d, it's ppid = %d\n", getpid(), getppid());
				waitpid(thrd_grandchild_pid, &thrd_wstatus, 0);
				exit(EXIT_SUCCESS);
			}
		}
		printf("1 Main process pid is equal to %d\n", getpid());
		waitpid(frst_child_pid, &frst_child_res, 0);
		waitpid(scnd_child_pid, &scnd_child_res, 0);
		printf("The result for a program is %d + %d (1st child + 2nd child)\n", frst_child_res, scnd_child_res);
		return (frst_child_res + scnd_child_res);
	}
}

int second_task(){
	printf("Second task\n");

	char ***cmds = NULL;
	int cmds_lim = DEF_CMDS_LIM;
	int *cmds_params = NULL;

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
	char buf[80];
	char * str_ptr;
	int index;
	int sec_index;
	int cur_cmd = 0;
	int cur_param = 1;

	printf("alloc to cmds = %d\n", cmds_lim);
	cmds = malloc(cmds_lim);
	cmds_params = malloc(cmds_lim);

	for (index = 0; index < cmds_lim; ++index)
	{
		cmds_params[index] = DEF_PARAMS_NUM;
		printf("alloc to cmds[%d] = %d\n", index, cmds_params[index]);
		cmds[index] = malloc(cmds_params[index]);
		for (sec_index = 0; sec_index < cmds_params[index]; ++sec_index)
		{
			printf("alloc cmds[%d][%d] to %d\n", index, sec_index, DEF_STR_SIZE);
			cmds[index][sec_index] = malloc(DEF_STR_SIZE);
		}
	}

	/* Read user input */
	fgets(buf, STDIN_READ_LIM, stdin);
	// snprintf(buf, STDIN_READ_LIM, "ls -a -l\n");

	for (index = STDIN_READ_LIM; index >= 0; --index)
	{
		if (buf[index] == '\n')
		{
			buf[index] = 0;
			break;
		}
	}

	/* Treat first sequence as command name for first arguments array */
	str_ptr = strtok(buf, " ");
	
	snprintf(cmds[cur_cmd][0], DEF_STR_SIZE, "/bin/%s", str_ptr);
	
	printf("<%s>\n", cmds[0][0]);

	while ((str_ptr = strtok(NULL, " ")) != NULL)
	{
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

					cmds_params[cur_cmd] = DEF_PARAMS_NUM;
				}

				ptr = realloc(cmds, (cur_cmd+1));
				if (ptr != NULL)
				{
					cmds[cur_cmd] = malloc(cmds_params[cur_cmd]);

					for (index = 0; index < cmds_params[cur_cmd]; ++index)
					{
						cmds[cur_cmd][index] = malloc(DEF_STR_SIZE);
					}

					cmds_lim = cur_cmd+1;
				}
			}
			str_ptr = strtok(NULL, " ");
			snprintf(cmds[cur_cmd][0], DEF_STR_SIZE, "/bin/%s", str_ptr);
			printf("<%s>\n", cmds[cur_cmd][0]);

			continue;
		}

		if (cur_param >= (cmds_params[cur_cmd]-1))
		{
			cmds_params[cur_cmd] += 1;
			// cur_param++;

			printf("Realloc cmds_params[%d] to %d\n", cur_cmd, cmds_params[cur_cmd]);
			char * ptr = realloc(cmds[cur_cmd], cmds_params[cur_cmd]);
			if (ptr != NULL)
			{
				// cmds[cur_cmd] = ptr;
				printf("alloc cmds[%d][%d] to %d\n", cur_cmd, cmds_params[cur_cmd]-1, DEF_STR_SIZE);
				cmds[cur_cmd][cmds_params[cur_cmd]-1] = malloc(DEF_STR_SIZE);
			}
		}

		printf("Strncpy %s to cmds[%d][%d]\n", str_ptr, cur_cmd, cur_param);
		strncpy(cmds[cur_cmd][cur_param], str_ptr, DEF_STR_SIZE);
		cur_param++;
	}

	printf("set cmds[%d][%d] to NULL\n", cur_cmd, cmds_params[cur_cmd]-1);
	// cmds[cur_cmd][(cmds_params[cur_cmd]-1)] = NULL; // old one
	cmds[cur_cmd][cur_param] = NULL;

	for (index = 0; index <= cur_cmd; ++index)
	{
		pid_t pid;
		pid = fork();
		if (pid == 0)
		{
			printf("Forked to call cmd: %s\n", cmds[index][0]);
			if (execv(cmds[index][0], cmds[index]) < 0)
			{
				strerror(errno);
				puts("execv() call returned error\n");
			}
		}
		wait(NULL);
	}

	printf("cur cmd = %d; cur param = %d; max params = %d\n", cur_cmd, cur_param, cmds_params[cur_cmd]);


	/* Free allocated memory */
	printf("free until index >= %d\n", cmds_lim);
	for (index = 0; index < cmds_lim; ++index)
	{
		printf("free args until sec_index >= %d\n", cmds_params[index]);
		// for (sec_index = 0; sec_index < cmds_params[index]; ++sec_index)
		for (sec_index = cmds_params[index]-1; sec_index >= 0; --sec_index)
		{
			printf("Free cmds[%d][%d] = %ld from %ld\n", index, sec_index, &cmds[index][sec_index], &cmds[index]);
			if (&cmds[index][sec_index] == NULL)
			{
				puts("Is null x(");
				continue;
			}
			free(cmds[index][sec_index]);
		}
		free(cmds[index]);
	}
	free(cmds);
	free(cmds_params);
	
	exit(EXIT_SUCCESS);
}