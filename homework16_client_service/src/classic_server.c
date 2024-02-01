#include "../hdr/functions.h"

/*
* Declare:
* - clients_counter - semaphore to store number of pending clients;
* - busy_threads - semaphore to store number of threads, processing clients;
* - served_clients - semaphore to store number of clients, served by server;
* - tid - dynamic array of thread ids;
* - clients_q - queue of pending clients;
* - alloc_threads - number of creating threads;
* - alloc_clients - size of 'clients_q'.
*/
sem_t *clients_counter = NULL;
sem_t *busy_threads = NULL;
sem_t *served_clients = NULL;
pthread_t *tid = NULL;
struct tcp_client_t *clients_q = NULL;
int alloc_threads = SERVER_DEF_ALLOC * 10;
int alloc_clients = SERVER_DEF_ALLOC;

/* Signal handler for SIGINT */
static void sigint_handler(int sig, siginfo_t *si, void *unused)
{
    exit(EXIT_SUCCESS);
}

/* Function provided to atexit() */
void shutdown_server(void)
{
	puts("Shutdown server");
	int clients_served;
	int index;

	/* Get number of clients*/
	sem_getvalue(served_clients, &clients_served);

	/* Close sems, delete files */
    sem_close(clients_counter);
	sem_close(busy_threads);
	sem_close(served_clients);
	unlink(SERVER_TCP_COUNTER_SEM_NAME);
	unlink(SERVER_TCP_BUSY_THREADS_SEM_NAME);
	unlink(SERVER_SERVED_CLIENTS_SEM_NAME);

	/* Free allocated memory */
	if (tid != NULL)
    {
        free(tid);
    }
	if (clients_q != NULL)
    {
        free(clients_q);
    }

	printf("Server shutdown: %d clients served\n", clients_served);
}

/* Server thread function */
void *tcp_server_thread(void *args)
{
	int client_fd = 0;
	char msg[MSG_SIZE];
	int index;

	/* Set canceltype so thread could be canceled at any time*/
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	/* Loop to process clients */
	while (1)
	{
		/* Wait for client */
		if (sem_wait(clients_counter) == -1)
		{
			perror("sem_wait");
			break;
		}

		/* Mark this thread as busy */
		if (sem_post(busy_threads) == -1)
		{
			perror("sem_post");
			break;
		}

		/* Go through 'clients_q' to get client's fd */
		for (index = 0; index < alloc_clients; ++index)
		{
			if (clients_q[index].client_fd != NULL)
			{
				if (pthread_mutex_trylock(&(clients_q[index].client_mutex))
					!= 0)
					continue;
				else
				{
					client_fd = clients_q[index].client_fd;
					break;
				}
			}
		}
		
		if (client_fd > 0)
		{
			/* Send message to client */
			strncpy(msg, SERVER_MSG, MSG_SIZE);
			if (send(client_fd, msg, MSG_SIZE, 0) == -1)
			{
				perror("send");
			}
			else
			{
				/* Wait for message from client */
				if (recv(client_fd, msg, MSG_SIZE, 0) == -1)
				{
					if (errno != ECONNRESET)
						perror("recv");
				}
				else
					sem_post(served_clients);
			}
			close(client_fd);
		}

		/* Mark entry in 'clients_q' and thread as free */
		client_fd = 0;
		clients_q[index].client_fd = NULL;
		pthread_mutex_unlock(&(clients_q[index].client_mutex));
		sem_trywait(busy_threads);
	}
}

