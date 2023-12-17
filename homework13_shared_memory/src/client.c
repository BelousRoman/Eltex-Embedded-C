#include "../hdr/functions.h"

/*
* Declare global variables:
* - size - current size of the terminal, changed in sig_winch();
* - login - variable, containing shm segment and semaphores, used in logging
*   sequence;
* - user - variable, containing all the info about user, its id, login, shared
*   memory segment and semaphores.
*/
struct winsize size;
struct login_t login = {
	.stc_sem = NULL,
	.cts_sem = NULL,
	.srv_sem = NULL,
	.usr_sem = NULL,
	.shm_buf = NULL
};
struct user_t user = {
	.stc_sem = NULL,
	.cts_sem = NULL,
	.srv_sem = NULL,
	.usr_sem = NULL,
	.shm_buf = NULL
};
struct windows_t windows;

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

/* Atexit function, freeing all the used resources */
void shutdown_client(void)
{
    /* Delete windows, deinit screen */
    delwin(windows.main_wnd);
    delwin(windows.users_wnd);
    delwin(windows.msg_wnd);
    delwin(windows.wnd);
    endwin();

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

	/* Free user shm and sems, if used */
	if (user.shm_buf != NULL)
    {
        munmap(user.shm_buf, sizeof(struct usr_shm_t));
    }
	if (user.stc_sem != NULL)
	{
		sem_close(user.stc_sem);
	}
    if (user.cts_sem != NULL)
	{
		sem_close(user.cts_sem);
	}
    if (user.srv_sem != NULL)
	{
		sem_close(user.srv_sem);
	}
	if (user.usr_sem != NULL)
	{
		sem_close(user.usr_sem);
	}

	puts("Client shutdown");
}

/* Function to handle message when new message in queue occurs */
static void *msg_handler(void *args)
{
	/* Set canceltype so thread could be canceled at any time*/
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	while(1)
	{
		if (sem_trywait(user.stc_sem) == 0)
		{
			switch (user.shm_buf->stoc.comm)
			{
			case T3_MSGSND_COMM:
				wprintw(windows.main_wnd, "%s\n", user.shm_buf->stoc.msg);
				wrefresh(windows.main_wnd);
				sem_post(user.srv_sem);
				break;
			case T3_UPD_USRLIST_COMM:
				wclear(windows.users_wnd);
				wprintw(windows.users_wnd, "%s", user.shm_buf->stoc.msg);
				wrefresh(windows.users_wnd);
				sem_post(user.srv_sem);
				break;
			case T3_DISCONNECT_COMM:
				exit(EXIT_SUCCESS);
				break;
			default:
				break;
			}
		}
	}
}

