#include "../hdr/functions.h"
#include <malloc.h>

struct command_t *cmds =  NULL;

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

	char buf[80];
	char * str_ptr;
	cmds = malloc(sizeof(struct command_t) * DEF_CMDS_LIM);
	if (atexit(free_on_exit) != 0) {
		free(cmds);
		perror("atexit");
		exit(EXIT_FAILURE);
	}

	while (1) {
		printf("Print your input here: ");
		fgets(buf, 80, stdin);
		int cmds_num = 0;
		cmds[cmds_num].param1 = 0;
		cmds[cmds_num].param2 = 0;
		cmds[cmds_num].param3 = 0;
		cmds[cmds_num].param4 = 0;

		str_ptr = strtok(buf, " ");
		strncpy(cmds[cmds_num].cmd, str_ptr, sizeof(cmds[cmds_num].cmd));
		
		for (int i = sizeof(cmds[0].cmd)-1; i >= 0; --i){
			if (cmds[cmds_num].cmd[i] == '\n'){
				cmds[cmds_num].cmd[i] = 0;
				break;
			}
		}

		while ((str_ptr = strtok(NULL, " ")) != NULL){
			for (int i = 1; str_ptr[i] != '\0' && str_ptr[i] != '\n'; ++i){
				if (str_ptr[0] == '-'){
					int index = str_ptr[i] - 'a';
					if (index >= 0 && index < 8){
						cmds[cmds_num].param1 = 
							cmds[cmds_num].param1 | 
							(1 << index);
					}
					else if (index >= 8 && index < 16){
						index = index - 8;
						cmds[cmds_num].param2 = 
							cmds[cmds_num].param2 | 
							(1 << index);
					}
					else if (index >= 16 && index < 24){
						index = index - 16;
						cmds[cmds_num].param3 = 
							cmds[cmds_num].param3 | 
							(1 << index);
					}
					else if (index >= 24 && index < 32){
						index = index - 24;
						cmds[cmds_num].param4 = 
							cmds[cmds_num].param4 | 
							(1 << index);
					}
				}
				else if (str_ptr[0] == '&' && str_ptr[1] == '&'){
					cmds_num++;
					if (cmds_num > DEF_CMDS_LIM){
						if (realloc(cmds, sizeof(struct command_t) * cmds_num) == NULL){
							perror("realloc");
							exit(EXIT_FAILURE);
						}
					}
					str_ptr = strtok(NULL, " ");
					strncpy(cmds[cmds_num].cmd, str_ptr, 
							sizeof(cmds[cmds_num].cmd));
					
					for (int i = sizeof(cmds[0].cmd)-1; i >= 0; --i){
						if (cmds[cmds_num].cmd[i] == '\n'){
							cmds[cmds_num].cmd[i] = 0;
							break;
						}
					}

					cmds[cmds_num].param1 = 0;
					cmds[cmds_num].param2 = 0;
					cmds[cmds_num].param3 = 0;
					cmds[cmds_num].param4 = 0;
				}
			}
		}
		for (int i = 0; i <= cmds_num; ++i){
			if (strcmp(cmds[i].cmd, "help") == 0) {
				command_help();
			}
			else if (strcmp(cmds[i].cmd, "exit") == 0) {
				command_exit();
			}
			else {
				printf("Unknown command '%s', type help to see list of commands\n", cmds[i].cmd);
			}
		}
	}
	
	exit(EXIT_SUCCESS);
}

void free_on_exit(void){
	free(cmds);
}

void command_help(){
	printf(	"\thelp - list of all commands\n"
		"\texit - exit the program\n");
}

void command_exit(){
	exit(EXIT_SUCCESS);
}
