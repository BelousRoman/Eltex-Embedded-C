#include "../hdr/functions.h"

/*
* Declare:
* - tcp_clients_counter & udp_clients_counter - semaphores to store number of pending clients for tcp and udp servers;
* - tcp_busy_threads & udp_busy_threads - semaphore to store number of threads, processing clients;
* - served_clients - semaphore to store number of clients, served by server;
* - tcp_server_fd & udp_server_fd & client_fd - fds of server and client;
* - tid - dynamic array of thread ids;
* - tcp_clients_q - queue of pending clients;
* - tcp_alloc_threads - number of creating threads;
* - tcp_alloc_clients - size of 'tcp_clients_q'.
*/
sem_t *tcp_clients_counter = NULL;
sem_t *udp_clients_counter = NULL;
sem_t *tcp_busy_threads = NULL;
sem_t *udp_busy_threads = NULL;
sem_t *served_clients = NULL;
int tcp_server_fd;
int udp_server_fd;
pthread_t *tcp_tids = NULL;
pthread_t *udp_tids = NULL;
struct tcp_client_t *tcp_clients_q = NULL;
struct udp_client_t *udp_clients_q = NULL;
struct pollfd *pfds = NULL;
int tcp_alloc_threads = SERVER_DEF_ALLOC * 10;
int udp_alloc_threads = SERVER_DEF_ALLOC * 10;
int tcp_alloc_clients = SERVER_DEF_ALLOC;
int udp_alloc_clients = SERVER_DEF_ALLOC;

/* Signal handler for SIGINT */
static void sigint_handler(int sig, siginfo_t *si, void *unused)
{
    exit(EXIT_SUCCESS);
}

/* Function provided to atexit() */
void shutdown_server(void)
{
	int clients_served;
	int index;

	/* Get number of clients*/
	sem_getvalue(served_clients, &clients_served);

	/* Close sems, delete files */
    sem_close(tcp_clients_counter);
	sem_close(tcp_busy_threads);
	sem_close(served_clients);
	unlink(SERVER_TCP_COUNTER_SEM_NAME);
	unlink(SERVER_TCP_BUSY_THREADS_SEM_NAME);
	unlink(SERVER_SERVED_CLIENTS_SEM_NAME);

	/* Free allocated memory */
	if (tcp_tids != NULL)
    {
        free(tcp_tids);
    }
	if (tcp_clients_q != NULL)
    {
        free(tcp_clients_q);
    }
	if (pfds != NULL)
    {
        free(pfds);
    }

	printf("Server shutdown: %d clients served\n", clients_served);
}

/* TCP server thread function */
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
		if (sem_wait(tcp_clients_counter) == -1)
		{
			perror("sem_wait");
			break;
		}

		/* Mark this thread as busy */
		if (sem_post(tcp_busy_threads) == -1)
		{
			perror("sem_post");
			break;
		}

		/* Go through 'tcp_clients_q' to get client's fd */
		for (index = 0; index < tcp_alloc_clients; ++index)
		{
			if (tcp_clients_q[index].client_fd != NULL)
			{
				if (pthread_mutex_trylock(&(tcp_clients_q[index].client_mutex))
					!= 0)
					continue;
				else
				{
					client_fd = tcp_clients_q[index].client_fd;
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
					perror("recv");
				}
				else
					sem_post(served_clients);
			}
			close(client_fd);
		}

		/* Mark entry in 'tcp_clients_q' and thread as free */
		client_fd = 0;
		tcp_clients_q[index].client_fd = NULL;
		pthread_mutex_unlock(&(tcp_clients_q[index].client_mutex));
		sem_trywait(tcp_busy_threads);
	}
}

/* UDP server thread function */
void *udp_server_thread(void *args)
{
	struct sockaddr_in client;
	memset(&client, NULL, sizeof(client));
	int client_size = 0;
	char msg[MSG_SIZE];
	int index;

	/* Set canceltype so thread could be canceled at any time*/
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	/* Loop to process clients */
	while (1)
	{
		/* Wait for client */
		if (sem_wait(udp_clients_counter) == -1)
		{
			perror("sem_wait");
			break;
		}

		/* Mark this thread as busy */
		if (sem_post(udp_busy_threads) == -1)
		{
			perror("sem_post");
			break;
		}

		/* Go through 'tcp_clients_q' to get client's endpoint */
		for (index = 0; index < udp_alloc_clients; ++index)
		{
			if (udp_clients_q[index].client.sin_zero != NULL)
			{
				if (pthread_mutex_trylock(&(udp_clients_q[index].client_mutex))
					!= 0)
					continue;
				else
				{
					client = udp_clients_q[index].client;
					break;
				}
			}
		}

		if (client.sin_zero != NULL)
		{
			/* Send message to client */
			strncpy(msg, SERVER_MSG, MSG_SIZE);
			client_size = sizeof(client);
			if (recvfrom(udp_server_fd, msg, MSG_SIZE, 0, (struct sockaddr *)&client, &client_size) == -1)
			{
				perror("recvfrom");
			}
			else
			{
				/* Wait for message from client */
				if (sendto(udp_server_fd, msg, MSG_SIZE, MSG_DONTWAIT,
					(struct sockaddr *)&client, client_size) == -1)
				{
					perror("sendto");
				}
				else
					sem_post(served_clients);
			}
		}

		/* Mark entry in 'tcp_clients_q' and thread as free */
		memset(&client, NULL, sizeof(client));
		udp_clients_q[index].client = client;
		pthread_mutex_unlock(&(udp_clients_q[index].client_mutex));
		sem_trywait(udp_busy_threads);
	}
}

