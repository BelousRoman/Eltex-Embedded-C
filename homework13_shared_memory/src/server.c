#include "../hdr/functions.h"

/*
* Declare global variables:
* - user_count - number of users, already registered;
* - active_users - number of users, currently connected;
* - login - variable, containing shm segment and semaphores, used in logging
*   sequence;
* - users - dynamic array, containing all the info about users, its id, login,
*   shared memory segment and semaphores;
* - ts - variable, used in timedwait in case if user could not unlock semaphore
*   in T3_SERVER_TIMEOUT_SEC seconds.
*/
int users_count = 0;
int active_users = 0;
struct login_t login = {
	.stc_sem = NULL,
	.cts_sem = NULL,
	.srv_sem = NULL,
	.usr_sem = NULL,
	.shm_buf = NULL
};
struct user_t *users = NULL;
struct timespec ts;

/* Signal handler for SIGINT to make sure no memory leaks are possible */
static void sigint_handler(int sig, siginfo_t *si, void *unused)
{
    exit(EXIT_SUCCESS);
}

/* Function provided to atexit() */
void shutdown_server(void)
{
    /*
    * Declare:
    * - index & sec_index - index variables.
    */
    int index;
    int sec_index;

    /* Free login shm and sems, if used */
    if (login.shm_buf != NULL)
    {
        munmap(login.shm_buf, sizeof(struct log_msg_t));
    }
	if (login.stc_sem != NULL)
	{
		sem_close(login.stc_sem);
	}
    if (login.cts_sem != NULL)
	{
		sem_close(login.cts_sem);
	}
    if (login.srv_sem != NULL)
	{
		sem_close(login.srv_sem);
	}
	if (login.usr_sem != NULL)
	{
		sem_close(login.usr_sem);
	}

    /* Unlink login shm and sems */
    sem_unlink(T3_LOGIN_STOC_SEM_NAME);
    sem_unlink(T3_LOGIN_CTOS_SEM_NAME);
    sem_unlink(T3_LOGIN_LOG_USR_SEM_NAME);
    sem_unlink(T3_LOGIN_LOG_SRV_SEM_NAME);

    /* Check if variable users is allocated with memory */
    if (users != NULL)
    {
        /*
        * If there is any connected users -> start cycle, sending disconnect
        * command to every connected user and then shut down connection.
        */
        if (active_users != 0)
        {
            for (index = 0; index < users_count; ++index)
            {
                /*
                * Check if user is connected by seeing whether id is set to -1
                * (disconnected) or is it greater than 0.
                */
                if (-1 == users[index].id)
                    continue;

                /* Send disconnect command to user */
                users[index].shm_buf->stoc.comm = T3_DISCONNECT_COMM;
                sem_post(users[index].stc_sem);

                /* Wait for user to get command */
                if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
                {
                    perror("clock_gettime");
                    exit(EXIT_FAILURE);
                }
                ts.tv_sec += T3_SERVER_TIMEOUT_SEC;
                if ((sem_timedwait(users[index].srv_sem, &ts)) == -1)
                {
                    if (errno == ETIMEDOUT)
                    {
                        printf("User #%d has timed out\n", index);
                    }
                    else
                    {
                        perror("sem_timedout");
                        exit(EXIT_FAILURE);
                    }
                }

                /* Close/unmap user's shm and sems */
                munmap(users[index].shm_buf, sizeof(struct usr_shm_t));
                sem_close(users[index].stc_sem);
                sem_close(users[index].cts_sem);
                sem_close(users[index].srv_sem);
                sem_close(users[index].usr_sem);

                /* Unlink shm and sems */
                shm_unlink(users[index].shm_name);
                sem_unlink(users[index].stc_sem_name);
                sem_unlink(users[index].cts_sem_name);
                sem_unlink(users[index].srv_sem_name);
                sem_unlink(users[index].usr_sem_name);
            }
        }

        /* Free allocated memory */
        free(users);
    }
    
	puts("Server shutdown");
}