int multiproto_server(void)
{
	puts("Classic server");

	/*
    * Declare:
	* - sa - sigaction, used to redefine signal handler for SIGINT;
    * - rlim - structure, storing soft and hard limits for maximum number of
    *   file descriptors;
	* - server & client - endpoints of server and client;
	* - server_fd & client_fd - fds of server and client;
	* - client_size - size of client's endpoint;
	* - sem_value - value in semaphores;
	* - index & sec_index - index variables;
    * - tmp_tid - pointer, used to temporary store pointer to reallocated
    *   'tid' array;
	* - tmp_clq - pointer, used to temporary store pointer to reallocated
    *   'clients_q' queue.
    */
	struct sigaction sa;
	struct rlimit rlim;
	struct sockaddr_in server;
	struct sockaddr_in client;
	int server_fd;
	int client_fd;
	int client_size;
	int sem_value = 0;
	int index;
	int sec_index;
	pthread_t *tmp_tid;
	struct tcp_client_t *tmp_clq;

	/* Allocate memory to 'tid' and 'clients_q' */
	tid = malloc(alloc_threads * sizeof(pthread_t));
	clients_q = malloc(alloc_clients * sizeof(struct tcp_client_t));

	/* Init 'clients_q' queue entries */
	for (index = 0; index < alloc_clients; ++index)
	{
		clients_q[index].client_fd = NULL;
		pthread_mutex_init(&(clients_q[index].client_mutex), NULL);
	}

	/* Print current rlimit, set new one */
    if (getrlimit(RLIMIT_NOFILE, &rlim) == -1)
    {
        perror("getrlimit");
        exit(EXIT_FAILURE);
    }
    printf("Current maximum file descriptor: %ld / %ld\n", rlim.rlim_cur,
			rlim.rlim_max);
    rlim.rlim_cur = rlim.rlim_max;
    printf("New maximum file descriptor: %ld / %ld\n", rlim.rlim_cur,
			rlim.rlim_max);
    if (setrlimit(RLIMIT_NOFILE, &rlim) == -1)
    {
        perror("setrlimit");
        exit(EXIT_FAILURE);
    }

	/*
	* Create 'clients_counter', 'busy_threads' and 'served_clients' semaphores.
	*/
	clients_counter = sem_open(SERVER_TCP_COUNTER_SEM_NAME, O_CREAT | O_RDWR, 0666,
								0);
    if (clients_counter == SEM_FAILED)
    {
        perror("sem_open counter_sem");
        exit(EXIT_FAILURE);
    }
	busy_threads = sem_open(SERVER_TCP_BUSY_THREADS_SEM_NAME, O_CREAT | O_RDWR,
							0666, 0);
    if (busy_threads == SEM_FAILED)
    {
        perror("sem_open busy_threads");
        exit(EXIT_FAILURE);
    }
	served_clients = sem_open(SERVER_SERVED_CLIENTS_SEM_NAME, O_CREAT | O_RDWR,
								0666, 0);
    if (served_clients == SEM_FAILED)
    {
        perror("sem_open served_clients");
        exit(EXIT_FAILURE);
    }

	/* Reset these semaphores */
	sem_getvalue(clients_counter, &sem_value);
	while (sem_value > 0)
	{
		sem_trywait(clients_counter);
		sem_getvalue(clients_counter, &sem_value);
	}
	sem_getvalue(busy_threads, &sem_value);
	while (sem_value > 0)
	{
		sem_trywait(busy_threads);
		sem_getvalue(busy_threads, &sem_value);
	}
	sem_getvalue(served_clients, &sem_value);
	while (sem_value > 0)
	{
		sem_trywait(served_clients);
		sem_getvalue(served_clients, &sem_value);
	}

	/* Create threads */
	for (index = 0; index < alloc_threads; ++index)
	{
		pthread_create(&tid[index], NULL, tcp_server_thread, NULL);
	}

	/* Fill 'server' with 0's */
	memset(&server, 0, sizeof(server));

	/* Set server's endpoint */
	server.sin_family = AF_INET;
	if (inet_pton(AF_INET, SERVER_ADDR, &server.sin_addr) == -1)
	{
		perror("inet_pton");
		exit(EXIT_FAILURE);
	}
	server.sin_port = SERVER_TCP_PORT;

	/* Create socket */
	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	/* Allow reuse of local address */
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

	if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		perror("bind");
		exit(EXIT_FAILURE);
	}

	/*
	* TCP protocol only function, mark 'server_fd' socket as passive
	* socket.
	*/
	if (listen(server_fd, SERVER_LISTEN_BACKLOG) == -1)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

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

	while(1)
	{
		/*
		* TCP protocol only function, await connection and get client's
		* endpoint and fd.
		*/
		client_size = sizeof(client);
		if ((client_fd = accept(server_fd, (struct sockaddr *)&client,
								&client_size)) == -1)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}

		/* Write free client_fd entry with new client's fd */
		for (index = 0; index < alloc_clients; ++index)
		{
			if (clients_q[index].client_fd == NULL)
			{
				clients_q[index].client_fd = client_fd;
				break;
			}
		}

		/* If there is no free entries in queue -> allocate more memory */
		if (index == alloc_clients)
		{
			printf("reallocating clients from %d to %d\n", alloc_clients,
					alloc_clients+SERVER_DEF_ALLOC);
			alloc_clients += SERVER_DEF_ALLOC;
            tmp_clq = realloc(clients_q,
								(alloc_clients*sizeof(struct tcp_client_t)));
            if (tmp_clq == NULL)
            {
                perror("realloc");
                break;
            }
			else
            	clients_q  = tmp_clq;

			for (sec_index = alloc_clients-1; 
					sec_index >= alloc_clients-SERVER_DEF_ALLOC; --sec_index)
			{
				printf("initing %dth clients_q(%ld addr)\n", sec_index, 
						&clients_q[sec_index]);
				clients_q[alloc_clients-sec_index].client_fd = NULL;
				pthread_mutex_init(&(clients_q[alloc_clients-sec_index].
										client_mutex), NULL);
			}
		}

		/* Get clients count */
		if (sem_getvalue(busy_threads, &sem_value) == -1)
		{
			perror("sem_getvalue");
			break;
		}

		/*
		* If number of threads, busy with processing clients is almost equal to
		* total number of created threads -> create more threads.
		*/
		if (sem_value > (alloc_threads - SERVER_DEF_ALLOC))
		{
			printf("reallocating threads from %d to %d\n", alloc_threads,
					alloc_threads+SERVER_DEF_ALLOC);
			alloc_threads += SERVER_DEF_ALLOC;
			tmp_tid = realloc(tid, (alloc_threads*sizeof(pthread_t)));
            if (tmp_tid == NULL)
            {
                perror("realloc");
                break;
            }
            tid  = tmp_tid;

			for (index = 0; index < SERVER_DEF_ALLOC; ++index)
			{
				pthread_create(&tid[index], NULL, tcp_server_thread, NULL);
			}
		}
		/*
		* Increment semaphore, so only one thread is allowed
		* to begin processing client.
		*/
		sem_post(clients_counter);
	}

	exit(EXIT_SUCCESS);
}
