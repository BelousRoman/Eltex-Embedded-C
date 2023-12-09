#include "../hdr/functions.h"

/*
* Global variables to be able to call atexit function to free used resources,
* close and unlink queues.
*/
struct user_t *users = NULL;
int users_count = 0;
int active_users = 0;
mqd_t login_q = 0;

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

    /* Close login queue and unlink file if login_q is opened */
    if (login_q > 0)
    {
        mq_close(login_q);
	    mq_unlink(T3_LOGIN_Q_NAME);
    }

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
                mq_send(users[index].snd_q, "0", T3_USER_Q_MSG_SIZE,
                        T3_DISCONNECT_PRIO);

                /* Close queues and delete files */
                mq_close(users[index].rcv_q);
                mq_close(users[index].snd_q);
                mq_unlink(users[index].cts_ch);
                mq_unlink(users[index].stc_ch);
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
    * - tmp_buf - char array, storing received message;
    * - users_num - number of users, allocated to 'users' var;
    * - login_attr & user_attr - attributes of login and user queues;
    * - prio - priority of the received message;
    * - index & sec_index - index variables;
    * - ret - ret value.
    */
    struct sigaction sa;
    char users_list[T3_USER_Q_MSG_SIZE];
    char tmp_buf[T3_USER_Q_MSG_SIZE];
	int users_num = T3_DEF_ALLOC_USRS;
	struct mq_attr login_attr;
	struct mq_attr user_attr;
    unsigned int prio;
    int index;
	int sec_index;
    ssize_t ret;

	/* Allocate memory to 'users' var */
    users = malloc(sizeof(struct user_t)*users_num);

    /* Set attributes */
	login_attr.mq_maxmsg = T3_LOGIN_Q_MAX_MSGS;
	login_attr.mq_msgsize = T3_LOGIN_Q_MSG_SIZE;
	user_attr.mq_maxmsg = T3_USER_Q_MAX_MSGS;
	user_attr.mq_msgsize = T3_USER_Q_MSG_SIZE;

    /* Clear users_list */
    memset(users_list, 0, T3_USER_Q_MSG_SIZE);

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

    /* Create login queue and open it in nonblock mode */
	login_q = mq_open(T3_LOGIN_Q_NAME, O_CREAT | O_RDWR | O_NONBLOCK, T3_PERMS,
						&login_attr);
	if (-1 == login_q)
		perror("login_q mq_open");

    /* Main loop */
	while(1)
	{
        /* Read login queue */
		if ((ret = mq_receive(login_q, tmp_buf, T3_LOGIN_Q_MSG_SIZE, &prio))
			 == -1)
		{
            /*
            * Check if mq_receive returned -1 because any error, but empty
            * queue.
            */
            if (errno != EAGAIN)
            {
                perror("login_q mq_receive");
                exit(EXIT_FAILURE);
            }
		}
		else if (ret > 0)
		{
            /*
            * Check if priority of received message is equal of ones, sent by
            * server, if so -> send same message with same priority to queue.
            */
			if (T3_SERVER_PRIO == prio)
			{
				mq_send(login_q, tmp_buf, T3_LOGIN_Q_MSG_SIZE, prio);
				memset(tmp_buf, 0, T3_LOGIN_Q_MSG_SIZE);
			}
            /*
            * Check if priority of received message is equal of ones, sent by
            * user, trying to connect to server
            */
			else if (T3_USER_PRIO == prio)
			{
                /*
                * Check if current number of users exceeded max number of
                * allocated users, if so -> realloc 'users' var to contain more
                * users.
                * */
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

                /* Set user's login to default 'user%d' */
				snprintf(users[users_count-1].login, T3_LOGIN_Q_MSG_SIZE,
                            "user%d", users[users_count-1].id);

                /* Set channel variables */
				snprintf(users[users_count-1].stc_ch, T3_LOGIN_Q_MSG_SIZE,
                    T3_STOC_CH_NAME, users[users_count-1].id);
				snprintf(users[users_count-1].cts_ch, T3_LOGIN_Q_MSG_SIZE,
                    T3_CTOS_CH_NAME, users[users_count-1].id);

                /* Terminal output, notifying when user is connected */
				printf("Register \"%s\" as user #%d with %s & %s channels\n",
                        users[users_count-1].login,
                        users[users_count-1].id,
                        users[users_count-1].stc_ch,
                        users[users_count-1].cts_ch);
                
                /* Open receive and send queues */
				users[users_count-1].rcv_q = 
                    mq_open(users[users_count-1].cts_ch,
                    O_CREAT | O_RDONLY | O_NONBLOCK, T3_PERMS,
                    &user_attr);
				if (users[users_count-1].rcv_q == -1)
				{
                    perror("rcv_q mq_open");
                    exit(EXIT_FAILURE);
                }
				users[users_count-1].snd_q = 
                    mq_open(users[users_count-1].stc_ch,
					O_CREAT | O_WRONLY | O_NONBLOCK, T3_PERMS,
                    &user_attr);
				if (users[users_count-1].snd_q == -1)
                {
                    perror("snd_q mq_open");
                    exit(EXIT_FAILURE);
                }

                /* Write user id to tmp_buf and send message back to user */
				snprintf(tmp_buf, T3_LOGIN_Q_MSG_SIZE, "%d",
                            users[users_count-1].id);
				mq_send(login_q, tmp_buf, T3_LOGIN_Q_MSG_SIZE,
						T3_SERVER_PRIO);

                /*
                * Append 'users_list' with connected user and resend update
                * userlist command to every connected user.
                */
				snprintf(tmp_buf, T3_USER_Q_MSG_SIZE, "<%s>\n",
                            users[index].login);
				strncat(users_list, tmp_buf, T3_USER_Q_MSG_SIZE);
				for (index = 0; index < users_count; ++index)
				{
                    if (users[index].id == -1)
                        continue;
					mq_send(users[index].snd_q, users_list, T3_USER_Q_MSG_SIZE,
                            T3_UPD_USRLIST_PRIO);
				}
			}
		}

        /* Try to receive messages from every connected user */
		for (index = 0; index < users_count; ++index)
		{
			if (users[index].id == -1)
				continue;
			if ((ret = mq_receive(users[index].rcv_q, users[index].rcv_buf,
                                    T3_USER_Q_MSG_SIZE, &prio)) == -1)
			{
                /*
                * Check if mq_receive returned -1 because any error, but empty
                * queue.
                */
                if (errno != EAGAIN)
                {
                    perror("rcv_q mq_receive");
				    exit(EXIT_FAILURE);
                }
			}
			else if (ret > 0)
			{
                /*
                * Check if priority of received message is equal of ones, sent by
                * user, sending message.
                */
				if (T3_USER_PRIO == prio)
				{
                    /*
                    * Terminal output, notifying when message from user is
                    * received.
                    */
					printf("Received: <%s> from user \"%s\"\n",
                            users[index].rcv_buf, users[index].login);

					snprintf(tmp_buf, T3_USER_Q_MSG_SIZE, "%s: %s",
                            users[index].login, users[index].rcv_buf);
					for (sec_index = 0; sec_index < users_count; ++sec_index)
					{
						if (users[sec_index].id != -1)
						{
							mq_send(users[sec_index].snd_q, tmp_buf,
                                    T3_USER_Q_MSG_SIZE, T3_MSGSND_PRIO);
						}
					}
				}
                /*
                * Check if priority of received message is equal of ones, sent by
                * user, sending server request to change user login.
                */
				else if (T3_SET_USRNAME_PRIO == prio)
				{
                    /* Change user login to one, read to rcv_buf */
					snprintf(users[index].login, T3_LOGIN_Q_MSG_SIZE, "%s",
                            users[index].rcv_buf);

                    /* Terminal output, notifying when user login is changed */
					printf("User #%d has been assigned new name: <%s>\n",
                            users[index].id, users[index].login);

                    /*
                    * Clear 'users_list', then continiously append every
                    * connected user.
                    */
					memset(users_list, 0, T3_USER_Q_MSG_SIZE);
					for (sec_index = 0; sec_index < users_count; ++sec_index)
					{
                        if (users[sec_index].id == -1)
                            continue;
						snprintf(tmp_buf, T3_USER_Q_MSG_SIZE, "<%s>\n",
                                    users[sec_index].login);
						strncat(users_list, tmp_buf, T3_USER_Q_MSG_SIZE);
					}
                    /* Send updated 'users_list' to every connected user */
					for (sec_index = 0; sec_index < users_count; ++sec_index)
					{
                        if (users[sec_index].id == -1)
                            continue;
						mq_send(users[sec_index].snd_q, users_list,
                                T3_USER_Q_MSG_SIZE, T3_UPD_USRLIST_PRIO);
					}
				}
                /*
                * Check if priority of received message is equal of ones, sent by
                * user, sending server request to send 'users_list'.
                */
				else if (T3_REQ_USRLIST_PRIO == prio)
				{
                    /*
                    * Terminal output, notifying when user is requested
                    * userlist.
                    */
					printf("User %s has requested userlist\n", users[index].login);
					mq_send(users[index].snd_q, users_list, T3_USER_Q_MSG_SIZE,
                            T3_UPD_USRLIST_PRIO);
				}
                /*
                * Check if priority of received message is equal of ones, sent by
                * user, disconnecting from server.
                */
				else if (T3_DISCONNECT_PRIO == prio)
				{
                    /* Terminal output, notifying when user is disconnected */
					printf("User %d %s disconnected\n", users[index].id,
                            users[index].login);

                    /* Close queues and delete files */
					mq_close(users[index].rcv_q);
					mq_close(users[index].snd_q);
					mq_unlink(users[index].cts_ch);
					mq_unlink(users[index].stc_ch);

                    /* Set user id to -1, decrement 'active_users'*/
                    users[index].id = -1;
					active_users--;

                    /* Exit app is there is no connected users left */
					if (active_users <= 0)
					{
                        exit(EXIT_SUCCESS);
                    }

                    /*
                    * Clear 'users_list', then continiously append every
                    * connected user.
                    */
					memset(users_list, 0, T3_USER_Q_MSG_SIZE);
					for (sec_index = 0; sec_index < users_count; ++sec_index)
					{
						if (users[sec_index].id == -1)
							continue;
						snprintf(tmp_buf, T3_USER_Q_MSG_SIZE, "<%s>\n",
                                    users[sec_index].login);
						strncat(users_list, tmp_buf, T3_USER_Q_MSG_SIZE);
					}

                    /* Send updated 'users_list' to every connected user */
					for (sec_index = 0; sec_index < users_count; ++sec_index)
					{
                        if (users[sec_index].id == -1)
							continue;
						mq_send(users[sec_index].snd_q, users_list,
                                T3_USER_Q_MSG_SIZE, T3_UPD_USRLIST_PRIO);
					}
				}
			}
		}
	}

	return EXIT_SUCCESS;
}