int third_task_server()
{
	puts("Third task: Server");
    /*
    * Declare:
    * - sa - sigaction, used to redefine signal handler for SIGINT;
    * - users_list - char array, storing list of currently connected users;
    * - tmp_buf - temporary buffer, used to form message;
    * - users_num - number of users, allocated to 'users' var;
    * - index & sec_index - index variables.
    */
    struct sigaction sa;
    char users_list[T3_USER_MSG_SIZE];
    char tmp_buf[T3_USER_MSG_SIZE];
	int users_num = T3_DEF_ALLOC_USRS;
    int index;
	int sec_index;

	/* Allocate memory to 'users' var */
    users = malloc(sizeof(struct user_t)*users_num);

    /* Clear 'users_list' and 'tmp_buf' */
    memset(users_list, 0, T3_USER_MSG_SIZE);

    /* Fill sa_mask, set signal handler, redefine SIGINT with sa */
    sigfillset(&sa.sa_mask);
    sa.sa_sigaction = sigint_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    /* Set atexit function */
	atexit(shutdown_server);

    /* Create shm segment and semaphores to handle login sequence */
    if ((login.shm_fd = shm_open(T3_LOGIN_SHM_NAME, O_CREAT | O_RDWR, T3_PERMS)) == -1)
	{
		perror("login_shm shm_open");
		exit(EXIT_FAILURE);
	}
	if (ftruncate(login.shm_fd, sizeof(struct log_msg_t)) == -1)
	{
		perror("login_shm ftruncate");
		exit(EXIT_FAILURE);
	}
	if ((login.shm_buf = mmap(NULL, sizeof(struct log_msg_t), PROT_READ | PROT_WRITE, MAP_SHARED,
					        login.shm_fd, 0)) == -1)
	{
		perror("log_msg mmap");
		exit(EXIT_FAILURE);
	}
	if ((login.stc_sem = sem_open(T3_LOGIN_STOC_SEM_NAME, O_CREAT | O_RDWR, T3_PERMS, 0)) == SEM_FAILED)
	{
		perror("login stc_sem sem_open");
		exit(EXIT_FAILURE);
	}
	if ((login.cts_sem = sem_open(T3_LOGIN_CTOS_SEM_NAME, O_CREAT | O_RDWR, T3_PERMS, 0)) == SEM_FAILED)
	{
		perror("login cts_sem sem_open");
		exit(EXIT_FAILURE);
	}
    if ((login.srv_sem = sem_open(T3_LOGIN_LOG_SRV_SEM_NAME, O_CREAT | O_RDWR, T3_PERMS, 0)) == SEM_FAILED)
	{
		perror("login srv_sem sem_open");
		exit(EXIT_FAILURE);
	}
	if ((login.usr_sem = sem_open(T3_LOGIN_LOG_USR_SEM_NAME, O_CREAT | O_RDWR, T3_PERMS, 1)) == SEM_FAILED)
	{
		perror("login_log_usr_sem sem_open");
		exit(EXIT_FAILURE);
	}

    /* Main loop */
	while(1)
	{
        /* Try to lock semaphore, if user is trying to connect */
        if (sem_trywait(login.cts_sem) == -1)
        {
            if (errno != EAGAIN)
            {
                perror("login cts_sem sem_trywait");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            /*
            * Check if current number of users exceeded number, already
            * allocated to 'users' var.
            */
            if (users_count >= users_num)
			{
                /* Increase users_num */
				users_num += T3_DEF_ALLOC_USRS;
                /* Realloc 'users' var */
				struct user_t *tmp = 
                    realloc(users, sizeof(struct user_t) * users_num);
				if (tmp == NULL)
				{
					perror("realloc");
                    exit(EXIT_FAILURE);
				}
			}
            /* Increment users_count and 'active_users' */
			users_count++;
			active_users++;

            /* Set user ID */
			users[users_count-1].id = users_count;

            /*
            * Get login from user, get names of shm segment and semaphores
            * based on ID.
            */
            strncpy(users[users_count-1].login, login.shm_buf->ctos, T3_LOGIN_MSG_SIZE);
			snprintf(users[users_count-1].shm_name, T3_LOGIN_MSG_SIZE,
                T3_USER_SHM_NAME, users[users_count-1].id);
            snprintf(users[users_count-1].stc_sem_name, T3_LOGIN_MSG_SIZE,
                T3_USER_STOC_SEM_NAME, users[users_count-1].id);
            snprintf(users[users_count-1].cts_sem_name, T3_LOGIN_MSG_SIZE,
                T3_USER_CTOS_SEM_NAME, users[users_count-1].id);
            snprintf(users[users_count-1].srv_sem_name, T3_LOGIN_MSG_SIZE,
                T3_USER_SRV_SEM_NAME, users[users_count-1].id);
            snprintf(users[users_count-1].usr_sem_name, T3_LOGIN_MSG_SIZE,
                T3_USER_USR_SEM_NAME, users[users_count-1].id);

            /* Create shm segment and semaphores to interact with user */
            if ((users[users_count-1].usr_shm = shm_open(users[users_count-1].shm_name, O_CREAT | O_RDWR, T3_PERMS)) == -1)
            {
                perror("usr_shm shm_open");
                exit(EXIT_FAILURE);
            }
            if (ftruncate(users[users_count-1].usr_shm, sizeof(struct usr_shm_t)) == -1)
            {
                perror("usr_shm ftruncate");
                exit(EXIT_FAILURE);
            }
            if ((users[users_count-1].shm_buf = mmap(NULL, sizeof(struct usr_shm_t), PROT_READ | PROT_WRITE, MAP_SHARED,
                                    users[users_count-1].usr_shm, 0)) == -1)
            {
                perror("shm_buf mmap");
                exit(EXIT_FAILURE);
            }
            if ((users[users_count-1].stc_sem = sem_open(users[users_count-1].stc_sem_name, O_CREAT | O_RDWR, T3_PERMS, 0)) == SEM_FAILED)
            {
                perror("stc_sem sem_open");
                exit(EXIT_FAILURE);
            }
            if ((users[users_count-1].cts_sem = sem_open(users[users_count-1].cts_sem_name, O_CREAT | O_RDWR, T3_PERMS, 0)) == SEM_FAILED)
            {
                perror("cts_sem sem_open");
                exit(EXIT_FAILURE);
            }
            if ((users[users_count-1].srv_sem = sem_open(users[users_count-1].srv_sem_name, O_CREAT | O_RDWR, T3_PERMS, 0)) == SEM_FAILED)
            {
                perror("srv_sem sem_open");
                exit(EXIT_FAILURE);
            }
            if ((users[users_count-1].usr_sem = sem_open(users[users_count-1].usr_sem_name, O_CREAT | O_RDWR, T3_PERMS, 0)) == SEM_FAILED)
            {
                perror("usr_sem sem_open");
                exit(EXIT_FAILURE);
            }

            printf("User #%d \"%s\" has been registered\n",
                    users[users_count-1].id,
                    users[users_count-1].login);

            /* Send ID to user */
            snprintf(login.shm_buf->stoc, T3_LOGIN_MSG_SIZE, "%d", users[users_count-1].id);
            if (sem_post(login.stc_sem) == -1)
            {
                perror("sem_post");
                exit(1);
            }

            /* Wait for user to get command */
            if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
            {
                perror("clock_gettime");
                exit(EXIT_FAILURE);
            }
            ts.tv_sec += T3_SERVER_TIMEOUT_SEC;
            if ((sem_timedwait(login.srv_sem, &ts)) == -1)
            {
                /* Delete user on timeout*/
                if (errno == ETIMEDOUT)
                {
                    printf("User #%d \"%s\" has timed out\n", users[users_count-1].id, users[users_count-1].login);

                    munmap(users[users_count-1].shm_buf, sizeof(struct usr_shm_t));
                    shm_unlink(users[users_count-1].shm_name);

                    sem_close(users[users_count-1].stc_sem);
                    sem_close(users[users_count-1].cts_sem);
                    sem_close(users[users_count-1].srv_sem);
                    sem_close(users[users_count-1].usr_sem);

                    sem_unlink(users[users_count-1].stc_sem_name);
                    sem_unlink(users[users_count-1].cts_sem_name);
                    sem_unlink(users[users_count-1].srv_sem_name);
                    sem_unlink(users[users_count-1].usr_sem_name);

                    users[users_count-1].id = -1;
                }
                /* Shut server upon error */
                else
                {
                    perror("sem_timedwait");
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                /* Send to connected users updated userlist */
                printf("User #%d \"%s\" has succesfully connected\n", users[users_count-1].id, users[users_count-1].login);
				snprintf(tmp_buf, T3_USER_MSG_SIZE, "<%s>\n",
                            users[users_count-1].login);
				strncat(users_list, tmp_buf, T3_USER_MSG_SIZE);
				for (index = 0; index < users_count; ++index)
				{
                    if (users[index].id == -1)
                        continue;
                    users[index].shm_buf->stoc.comm = T3_UPD_USRLIST_COMM;
                    strncpy(users[index].shm_buf->stoc.msg, users_list, T3_USER_MSG_SIZE);
                    sem_post(users[index].stc_sem);
				}
			}

            sem_post(login.usr_sem);
        }
        /* Read connected users */
        for (index = 0; index < users_count; ++index)
        {
            if (users[index].id == -1)
				continue;
            if (sem_trywait(users[index].cts_sem) == -1)
            {
                if (errno != EAGAIN)
                {
                    perror("cts_sem sem_trywait");
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                /* Parse command */
                switch (users[index].shm_buf->ctos.comm)
                {
                /* If user has sent a message -> resend it to every connected user*/
                case T3_MSGSND_COMM:
                    printf("Received from \"%s\": <%s>\n", users[index].login, users[index].shm_buf->ctos.msg);
                    snprintf(tmp_buf, T3_USER_MSG_SIZE, "%s: %s", users[index].login, users[index].shm_buf->ctos.msg);
                    sem_post(users[index].usr_sem);

                    for (sec_index = 0; sec_index < users_count; ++sec_index)
                    {
                        if (users[sec_index].id == -1)
                            continue;
                            
                        users[sec_index].shm_buf->stoc.comm = T3_MSGSND_COMM;
                        strncpy(users[sec_index].shm_buf->stoc.msg, tmp_buf, T3_USER_MSG_SIZE);
                        sem_post(users[sec_index].stc_sem);

                        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
                        {
                            perror("clock_gettime");
                            exit(EXIT_FAILURE);
                        }
                        ts.tv_sec += T3_SERVER_TIMEOUT_SEC;
                        if ((sem_timedwait(users[sec_index].srv_sem, &ts)) == -1)
                        {
                            if (errno == ETIMEDOUT)
                            {
                                printf("User #%d has timed out\n", sec_index);
                            }
                            else
                            {
                                perror("sem_timedout");
                                exit(EXIT_FAILURE);
                            }
                        }
                    }
                    break;
                /*
                * If user has requested to change login -> update userlist and
                * then send it to every connected user.
                */
                case T3_SET_USRNAME_COMM:
                    strncpy(users[index].login, users[index].shm_buf->ctos.msg, T3_LOGIN_MSG_SIZE);
                        
                    memset(users_list, 0, T3_USER_MSG_SIZE);
                    for (sec_index = 0; sec_index < users_count; ++sec_index)
            		{
                        if (users[sec_index].id == -1)
                            continue;
            			snprintf(tmp_buf, T3_USER_MSG_SIZE, "<%s>\n",
                                    users[sec_index].login);
            			strncat(users_list, tmp_buf, T3_USER_MSG_SIZE);
            		}
                    for (sec_index = 0; sec_index < users_count; ++sec_index)
            		{
                        if (users[sec_index].id == -1)
                            continue;
                        users[sec_index].shm_buf->stoc.comm = T3_UPD_USRLIST_COMM;
                        strncpy(users[sec_index].shm_buf->stoc.msg, users_list, T3_USER_MSG_SIZE);

                        sem_post(users[sec_index].stc_sem);

                        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
                            {
                            perror("clock_gettime");
                            exit(EXIT_FAILURE);
                        }
                        ts.tv_sec += T3_SERVER_TIMEOUT_SEC;
                        if ((sem_timedwait(users[sec_index].srv_sem, &ts)) == -1)
                        {
                            if (errno == ETIMEDOUT)
                            {
                                printf("User #%d has timed out\n", sec_index);
                            }
                            else
                            {
                                perror("sem_timedout");
                                exit(EXIT_FAILURE);
                            }
                        }
            		}
                    printf("User #%d has been assigned new name: <%s>\n", users[index].id, users[index].login);
                    break;
                /*
                * If user has requested userlist -> send userlist to this user.
                */
                case T3_REQ_USRLIST_COMM:
                    users[index].shm_buf->stoc.comm = T3_UPD_USRLIST_COMM;
                    strncpy(users[index].shm_buf->stoc.msg, users_list, T3_USER_MSG_SIZE);

                    sem_post(users[index].stc_sem);

                    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
                    {
                        perror("clock_gettime");
                        exit(EXIT_FAILURE);
                    }
                    ts.tv_sec += T3_SERVER_TIMEOUT_SEC;
                    if ((sem_timedwait(users[index].srv_sem, &ts)) == -1)
                    {
                        if (errno == ETIMEDOUT)
                        {
                            printf("User #%d has timed out\n", sec_index);
                        }
                        else
                        {
                            perror("srv_sem sem_timedout");
                            exit(EXIT_FAILURE);
                        }
                    }
                    printf("User %s has requested userlist\n", users[index].login);
                    break;
                /*
                * If user has disconnected userlist -> close fds, unlink files
                * and send updated userlist to rest connected users.
                */
                case T3_DISCONNECT_COMM:
                    munmap(users[index].shm_buf, sizeof(struct usr_shm_t));
                    shm_unlink(users[index].shm_name);

                    sem_close(users[index].stc_sem);
                    sem_close(users[index].cts_sem);
                    sem_close(users[index].srv_sem);
                    sem_close(users[index].usr_sem);

                    sem_unlink(users[index].stc_sem_name);
                    sem_unlink(users[index].cts_sem_name);
                    sem_unlink(users[index].srv_sem_name);
                    sem_unlink(users[index].usr_sem_name);
                    
                    printf("User #%d \"%s\" disconnected\n", users[index].id, users[index].login);

                    users[index].id = -1;
                    active_users--;
                    if (active_users <= 0)
                    {
                        exit(EXIT_SUCCESS);
                    }
                    memset(users_list, 0, T3_USER_MSG_SIZE);
                    for (sec_index = 0; sec_index < users_count; ++sec_index)
            		{
                        if (users[sec_index].id == -1)
                            continue;
            			snprintf(tmp_buf, T3_USER_MSG_SIZE, "<%s>\n",
                                    users[sec_index].login);
            			strncat(users_list, tmp_buf, T3_USER_MSG_SIZE);
            		}
                    for (sec_index = 0; sec_index < users_count; ++sec_index)
            		{
                        if (users[sec_index].id == -1)
                            continue;
                        users[sec_index].shm_buf->stoc.comm = T3_UPD_USRLIST_COMM;
                        strncpy(users[sec_index].shm_buf->stoc.msg, users_list, T3_USER_MSG_SIZE);

                        sem_post(users[sec_index].stc_sem);

                        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
                            {
                            perror("clock_gettime");
                            exit(EXIT_FAILURE);
                        }
                        ts.tv_sec += T3_SERVER_TIMEOUT_SEC;
                        if ((sem_timedwait(users[sec_index].srv_sem, &ts)) == -1)
                        {
                            if (errno == ETIMEDOUT)
                            {
                                printf("User #%d has timed out\n", sec_index);
                            }
                            else
                            {
                                perror("sem_timedout");
                                exit(EXIT_FAILURE);
                            }
                        }
            		}
                    break;
                default:
                    break;
                }
            }
        }
    }
    
	return EXIT_SUCCESS;
}