/* Function, creating subwindow, asking user to input username */
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
			if (T3_LOGIN_MSG_SIZE < index)
				break;
			wprintw(subwnd, "%c", symbol);
			name[index] = symbol;
			index++;
		}
	}
    /* If no symbols provided -> set user name to "user" */
	if (0 == index)
	{
		// snprintf(name, T3_LOGIN_MSG_SIZE, "user");
		strncpy(name, "user", T3_LOGIN_MSG_SIZE);
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
    // struct sigevent sev;
    struct sigaction sa;
	// char *login_stoc_buf;
	// char *login_ctos_buf;
    char tmp_buf[T3_USER_MSG_SIZE];
	int symbol;
	int index = 0;
	// int prio;
	ssize_t ret;
	pthread_t tid;

    /* Clear 'tmp_buf' */
	memset(tmp_buf, 0, T3_USER_MSG_SIZE);

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
	start_color(); /* Enable color support */
    /* Init color pair for a usual color palette */
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	bkgd(COLOR_PAIR(1)); /* Set background color to usual color palette */
	refresh(); /* Update stdscr */

    /* Create terminal windows */
	windows.wnd = newwin(size.ws_row, size.ws_col, 0, 0);
	windows.main_wnd = derwin(windows.wnd,
                                (size.ws_row-7),
                                (size.ws_col/4)*3-1,
                                1,
                                1);
	windows.users_wnd = derwin(windows.wnd, 
                                size.ws_row-2,
                                (size.ws_col/4)-2,
                                1,
                                (size.ws_col/4)*3+1);
	windows.msg_wnd = derwin(windows.wnd,
                                4,
                                (size.ws_col/4)*3-1,
                                (size.ws_row-5),
                                1);

	box(windows.wnd, '|', '-');
	wmove(windows.wnd, (size.ws_row-6), 1);
	whline(windows.wnd, '-', (size.ws_col/4)*3-1);
	wmove(windows.wnd, 1, (size.ws_col/4)*3);
	wvline(windows.wnd, '|', size.ws_row-2);

	wrefresh(windows.wnd);

    /* Call function to create login window, asking user to input username */
	_login_user(&windows.wnd, user.login);

    /* Wait for server to create login queue */
	wprintw(windows.main_wnd, "Please wait the client to establish "
            "connection with server for user '%s'\n", user.login);
	wrefresh(windows.main_wnd);

	/* Lock until server create login shared memory and semaphores segments */
	while ((login.shm_fd = shm_open(T3_LOGIN_SHM_NAME, O_RDWR, NULL)) == -1)
	{
		if (errno != ENOENT)
		{
			perror("shm_open");
			exit(EXIT_FAILURE);
		}
	}
	if ((login.shm_buf = mmap(NULL, sizeof(struct log_msg_t), PROT_READ |
							PROT_WRITE, MAP_SHARED, login.shm_fd, 0)) == -1)
	{
		perror("log_msg mmap");
		exit(EXIT_FAILURE);
	}
	while ((login.stc_sem = sem_open(T3_LOGIN_STOC_SEM_NAME, O_RDWR))
			== SEM_FAILED)
	{
		if (errno != ENOENT)
		{
			perror("login_stoc_sem sem_open");
			exit(EXIT_FAILURE);
		}
	}
	while ((login.cts_sem = sem_open(T3_LOGIN_CTOS_SEM_NAME, O_RDWR))
			== SEM_FAILED)
	{
		if (errno != ENOENT)
		{
			perror("login_ctos_sem sem_open");
			exit(EXIT_FAILURE);
		}
	}
	while ((login.srv_sem = sem_open(T3_LOGIN_LOG_SRV_SEM_NAME, O_RDWR))
			== SEM_FAILED)
	{
		if (errno != ENOENT)
		{
			perror("login_log_srv_sem sem_open");
			exit(EXIT_FAILURE);
		}
	}
	while ((login.usr_sem = sem_open(T3_LOGIN_LOG_USR_SEM_NAME, O_RDWR))
			== SEM_FAILED)
	{
		if (errno != ENOENT)
		{
			perror("login_log_usr_sem sem_open");
			exit(EXIT_FAILURE);
		}
	}
	
	/* Try to lock semaphore to request register sequence from server */
	sem_wait(login.usr_sem);

	/* Send username to server, unlock client-to-server semaphore */
	strncpy(login.shm_buf->ctos, user.login, T3_LOGIN_MSG_SIZE);
	sem_post(login.cts_sem);

	/* Wait until server processed username and registered user*/
	sem_wait(login.stc_sem);

	/* Get ID from server, use it to get names of shm segment and semaphores */
	user.id = atoi(login.shm_buf->stoc);
	snprintf(user.shm_name, T3_LOGIN_MSG_SIZE,
        T3_USER_SHM_NAME, user.id);
    snprintf(user.stc_sem_name, T3_LOGIN_MSG_SIZE,
        T3_USER_STOC_SEM_NAME, user.id);
    snprintf(user.cts_sem_name, T3_LOGIN_MSG_SIZE,
        T3_USER_CTOS_SEM_NAME, user.id);
    snprintf(user.srv_sem_name, T3_LOGIN_MSG_SIZE,
        T3_USER_SRV_SEM_NAME, user.id);
    snprintf(user.usr_sem_name, T3_LOGIN_MSG_SIZE,
        T3_USER_USR_SEM_NAME, user.id);

	/* Lock until server create user's shared memory and semaphores segments */
	while ((user.usr_shm = shm_open(user.shm_name, O_RDWR, NULL)) == -1)
	{
		if (errno != ENOENT)
		{
			perror("usr_shm shm_open");
			exit(EXIT_FAILURE);
		}
	}
	if ((user.shm_buf = mmap(NULL, sizeof(struct usr_shm_t), PROT_READ | PROT_WRITE, MAP_SHARED,
					        user.usr_shm, 0)) == -1)
	{
		perror("shm_buf mmap");
		exit(EXIT_FAILURE);
	}
	while ((user.stc_sem = sem_open(user.stc_sem_name, O_RDWR)) == SEM_FAILED)
	{
		if (errno != ENOENT)
		{
			perror("stc_sem sem_open");
			exit(EXIT_FAILURE);
		}
	}
	while ((user.cts_sem = sem_open(user.cts_sem_name, O_RDWR)) == SEM_FAILED)
	{
		if (errno != ENOENT)
		{
			perror("cts_sem sem_open");
			exit(EXIT_FAILURE);
		}
	}
	while ((user.srv_sem = sem_open(user.srv_sem_name, O_RDWR)) == SEM_FAILED)
	{
		if (errno != ENOENT)
		{
			perror("srv_sem sem_open");
			exit(EXIT_FAILURE);
		}
	}
	while ((user.usr_sem = sem_open(user.usr_sem_name, O_RDWR)) == SEM_FAILED)
	{
		if (errno != ENOENT)
		{
			perror("usr_sem sem_open");
			exit(EXIT_FAILURE);
		}
	}

	wprintw(windows.main_wnd, "Registered with ID: %d\n", user.id);
	wrefresh(windows.main_wnd);

	/* Unlock 'srv_sem' to sync server */
	sem_post(login.srv_sem);

	/* Free resources, used to login */
	munmap(login.shm_buf, sizeof(struct log_msg_t));
	sem_close(login.stc_sem);
	sem_close(login.cts_sem);
	sem_close(login.srv_sem);
	sem_close(login.usr_sem);

	/* Create message handler thread for dynamic messages handling */
	pthread_create(&tid, NULL, msg_handler, NULL);

    /* Main loop */
	keypad(windows.msg_wnd, true);
	while(1)
	{
		symbol = wgetch(windows.msg_wnd);
        /* Check, if input symbol is equal ENTER button -> send message */
		if ('\n' == symbol) 
		{
            /* Check if message is empty */
			if (0 == index)
				continue;

            /* Clear message input window */
			wclear(windows.msg_wnd);
			wrefresh(windows.msg_wnd);

			user.shm_buf->ctos.comm = T3_MSGSND_COMM;
			strncpy(user.shm_buf->ctos.msg, tmp_buf, T3_USER_MSG_SIZE);

			sem_post(user.cts_sem);
			sem_wait(user.usr_sem);

            /* Reset 'index', clear 'tmp_buf' */
            index = 0;
			memset(tmp_buf, 0, T3_USER_MSG_SIZE);
		}
        /*
        * Check, if input symbol fits in range of printable characters -> write
        * it to 'tmp_buf'.
        * */
        else if (symbol >= 32 && symbol < 127)
		{
			if (80 < index) break;
			wprintw(windows.msg_wnd, "%c", (char)symbol);
			tmp_buf[index] = (char)symbol;
			index++;
		}
        /* Check, if input symbol is equal F3 button -> exit from loop */
		else if (KEY_F(3) == symbol)
		{
			break;
		}
	}

	user.shm_buf->ctos.comm = T3_DISCONNECT_COMM;
	sem_post(user.cts_sem);

	pthread_cancel(tid);

	return EXIT_SUCCESS;
}