int multiproto_server(void)
{
	puts("Multiprotocol server");

	/*
    * Declare:
	* - sa - sigaction, used to redefine signal handler for SIGINT;
    * - rlim - structure, storing soft and hard limits for maximum number of
    *   file descriptors;
	* - tcp_server & udp_server & client - endpoints of server and client;
	* - client_fd - fd of TCP server's client;
	* - client_size - size of client's endpoint;
	* - sem_value - value in semaphores;
	* - index & sec_index - index variables;
    * - tmp_tid - pointer, used to temporary store pointer to reallocated
    *   'tcp_tids' & 'udp_tids' arrays;
	* - tmp_clq - pointer, used to temporary store pointer to reallocated
    *   'tcp_clients_q' & 'udp_clients_q' queues.
    */
	struct sigaction sa;
	struct rlimit rlim;
	struct sockaddr_in tcp_server;
	struct sockaddr_in udp_server;
	struct sockaddr_in client;
	int client_fd;
	int client_size;
	int sem_value = 0;
	int ret;
	int index;
	int sec_index;
	pthread_t *tmp_tid;
	struct tcp_client_t *tmp_clq;

	/* Allocate memory to 'tcp_tids' and 'tcp_clients_q' */
	tcp_tids = malloc(tcp_alloc_threads * sizeof(pthread_t));
	udp_tids = malloc(udp_alloc_threads * sizeof(pthread_t));
	tcp_clients_q = malloc(tcp_alloc_clients * sizeof(struct tcp_client_t));
	udp_clients_q = malloc(udp_alloc_clients * sizeof(struct tcp_client_t));
	pfds = malloc(2 * sizeof(struct pollfd));

	/* Init 'tcp_clients_q' & 'udp_clients_q' queue entries */
	for (index = 0; index < tcp_alloc_clients; ++index)
	{
		// printf("initing %dth tcp_clients_q\n", index);
		tcp_clients_q[index].client_fd = NULL;
		pthread_mutex_init(&(tcp_clients_q[index].client_mutex), NULL);
	}
	for (index = 0; index < udp_alloc_clients; ++index)
	{
		// printf("initing %dth udp_clients_q\n", index);
		//  udp_clients_q[index].client = NULL;
		memset(&udp_clients_q[index].client, NULL, sizeof(udp_clients_q[index].client));
		pthread_mutex_init(&(udp_clients_q[index].client_mutex), NULL);
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
	* Create 'tcp_clients_counter', 'udp_clients_counter', 'tcp_busy_threads', 'udp_busy_threads' and 'served_clients' semaphores.
	*/
	tcp_clients_counter = sem_open(SERVER_TCP_COUNTER_SEM_NAME, O_CREAT | O_RDWR, 0666,
								0);
    if (tcp_clients_counter == SEM_FAILED)
    {
        perror("sem_open tcp_clients_counter");
        exit(EXIT_FAILURE);
    }
	udp_clients_counter = sem_open(SERVER_UDP_COUNTER_SEM_NAME, O_CREAT | O_RDWR, 0666,
								0);
    if (udp_clients_counter == SEM_FAILED)
    {
        perror("sem_open udp_clients_counter");
        exit(EXIT_FAILURE);
    }
	tcp_busy_threads = sem_open(SERVER_TCP_BUSY_THREADS_SEM_NAME, O_CREAT | O_RDWR,
							0666, 0);
    if (tcp_busy_threads == SEM_FAILED)
    {
        perror("sem_open tcp_busy_threads");
        exit(EXIT_FAILURE);
    }
	udp_busy_threads = sem_open(SERVER_UDP_BUSY_THREADS_SEM_NAME, O_CREAT | O_RDWR,
							0666, 0);
    if (udp_busy_threads == SEM_FAILED)
    {
        perror("sem_open tcp_busy_threads");
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
	sem_getvalue(tcp_clients_counter, &sem_value);
	while (sem_value > 0)
	{
		sem_trywait(tcp_clients_counter);
		sem_getvalue(tcp_clients_counter, &sem_value);
	}
	sem_getvalue(udp_clients_counter, &sem_value);
	while (sem_value > 0)
	{
		sem_trywait(udp_clients_counter);
		sem_getvalue(udp_clients_counter, &sem_value);
	}
	sem_getvalue(tcp_busy_threads, &sem_value);
	while (sem_value > 0)
	{
		sem_trywait(tcp_busy_threads);
		sem_getvalue(tcp_busy_threads, &sem_value);
	}
	sem_getvalue(udp_busy_threads, &sem_value);
	while (sem_value > 0)
	{
		sem_trywait(udp_busy_threads);
		sem_getvalue(udp_busy_threads, &sem_value);
	}
	sem_getvalue(served_clients, &sem_value);
	while (sem_value > 0)
	{
		sem_trywait(served_clients);
		sem_getvalue(served_clients, &sem_value);
	}

	/* Create threads */
	for (index = 0; index < tcp_alloc_threads; ++index)
	{
		pthread_create(&tcp_tids[index], NULL, tcp_server_thread, NULL);
	}
	for (index = 0; index < udp_alloc_threads; ++index)
	{
		pthread_create(&udp_tids[index], NULL, udp_server_thread, NULL);
	}

	/* Fill 'tcp_server' & 'udp_server' with 0's */
	memset(&tcp_server, 0, sizeof(tcp_server));
	memset(&udp_server, 0, sizeof(udp_server));

	/* Set server's endpoints */
	tcp_server.sin_family = AF_INET;
	if (inet_pton(AF_INET, SERVER_ADDR, &tcp_server.sin_addr) == -1)
	{
		perror("inet_pton tcp_server");
		exit(EXIT_FAILURE);
	}
	tcp_server.sin_port = SERVER_TCP_PORT;

	udp_server.sin_family = AF_INET;
	if (inet_pton(AF_INET, SERVER_ADDR, &udp_server.sin_addr) == -1)
	{
		perror("inet_pton udp_server");
		exit(EXIT_FAILURE);
	}
	udp_server.sin_port = SERVER_UDP_PORT;

	/* Create sockets */
	tcp_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	udp_server_fd = socket(AF_INET, SOCK_DGRAM, 0);

	/* Allow reuse of local address in sockets */
	setsockopt(tcp_server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	setsockopt(udp_server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

	/* Assign addresses to sockets */
	if (bind(tcp_server_fd, (struct sockaddr *)&tcp_server, sizeof(tcp_server)) == -1)
	{
		perror("bind tcp_server");
		exit(EXIT_FAILURE);
	}
	if (bind(udp_server_fd, (struct sockaddr *)&udp_server, sizeof(udp_server)) == -1)
	{
		perror("bind udp_server");
		exit(EXIT_FAILURE);
	}

	/*
	* TCP protocol only function, mark 'tcp_server_fd' socket as passive
	* socket.
	*/
	if (listen(tcp_server_fd, SERVER_LISTEN_BACKLOG) == -1)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	pfds[0].fd = tcp_server_fd;
	pfds[1].fd = udp_server_fd;
	pfds[0].events = POLLIN;
	pfds[1].events = POLLIN;

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
		int ret = poll(pfds, 2, 0);
		if (ret > 0)
		{
			if (pfds[0].revents & POLLIN)
			{
				/*
				* TCP protocol only function, await connection and get client's
				* endpoint and fd.
				*/
				client_size = sizeof(client);
				if ((client_fd = accept(tcp_server_fd, (struct sockaddr *)&client,
										&client_size)) == -1)
				{
					perror("accept");
					exit(EXIT_FAILURE);
				}

				/* Write free client_fd entry with new client's fd */
				for (index = 0; index < tcp_alloc_clients; ++index)
				{
					if (tcp_clients_q[index].client_fd == NULL)
					{
						tcp_clients_q[index].client_fd = client_fd;
						break;
					}
				}

				/* If there is no free entries in queue -> allocate more memory */
				if (index == tcp_alloc_clients)
				{
					printf("reallocating clients from %d to %d\n", tcp_alloc_clients,
							tcp_alloc_clients+SERVER_DEF_ALLOC);
					tcp_alloc_clients += SERVER_DEF_ALLOC;
					tmp_clq = realloc(tcp_clients_q,
										(tcp_alloc_clients*sizeof(struct tcp_client_t)));
					if (tmp_clq == NULL)
					{
						perror("realloc");
						break;
					}
					else
						tcp_clients_q  = tmp_clq;

					for (sec_index = tcp_alloc_clients-1; 
							sec_index >= tcp_alloc_clients-SERVER_DEF_ALLOC; --sec_index)
					{
						printf("initing %dth tcp_clients_q(%ld addr)\n", sec_index, 
								&tcp_clients_q[sec_index]);
						tcp_clients_q[tcp_alloc_clients-sec_index].client_fd = NULL;
						pthread_mutex_init(&(tcp_clients_q[tcp_alloc_clients-sec_index].
												client_mutex), NULL);
					}
				}

				/* Get clients count */
				if (sem_getvalue(tcp_busy_threads, &sem_value) == -1)
				{
					perror("sem_getvalue");
					break;
				}

				/*
				* If number of threads, busy with processing clients is almost equal to
				* total number of created threads -> create more threads.
				*/
				if (sem_value > (tcp_alloc_threads - SERVER_DEF_ALLOC))
				{
					printf("reallocating threads from %d to %d\n", tcp_alloc_threads,
							tcp_alloc_threads+SERVER_DEF_ALLOC);
					tcp_alloc_threads += SERVER_DEF_ALLOC;
					tmp_tid = realloc(tcp_tids, (tcp_alloc_threads*sizeof(pthread_t)));
					if (tmp_tid == NULL)
					{
						perror("realloc");
						break;
					}
					tcp_tids  = tmp_tid;

					for (index = 0; index < SERVER_DEF_ALLOC; ++index)
					{
						pthread_create(&tcp_tids[index+(tcp_alloc_threads-SERVER_DEF_ALLOC)], NULL, tcp_server_thread, NULL);
					}
				}
				/*
				* Increment semaphore, so only one thread is allowed
				* to begin processing client.
				*/
				sem_post(tcp_clients_counter);
				if (ret == 1)
					continue;
			}
			if (pfds[1].revents & POLLIN)
			{
				client_size = sizeof(client);
				recvfrom(udp_server_fd, NULL, NULL, 0, (struct sockaddr *)&client, &client_size);

				/* Write free client_fd entry with new client's fd */
				for (index = 0; index < udp_alloc_clients; ++index)
				{
					if (udp_clients_q[index].client.sin_zero == NULL)
					{
						udp_clients_q[index].client = client;
						break;
					}
				}

				/* If there is no free entries in queue -> allocate more memory */
				if (index == udp_alloc_clients)
				{
					printf("reallocating clients from %d to %d\n", udp_alloc_clients,
							udp_alloc_clients+SERVER_DEF_ALLOC);
					udp_alloc_clients += SERVER_DEF_ALLOC;
					tmp_clq = realloc(udp_clients_q,
										(udp_alloc_clients*sizeof(struct udp_client_t)));
					if (tmp_clq == NULL)
					{
						perror("realloc");
						break;
					}
					else
						udp_clients_q  = tmp_clq;

					for (sec_index = udp_alloc_clients-1; 
							sec_index >= udp_alloc_clients-SERVER_DEF_ALLOC; --sec_index)
					{
						printf("initing %dth udp_clients_q(%ld addr)\n", sec_index, 
								&udp_clients_q[sec_index]);
						// udp_clients_q[udp_alloc_clients-sec_index].client = NULL;
						memset(&udp_clients_q[udp_alloc_clients-sec_index].client, NULL, sizeof(udp_clients_q[udp_alloc_clients-sec_index].client));
						pthread_mutex_init(&(udp_clients_q[udp_alloc_clients-sec_index].
												client_mutex), NULL);
					}
				}

				/* Get clients count */
				if (sem_getvalue(udp_busy_threads, &sem_value) == -1)
				{
					perror("sem_getvalue");
					break;
				}

				/*
				* If number of threads, busy with processing clients is almost equal to
				* total number of created threads -> create more threads.
				*/
				if (sem_value > (udp_alloc_threads - SERVER_DEF_ALLOC))
				{
					printf("reallocating threads from %d to %d\n", udp_alloc_threads,
							udp_alloc_threads+SERVER_DEF_ALLOC);
					udp_alloc_threads += SERVER_DEF_ALLOC;
					tmp_tid = realloc(udp_tids, (udp_alloc_threads*sizeof(pthread_t)));
					if (tmp_tid == NULL)
					{
						perror("realloc");
						break;
					}
					udp_tids  = tmp_tid;

					for (index = 0; index < SERVER_DEF_ALLOC; ++index)
					{
						pthread_create(&udp_tids[index+(udp_alloc_threads-SERVER_DEF_ALLOC)], NULL, udp_server_thread, NULL);
					}
				}
				/*
				* Increment semaphore, so only one thread is allowed
				* to begin processing client.
				*/
				sem_post(udp_clients_counter);
				if (ret == 1)
					continue;
			}
		}
	}

	return EXIT_SUCCESS;
}
