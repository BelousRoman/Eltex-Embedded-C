#include "../hdr/functions.h"

/*
* Global variables used:
* - to get current terminal window size;
* - to be able to call atexit function to free used resources, close and unlink
*   queues.
*/
struct winsize size;
mqd_t login_q;
struct user_t user;
struct q_handler_t q_handler;

/* Function to process signal, called on resizing terminal window */
void sig_winch(int signo)
{
	ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size);
	resizeterm(size.ws_row, size.ws_col);
}

/* Signal handler for SIGINT to make sure no memory leaks are possible */
static void sigint_handler(int sig, siginfo_t *si, void *unused)
{
    exit(EXIT_SUCCESS);
}

/* Function to handle message when new message in queue occurs */
static void handle_msg(union sigval sv)
{
    /*
    * Declare:
    * - q_handler - is a pointer to struct, containing queue, windows and char
    *   array buffer;
    * - sev - sigevent, to repeatedly recall mq_notify;
    * - prio - prioirity of received message;
    * - ret - return value.
    */
	struct q_handler_t *q_handler = ((struct q_handler_t *) sv.sival_ptr);
    struct sigevent sev;
    int prio;
    ssize_t ret;
    /*
    * Configure sigevent, set notification method, function called, attributes
    * and pointer to q_handler.
    * */
	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_notify_function = handle_msg;
	sev.sigev_notify_attributes = NULL;
	sev.sigev_value.sival_ptr = q_handler;

    /* Read queue until an error occurs */
	while (ret = mq_receive(q_handler->rcv_q, q_handler->buf, T3_USER_Q_MSG_SIZE,
                        &prio) > 0)
	{
		/*
		* Check if priority of received message is equal of ones, sent by
		* server, resending message to every connected user.
		*/
		if (T3_MSGSND_PRIO == prio)
		{
			wprintw(q_handler->main_wnd, "%s\n", q_handler->buf);
			wrefresh(q_handler->main_wnd);
		}
		/*
		* Check if priority of received message is equal of ones, sent by
		* server, sending userlist.
		*/
		else if (T3_UPD_USRLIST_PRIO == prio)
		{
			wclear(q_handler->users_wnd);
			wprintw(q_handler->users_wnd, "%s", q_handler->buf);
			wrefresh(q_handler->users_wnd);
		}
		/*
		* Check if priority of received message is equal of ones, sent by
		* server, sending disconnect command.
		*/
		else if (T3_DISCONNECT_PRIO == prio)
		{
			exit(EXIT_SUCCESS);
		}
	}
    if (ret == -1)
    {
		if (errno != EAGAIN)
		{
			perror("handle_msg mq_receive");
			exit(EXIT_FAILURE);
		}
	}

    /* Register mq_notify */
	while (ret = mq_notify(q_handler->rcv_q, &sev) == -1)
	{
		perror("rcv_q mq_notify");
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}

void shutdown_client(void)
{
    /* Delete windows, deinit screen */
    delwin(q_handler.main_wnd);
    delwin(q_handler.users_wnd);
    delwin(q_handler.msg_wnd);
    delwin(q_handler.wnd);
    endwin();

    /* Send disconnect command to server, close queues and delete files */
	if (login_q > 0)
	{
		mq_close(login_q);
	}
    
	if (user.snd_q > 0)
	{
		mq_send(user.snd_q, "0", T3_USER_Q_MSG_SIZE, T3_DISCONNECT_PRIO);
		mq_close(user.snd_q);
	}
    
	if (user.rcv_q)
	{
		mq_close(user.rcv_q);
	}

	puts("Client shutdown");
}

int _login_user(WINDOW **pwnd, char *name)
{
    /*
    * Declare:
    * - wnd & subwnd - windows, created from pwnd by derwin();
    * - symbol - symbol, read by wgetch;
    * - index - current index of symbol in name char array.
    */
    WINDOW *wnd;
	WINDOW *subwnd;
	int symbol;
    int index = 0;

    /* Derive windows from pwnd, fill with info */
	wnd = derwin(*pwnd, 7, 30, (size.ws_row/2)-3, (size.ws_col/2)-11);
	box(wnd, '|', '-');
	subwnd = derwin(wnd, 1, 28, 3, 1);
	wmove(wnd, 1, 8);
	wprintw(wnd, "Enter user name");
	wmove(wnd, 2, 1);
	whline(wnd, '*', 28);
	wmove(wnd, 4, 1);
	whline(wnd, '*', 28);
	wmove(wnd, 5, 2);
	wprintw(wnd, "Press ENTER to confirm...");

	wrefresh(wnd);
	
	/* Read user input to fill username */
	keypad(subwnd, true);
	while(1)
	{
		symbol = wgetch(subwnd);
		if ('\n' == symbol)
		{
			break;
		}
		else if (127 > symbol && 32 <= symbol)
		{
			if (T3_LOGIN_Q_MSG_SIZE < index) 
				break;
			wprintw(subwnd, "%c", symbol);
			name[index] = symbol;
			index++;
		}
	}
    /* If no symbols provided -> set user name to "user" */
	if (0 == index)
	{
		snprintf(name, T3_LOGIN_Q_MSG_SIZE, "user");
	}

	wclear(wnd);
	wrefresh(wnd);
	delwin(subwnd);
	delwin(wnd);

	return EXIT_SUCCESS;
}

int third_task_client()
{
	puts("Third task: Client");

    /*
    * Declare:
    * - sev - sigevent to register mq_notify as queue messages handler;
    * - sa - sigaction to redefine SIGINT signal handler;
    * - tmp_buf - temporary buffer for receiving messages from login queue;
    * - symbol - symbol, read by wgetch;
    * - index - current index of symbol in message to be sent to server;
    * - prio - priority of the received message;
    * - ret - return value.
    */
    struct sigevent sev;
    struct sigaction sa;
    char tmp_buf[T3_USER_Q_MSG_SIZE];
	int symbol;
	int index = 0;
	int prio;
	ssize_t ret;

    /* Clear 'tmp_buf' and 'q_handler.buf' */
	memset(tmp_buf, 0, T3_USER_Q_MSG_SIZE);
	memset(q_handler.buf, 0, T3_USER_Q_MSG_SIZE);

    /*
    * Configure sigevent, set notification method, function called, attributes
    * and pointer to q_handler.
    * */
	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_notify_function = handle_msg;
	sev.sigev_notify_attributes = NULL;
	sev.sigev_value.sival_ptr = &q_handler;

    /* Set atexit function*/
	atexit(shutdown_client);

    /* Fill sa_mask, set signal handler, redefine SIGINT with sa */
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = sigint_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

	initscr(); /* Init ncurses lib */

    /* Set signal handler for resizing terminal window */
	signal(SIGWINCH, sig_winch);
	
    /* Get current size of a terminal */
	ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size);
	
	cbreak(); /* Disable line buffering */
	
	noecho(); /* Disable write of user input on a screen */

	curs_set(1); /* Set cursor invisible */
	start_color(); /* */
    /* Init color pair for a usual color palette */
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	bkgd(COLOR_PAIR(1)); /* Set background color to usual color palette */
	refresh(); /* Update stdscr */

    /* Create terminal windows */
	q_handler.wnd = newwin(size.ws_row, size.ws_col, 0, 0);
	q_handler.main_wnd = derwin(q_handler.wnd,
                                (size.ws_row-7),
                                (size.ws_col/4)*3-1,
                                1,
                                1);
	q_handler.users_wnd = derwin(q_handler.wnd, 
                                size.ws_row-2,
                                (size.ws_col/4)-2,
                                1,
                                (size.ws_col/4)*3+1);
	q_handler.msg_wnd = derwin(q_handler.wnd,
                                4,
                                (size.ws_col/4)*3-1,
                                (size.ws_row-5),
                                1);

	box(q_handler.wnd, '|', '-');
	wmove(q_handler.wnd, (size.ws_row-6), 1);
	whline(q_handler.wnd, '-', (size.ws_col/4)*3-1);
	wmove(q_handler.wnd, 1, (size.ws_col/4)*3);
	wvline(q_handler.wnd, '|', size.ws_row-2);

	wrefresh(q_handler.wnd);

    /* Call function to create login window, asking user to input username */
	_login_user(&q_handler.wnd, user.login);

    /* Waiting for server to create login queue */
	wprintw(q_handler.main_wnd, "Please wait the client to establish "
            "connection with server for user '%s'\n", user.login);
	wrefresh(q_handler.main_wnd);
	login_q = mq_open(T3_LOGIN_Q_NAME, O_RDWR);
	while((login_q = mq_open(T3_LOGIN_Q_NAME, O_RDWR)) == -1)
	{
        /*
        * Check if mq_receive returned -1 because any error, but nonexisting
        * queue error.
        */
		if (errno != ENOENT)
		{
			perror("login_q mq_open");
			exit(EXIT_FAILURE);
		}
	}

    /* Try to send message until value > 0 is returned */
	while (mq_send(login_q, "0", T3_LOGIN_Q_MSG_SIZE, T3_USER_PRIO) == -1)
	{
        /*
        * Check if mq_receive returned -1 because any error, but full queue
        * error.
        */
		if (errno != EAGAIN)
		{
			perror("login_q mq_send");
			exit(EXIT_FAILURE);
		}
	}

    /* Register user loop */
	while (1)
	{
		if (mq_receive(login_q, tmp_buf, T3_LOGIN_Q_MSG_SIZE, &prio) == -1)
		{
			perror("mq_receive");
		}
		else
		{
            /*
            * Check if priority of received message is equal of ones, sent by
            * user, if so -> send same message with same priority to queue.
            */
			if (T3_USER_PRIO == prio)
			{
				mq_send(login_q, tmp_buf, T3_LOGIN_Q_MSG_SIZE, prio);
			}
            /*
            * Check if priority of received message is equal of ones, sent by
            * server, registering user.
            */
			else if (T3_SERVER_PRIO == prio)
			{
                /* Convert received 'tmp_buf' to 'user.id' */
				user.id = atoi(tmp_buf);

                /* Set queue file names */
				snprintf(user.cts_ch, T3_LOGIN_Q_MSG_SIZE, T3_CTOS_CH_NAME,
                            user.id);
				snprintf(user.stc_ch, T3_LOGIN_Q_MSG_SIZE, T3_STOC_CH_NAME,
                            user.id);

				wclear(q_handler.main_wnd);
				wprintw(q_handler.main_wnd, "ID has been set to %d\n",
                        user.id);
				wrefresh(q_handler.main_wnd);

                /* Open queues */
				user.rcv_q = mq_open(user.stc_ch, O_RDONLY | O_NONBLOCK);
				if (-1 == user.rcv_q)
				{
					perror("rcv_q mq_open");
					exit(EXIT_FAILURE);
				}
				user.snd_q = mq_open(user.cts_ch, O_WRONLY | O_NONBLOCK);
				if (-1 == user.snd_q)
				{
					perror("snd_q mq_open");
					exit(EXIT_FAILURE);
				}

                /* Register mq_notify */
				q_handler.rcv_q = user.rcv_q;
				if (mq_notify(user.rcv_q, &sev) == -1)
				{
					perror("rcv_q mq_notify");
					exit(EXIT_FAILURE);
				}

                /* Send server command to change user login */
				ret = mq_send(user.snd_q, user.login, T3_LOGIN_Q_MSG_SIZE,
                                T3_SET_USRNAME_PRIO);
				if (ret == -1)
				{
					if (errno != EAGAIN)
					{
						perror("snd_q mq_send");
						exit(EXIT_FAILURE);
					}
				}

                /*
                * Check if queue contains any message before mq_notify was
                * registered.
                */
				while (ret = mq_receive(user.rcv_q, user.rcv_buf, T3_USER_Q_MSG_SIZE,
                                &prio) > 0)
				{
					/*
					* Check if priority of received message is equal of ones, sent by
					* server, resending message to every connected user.
					*/
					if (T3_MSGSND_PRIO == prio)
					{
						wprintw(q_handler.main_wnd, "%s\n", q_handler.buf);
						wrefresh(q_handler.main_wnd);
					}
					/*
					* Check if priority of received message is equal of ones, sent by
					* server, sending userlist.
					*/
					else if (T3_UPD_USRLIST_PRIO == prio)
					{
						wclear(q_handler.users_wnd);
						wprintw(q_handler.users_wnd, "%s", user.rcv_buf);
						wrefresh(q_handler.users_wnd);
						memset(user.rcv_buf, 0, T3_USER_Q_MSG_SIZE);
					}
					/*
					* Check if priority of received message is equal of ones, sent by
					* server, sending disconnect command.
					*/
					else if (T3_DISCONNECT_PRIO == prio)
					{
						exit(EXIT_SUCCESS);
					}
				}
				if (ret == -1)
				{
                    /*
                    * Check if mq_receive returned -1 because any error, but
                    * empty queue error.
                    */
					if (errno != EAGAIN)
					{
						perror("rcv_q mq_receive");
						exit(EXIT_FAILURE);
					}
				}
				break;
			}
		}
	}

    /* Main loop */
	keypad(q_handler.msg_wnd, true);
	while(1)
	{
		symbol = wgetch(q_handler.msg_wnd);
        /* Check, if input symbol is equal ENTER button -> send message */
		if ('\n' == symbol) 
		{
            /* Check if message is empty */
			if (0 == index)
				continue;

            /* Clear message input window */
			wclear(q_handler.msg_wnd);
			wrefresh(q_handler.msg_wnd);

			mq_send(user.snd_q, user.snd_buf, T3_USER_Q_MSG_SIZE,
                    T3_USER_PRIO);

            /* Reset 'index', clear 'user.snd_buf' */
            index = 0;
			memset(user.snd_buf, 0, T3_USER_Q_MSG_SIZE);
		}
        /*
        * Check, if input symbol fits in range of printable characters -> write
        * it to 'user.snd_buf'.
        * */
        else if (symbol >= 32 && symbol < 127)
		{
			if (80 < index) break;
			wprintw(q_handler.msg_wnd, "%c", (char)symbol);
			user.snd_buf[index] = (char)symbol;
			index++;
		}
        /* Check, if input symbol is equal F3 button -> exit from loop */
		else if (KEY_F(3) == symbol)
		{
			break;
		}
	}

	return EXIT_SUCCESS;
}
