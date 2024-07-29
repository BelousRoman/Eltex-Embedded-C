#include "../hdr/functions.h"

/*
* Declare global variables:
* - tcp_server_fd & udp_server_fd - server fds;
* - free_tcp_threads_sem & free_udp_threads_sem - semaphores, counting 
*	currently free threads;
* - busy_tcp_threads_sem & busy_udp_threads_sem - semaphores, counting 
*	currently busy threads;
* - served_tcp_clients & served_udp_clients - semaphores, counting number of 
*	served clients;
* - tcp_threads & udp_threads - dynamic arrays, containing threads 
*	ids (for TCP and UDP respectively);
* - tcp_threads_count & udp_threads_count - variables, counting number on
* - elements in above-mentioned arrays.
*/
int tcp_server_fd;
int udp_server_fd;
sem_t *free_tcp_threads_sem = NULL;
sem_t *free_udp_threads_sem = NULL;
sem_t *busy_tcp_threads_sem = NULL;
sem_t *busy_udp_threads_sem = NULL;
sem_t *served_tcp_clients = NULL;
sem_t *served_udp_clients = NULL;

pthread_t *tcp_threads = NULL;
pthread_t *udp_threads = NULL;
int tcp_threads_count = 0;
int udp_threads_count = 0;

/* Handler function for SIGINT signal */
static void sigint_handler(int sig, siginfo_t *si, void *unused)
{
    exit(EXIT_SUCCESS);
}

/* Atexit function */
void shutdown_server(void)
{
	int tcp_clients_count = 0;
	int udp_clients_count = 0;
	int index;

	/* Get number of served clients */
	if (served_tcp_clients != NULL)
		sem_getvalue(served_tcp_clients, &tcp_clients_count);

	if (served_udp_clients != NULL)
		sem_getvalue(served_udp_clients, &udp_clients_count);

	/* Close sockets */
	if (tcp_server_fd > 0)
		close(tcp_server_fd);
	if (udp_server_fd > 0)
		close(udp_server_fd);

	/* Cancel every created thread, close fd, if possible, free memory */
    if (tcp_threads != NULL)
    {
        for (index = 0; index < tcp_threads_count; index++)
        {
            pthread_cancel(tcp_threads[index]);
        }

        free(tcp_threads);
    }
	if (udp_threads != NULL)
    {
        for (index = 0; index < udp_threads_count; index++)
        {
            pthread_cancel(udp_threads[index]);
        }

        free(udp_threads);
    }

	/* Close semaphore fds, delete files */
	if (free_tcp_threads_sem != NULL)
		sem_close(free_tcp_threads_sem);
	if (free_udp_threads_sem != NULL)
		sem_close(free_udp_threads_sem);
	if (busy_tcp_threads_sem != NULL)
		sem_close(busy_tcp_threads_sem);
	if (busy_udp_threads_sem != NULL)
		sem_close(busy_udp_threads_sem);
	if (served_tcp_clients != NULL)
		sem_close(served_tcp_clients);
	if (served_udp_clients != NULL)
		sem_close(served_udp_clients);

	unlink(SERVER_TCP_QUEUE_NAME);
	unlink(SERVER_UDP_QUEUE_NAME);
    unlink(SERVER_TCP_FREE_THREADS_SEM_NAME);
	unlink(SERVER_UDP_FREE_THREADS_SEM_NAME);
    unlink(SERVER_TCP_BUSY_THREADS_SEM_NAME);
    unlink(SERVER_UDP_BUSY_THREADS_SEM_NAME);
	unlink(SERVER_SERVED_TCP_CLIENTS_SEM_NAME);
    unlink(SERVER_SERVED_UDP_CLIENTS_SEM_NAME);

	printf("Shut server at:\n* %d client(s) (%d by TCP and %d by UDP)\n", 
			(tcp_clients_count+udp_clients_count), 
			tcp_clients_count, 
			udp_clients_count);
}

/* TCP Server thread function */
void *_tcp_server_thread(void *args)
{
	/*
    * Declare:
	* - free_tcp_threads_sem - semaphore, counting currently free threads;
	* - busy_tcp_threads_sem - semaphore, counting currently busy threads;
	* - clients_count_sem - semaphore, counting number of served clients;
	* - tcp_fds_q - fds message queue;
    * - fd - socket's fd;
    * - msg - message buffer.
    */
   	sem_t *free_tcp_threads_sem;
    sem_t *busy_tcp_threads_sem;
    sem_t *clients_count_sem;
	mqd_t tcp_fds_q;
   	int fd = 0;
	char msg[SERVER_MSG_SIZE];

	/* Open 'tcp_fds_q' */
    tcp_fds_q = mq_open(SERVER_TCP_QUEUE_NAME, O_RDONLY);
    if (tcp_fds_q == -1)
    {
        printf("tcp mq_open: %s (%d)\n", strerror(errno), errno);
    }

	/* Set canceltype so thread could be canceled at any time*/
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    /* Wait for parent to create sem segments, then get it's fds */
	while ((free_tcp_threads_sem = sem_open(SERVER_TCP_FREE_THREADS_SEM_NAME, 
			O_RDWR)) == SEM_FAILED)
	{
		if (errno != ENOENT)
		{
			printf("free_tcp_threads_sem sem_open: %s (%d)\n", strerror(errno), 
					errno);
			exit(EXIT_FAILURE);
		}
	}
	while ((busy_tcp_threads_sem = sem_open(SERVER_TCP_BUSY_THREADS_SEM_NAME, 
			O_RDWR)) == SEM_FAILED)
	{
		if (errno != ENOENT)
		{
			printf("busy_tcp_threads_sem sem_open: %s (%d)\n", strerror(errno), 
					errno);
			exit(EXIT_FAILURE);
		}
	}
    while ((clients_count_sem = sem_open(SERVER_SERVED_TCP_CLIENTS_SEM_NAME, 
			O_RDWR)) == SEM_FAILED)
	{
		if (errno != ENOENT)
		{
			printf("clients_count_sem sem_open: %s (%d)\n", strerror(errno), 
					errno);
			exit(EXIT_FAILURE);
		}
	}

	/* Increment counter of free threads */
    sem_post(free_tcp_threads_sem);

    while(1)
    {
		/* Wait for fd from main thread */
        if (mq_receive(tcp_fds_q, &fd, sizeof(int), NULL) == -1)
        {
			printf("mq_receive: %s (%d)\n", strerror(errno), errno);
            continue;
        }

		/* Decrement free threads counter, increment busy threads counter */
        sem_wait(free_tcp_threads_sem);
        sem_post(busy_tcp_threads_sem);

		/* Receive a message from client */
        if (recv(fd, &msg, sizeof(msg), 0) == -1)
        {
			printf("tcp recv: %s (%d)\n", strerror(errno), errno);
			if (errno == ECONNRESET)
				exit(EXIT_FAILURE);
        }

		/* Close client's socket */
		close(fd);

		/* Decrement busy threads counter, increment free threads counter */
        sem_wait(busy_tcp_threads_sem);
        sem_post(free_tcp_threads_sem);

		/* Increment tcp clients counter */
        sem_post(clients_count_sem);
    }

	/* Close MQ fd */
    mq_close(tcp_fds_q);

	return;
}

/* UDP Server thread function */
void *_udp_server_thread(void *args)
{
	/*
    * Declare:
	* - free_tcp_threads_sem - semaphore, counting currently free threads;
	* - busy_tcp_threads_sem - semaphore, counting currently busy threads;
	* - clients_count_sem - semaphore, counting number of served clients;
	* - udp_endpoints_q - client's endpoints message queue;
    * - fd - socket's fd;
	* - server - this server's endpoint;
	* - endp - endpoint, received from listener server;
	* - tmp_endp - temporary endpoint to verify that message received from 
	*	client;
	* - tmp_size - size of endpoint, passed to recvfrom function;
    * - msg - message buffer.
    */
   	sem_t *free_udp_threads_sem;
    sem_t *busy_udp_threads_sem;
    sem_t *clients_count_sem;
    mqd_t udp_endpoints_q;
	int fd = 0;
	struct sockaddr_in server;
	struct sockaddr_in endp;
	struct sockaddr_in tmp_endp;
	int tmp_size;
	char msg[SERVER_MSG_SIZE];

	/* Open 'tcp_fds_q' */
    udp_endpoints_q = mq_open(SERVER_UDP_QUEUE_NAME, O_RDONLY);
    if (udp_endpoints_q == -1)
    {
		printf("udp mq_open: %s (%d)\n", strerror(errno), errno);
    }

	/* Set canceltype so thread could be canceled at any time*/
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    /* Wait for parent to create sem segments, then get it's fds */
	while ((free_udp_threads_sem = sem_open(SERVER_UDP_FREE_THREADS_SEM_NAME, 
			O_RDWR)) == SEM_FAILED)
	{
		if (errno != ENOENT)
		{
			printf("free_udp_threads_sem sem_open: %s (%d)\n", strerror(errno), 
					errno);
			exit(EXIT_FAILURE);
		}
	}
	while ((busy_udp_threads_sem = sem_open(SERVER_UDP_BUSY_THREADS_SEM_NAME, 
			O_RDWR)) == SEM_FAILED)
	{
		if (errno != ENOENT)
		{
			printf("busy_udp_threads_sem sem_open: %s (%d)\n", strerror(errno), 
					errno);
			exit(EXIT_FAILURE);
		}
	}
    while ((clients_count_sem = sem_open(SERVER_SERVED_UDP_CLIENTS_SEM_NAME, 
			O_RDWR)) == SEM_FAILED)
	{
		if (errno != ENOENT)
		{
			printf("udp clients_count_sem sem_open: %s (%d)\n", 
					strerror(errno), errno);
			exit(EXIT_FAILURE);
		}
	}

	/* Set server's endpoint */
	server.sin_family = AF_INET;
	if (inet_pton(AF_INET, SERVER_ADDR, &server.sin_addr) == -1)
	{
        printf("inet_pton: %s(%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
	}
	server.sin_port = 0;

	/* Create socket, bind endpoint */
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd <= 0)
	{
		printf("socket: %s(%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
	}
	if (bind(fd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
        printf("bind: %s(%d)\n", strerror(errno), errno);
		exit(EXIT_FAILURE);
	}

	/* Increment counter of free threads */
    sem_post(free_udp_threads_sem);

    while(1)
    {
		/* Wait for endpoint from main thread */
		if (mq_receive(udp_endpoints_q, &endp, SERVER_UDP_Q_MSGSIZE, NULL) 
			== -1)
        {
			printf("udp mq_receive: %s (%d)\n", strerror(errno), errno);
            continue;
        }

		/* Send a message, so that client could receive */
		strncpy(msg, SERVER_MSG, CLIENT_MSG_SIZE);
		if (sendto(fd, msg, SERVER_MSG_SIZE, NULL, &endp, sizeof(endp)) 
			== -1)
		{
			printf("udp sendto: %s (%d)\n", strerror(errno), errno);
			return;
		}

		/* Decrement free threads counter, increment busy threads counter */
        sem_wait(free_udp_threads_sem);
        sem_post(busy_udp_threads_sem);

        do
        {
			/* Wait for message from client */
            tmp_size = sizeof(tmp_endp);
            if (recvfrom(fd, msg, SERVER_MSG_SIZE, NULL, &tmp_endp, &tmp_size) 
				== -1)
            {
                printf("udp recvfrom: %s (%d)\n", strerror(errno), errno);
                break;
            }

			/*
			* Verify that endpoint, set in recvfrom function call matches 
			* endpoint, received from main thread.
			*/
            if (endp.sin_port != tmp_endp.sin_port || memcmp(&endp.sin_addr, 
				&tmp_endp.sin_addr, sizeof(struct in_addr)) != 0)
            {
                puts("Incorrect endpoint");
                break;
            }

			/* Send answer to client */
            strncpy(msg, SERVER_MSG, CLIENT_MSG_SIZE);
            if (sendto(fd, msg, CLIENT_MSG_SIZE, NULL, &endp, 
				sizeof(endp)) == -1)
            {
                printf("udp sendto: %s (%d)\n", strerror(errno), errno);
                break;
            }
        } while (CLIENT_MODE);

		/* Decrement busy threads counter, increment free threads counter */
        sem_wait(busy_udp_threads_sem);
        sem_post(free_udp_threads_sem);

		/* Increment tcp clients counter */
        sem_post(clients_count_sem);
    }
	
	/* Close MQ fd */
    mq_close(udp_endpoints_q);

	return;
}

int multiproto_server()
{
	puts("Multiprotocol server");

	/*
    * Declare:
	* - rlim - ;
	* - sa - sigaction structure, used to change disposition for SIGINT signal;
	* - pfds - array of tcp and udp socket fds for multiplexed I/O;
	* - tcp_fds_q & udp_endpoints_q - message queues, used to transfer client 
	*	fds and endpoints to tcp and udp servers, respectively;
	* - tcp_attr & udp_attr - attributes for above-mentioned MQs;
	* - tcp_server_addr & udp_server_addr - endpoints for tcp and udp listener 
	* 	servers;
	* - client_fd - fd of new accepted client;
	* - client_endp - endpoint of new accepted client;
	* - client_size - size of endpoint passed to recvfrom function call;
	* - msg - message buffer, used for receiving messages from sockets;
	* - sem_value - read semaphore value, used in resetting these semaphores, 
	* 	or comparing value;
	* - index - index, used in for loop;
    * - ret - return value.
    */
	struct rlimit rlim;
	struct sigaction sa;
	struct pollfd pfds[2];
	mqd_t tcp_fds_q;
	mqd_t udp_endpoints_q;
    struct mq_attr tcp_attr;
	struct mq_attr udp_attr;
    struct sockaddr_in tcp_server_addr;
	struct sockaddr_in udp_server_addr;
    int client_fd = 0;
    struct sockaddr_in client_endp;
    int client_size;
	char msg[SERVER_MSG_SIZE];
    int sem_value = 0;
    int index;
    int ret = 0;

	/* Print current rlimit, set new one */
    if (getrlimit(RLIMIT_NOFILE, &rlim) == -1)
    {
        printf("getrlimit: %s (%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }
    printf("Current maximum file descriptor: %ld / %ld\n", rlim.rlim_cur, 
			rlim.rlim_max);
    rlim.rlim_cur = rlim.rlim_max;
    printf("New maximum file descriptor: %ld / %ld\n", rlim.rlim_cur, 
			rlim.rlim_max);
    if (setrlimit(RLIMIT_NOFILE, &rlim) == -1)
    {
        printf("setrlimit: %s (%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

	/* Change SIGINT disposition */
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = sigint_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
		printf("sigaction: %s (%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

	/* Set atexit function */
    if (atexit(shutdown_server) != 0)
    {
        printf("atexit: %s (%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

	/* Create counter's semaphores */
    free_tcp_threads_sem = sem_open(SERVER_TCP_FREE_THREADS_SEM_NAME, O_CREAT 
									| O_RDWR, 0666, 0);
	if (free_tcp_threads_sem == SEM_FAILED)
	{
		printf("free_tcp_threads_sem sem_open: %s (%d)\n", strerror(errno), 
				errno);
		exit(EXIT_FAILURE);
	}
	free_udp_threads_sem = sem_open(SERVER_UDP_FREE_THREADS_SEM_NAME, O_CREAT 
									| O_RDWR, 0666, 0);
	if (free_udp_threads_sem == SEM_FAILED)
	{
		printf("free_udp_threads_sem sem_open: %s (%d)\n", strerror(errno), 
				errno);
		exit(EXIT_FAILURE);
	}

	busy_tcp_threads_sem = sem_open(SERVER_TCP_BUSY_THREADS_SEM_NAME, O_CREAT 
									| O_RDWR, 0666, 0);
	if (busy_tcp_threads_sem == SEM_FAILED)
	{
		printf("busy_tcp_threads_sem sem_open: %s (%d)\n", strerror(errno), 
				errno);
		exit(EXIT_FAILURE);
	}
	busy_udp_threads_sem = sem_open(SERVER_UDP_BUSY_THREADS_SEM_NAME, O_CREAT 
									| O_RDWR, 0666, 0);
	if (busy_udp_threads_sem == SEM_FAILED)
	{
		printf("busy_udp_threads_sem sem_open: %s (%d)\n", strerror(errno), 
				errno);
		exit(EXIT_FAILURE);
	}

    served_tcp_clients = sem_open(SERVER_SERVED_TCP_CLIENTS_SEM_NAME, O_CREAT 
									| O_RDWR, 0666, 0);
	if (served_tcp_clients == SEM_FAILED)
	{
		printf("served_tcp_clients sem_open: %s (%d)\n", strerror(errno), 
				errno);
		exit(EXIT_FAILURE);
	}
	served_udp_clients = sem_open(SERVER_SERVED_UDP_CLIENTS_SEM_NAME, O_CREAT 
									| O_RDWR, 0666, 0);
	if (served_udp_clients == SEM_FAILED)
	{
		printf("served_udp_clients sem_open: %s (%d)\n", strerror(errno), 
				errno);
		exit(EXIT_FAILURE);
	}

	/* Set these semaphores to 0 */
    sem_getvalue(free_tcp_threads_sem, &sem_value);
	while (sem_value > 0)
	{
		sem_trywait(free_tcp_threads_sem);
		sem_getvalue(free_tcp_threads_sem, &sem_value);
	}
    sem_getvalue(free_udp_threads_sem, &sem_value);
	while (sem_value > 0)
	{
		sem_trywait(free_udp_threads_sem);
		sem_getvalue(free_udp_threads_sem, &sem_value);
	}
	sem_getvalue(busy_tcp_threads_sem, &sem_value);
	while (sem_value > 0)
	{
		sem_trywait(busy_tcp_threads_sem);
		sem_getvalue(busy_tcp_threads_sem, &sem_value);
	}
	sem_getvalue(busy_udp_threads_sem, &sem_value);
	while (sem_value > 0)
	{
		sem_trywait(busy_udp_threads_sem);
		sem_getvalue(busy_udp_threads_sem, &sem_value);
	}
	sem_getvalue(served_tcp_clients, &sem_value);
	while (sem_value > 0)
	{
		sem_trywait(served_tcp_clients);
		sem_getvalue(served_tcp_clients, &sem_value);
	}
	sem_getvalue(served_udp_clients, &sem_value);
	while (sem_value > 0)
	{
		sem_trywait(served_udp_clients);
		sem_getvalue(served_udp_clients, &sem_value);
	}

	/* Set message queue's attributes */
    tcp_attr.mq_maxmsg = SERVER_Q_MAXMSG;
	tcp_attr.mq_msgsize = SERVER_TCP_Q_MSGSIZE;

	udp_attr.mq_maxmsg = SERVER_Q_MAXMSG;
	udp_attr.mq_msgsize = SERVER_UDP_Q_MSGSIZE;

	/* Create MQs */
    tcp_fds_q = mq_open(SERVER_TCP_QUEUE_NAME, O_CREAT | O_RDWR, 0666, 
						&tcp_attr);
    if (tcp_fds_q == -1)
    {
		printf("tcp_fds_q mq_open: %s (%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }
	udp_endpoints_q = mq_open(SERVER_UDP_QUEUE_NAME, O_CREAT | O_RDWR, 0666, 
								&udp_attr);
    if (udp_endpoints_q == -1)
    {
		printf("udp_endpoints_q mq_open: %s (%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }


	/*
	* Set values to thread counters, allocate memory to 'tcp_threads' and 
	* 'udp_threads'.
	*/
    tcp_threads_count = SERVER_DEF_ALLOC;
    tcp_threads = malloc(tcp_threads_count * 
							sizeof(struct tcp_server_thread_t));
    if (tcp_threads == NULL)
    {
        printf("malloc: %s(%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }
    for (index = 0; index < tcp_threads_count; index++)
    {
        pthread_create(&tcp_threads[index], NULL, _tcp_server_thread, NULL);
    }

    udp_threads_count = SERVER_DEF_ALLOC;
    udp_threads = malloc(udp_threads_count * 
							sizeof(struct udp_server_thread_t));
    if (udp_threads == NULL)
    {
        printf("malloc: %s(%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    for (index = 0; index < udp_threads_count; index++)
    {
        pthread_create(&udp_threads[index], NULL, _udp_server_thread, NULL);
    }

    /* Fill 'tcp_server_addr' and 'udp_server_addr' with 0's */
	memset(&tcp_server_addr, 0, sizeof(tcp_server_addr));
	memset(&udp_server_addr, 0, sizeof(udp_server_addr));

	/* Set endpoints */
	tcp_server_addr.sin_family = AF_INET;
	if (inet_pton(AF_INET, SERVER_ADDR, &tcp_server_addr.sin_addr) == -1)
	{
        printf("inet_pton: %s(%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
	}
	tcp_server_addr.sin_port = htons(SERVER_TCP_PORT);

	udp_server_addr.sin_family = AF_INET;
	if (inet_pton(AF_INET, SERVER_ADDR, &udp_server_addr.sin_addr) == -1)
	{
        printf("inet_pton: %s(%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
	}
	udp_server_addr.sin_port = htons(SERVER_UDP_PORT);

	/* Create sockets */
    tcp_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_server_fd <= 0)
	{
		printf("socket: %s(%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
	}

	udp_server_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (udp_server_fd <= 0)
	{
		printf("socket: %s(%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
	}

	/*
	* Allow sockets to claim used port if necessary, should be used only in 
	* educational purposes.
	*/
	setsockopt(udp_server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, 
				sizeof(int));
	setsockopt(tcp_server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, 
				sizeof(int));

	/* Bind endpoint to each socket */
    if (bind(tcp_server_fd, (struct sockaddr *)&tcp_server_addr, 
		sizeof(tcp_server_addr)) == -1)
	{
        printf("bind: %s(%d)\n", strerror(errno), errno);
		exit(EXIT_FAILURE);
	}

	if (bind(udp_server_fd, (struct sockaddr *)&udp_server_addr, 
		sizeof(udp_server_addr)) == -1)
	{
        printf("bind: %s(%d)\n", strerror(errno), errno);
		exit(EXIT_FAILURE);
	}

	/* Set TCP server as passive socket */
    if (listen(tcp_server_fd, SERVER_LISTEN_BACKLOG) == -1)
	{
        printf("listen: %s(%d)\n", strerror(errno), errno);
		exit(EXIT_FAILURE);
	}

	/* Fill 'pfds' array with socket fd and set event to wait for */
	pfds[0].fd = tcp_server_fd;
	pfds[1].fd = udp_server_fd;
	pfds[0].events = POLLIN;
	pfds[1].events = POLLIN;

    while(1)
    {
		/* Poll until one or both sockets receive a message from client */
		ret = poll(pfds, 2, 0);
		if (ret > 0)
		{
			if (pfds[0].revents & POLLIN)
			{
				/* Accept TCP client */
				if ((client_fd = accept(tcp_server_fd, NULL, NULL)) == -1)
				{
					printf("tcp accept: %s(%d)\n", strerror(errno), errno);
					exit(EXIT_FAILURE);
				}

				/*
				* Check that there is free processing servers available,
				* if not, check that all the processing threads initialized and
				* busy, then allocate more memory to 'tcp_threads', increase
				* counter and create new threads, if there are still not
				* initialized threads, wait in a loop, sleeping for duration of
				* SERVER_SLEEP_DUR_NSEC nanoseconds every iteration.
				*/
				while(sem_getvalue(free_tcp_threads_sem, &sem_value) != -1 && 
						sem_value == 0)
				{
					if (sem_getvalue(busy_tcp_threads_sem, &sem_value) != -1 && 
						sem_value == tcp_threads_count)
					{
						struct tcp_server_thread_t *tmp;

						tmp = realloc(tcp_threads, 
									(tcp_threads_count+SERVER_DEF_ALLOC) * 
									sizeof(struct tcp_server_thread_t));
						if (tmp == NULL)
						{
							printf("tcp realloc: %s (%d)\n", strerror(errno), 
									errno);
							exit(EXIT_FAILURE);
						}

						tcp_threads = tmp;

						for (index = tcp_threads_count; 
							index < (tcp_threads_count+SERVER_DEF_ALLOC); 
							index++)
						{
							pthread_create(&tcp_threads[index], NULL, 
											_tcp_server_thread, NULL);
						}

						tcp_threads_count += SERVER_DEF_ALLOC;
						tmp = NULL;

						break;
					}
					
					usleep(SERVER_SLEEP_DUR_NSEC);
				}

				/*
				* Send client's fd to MQ, read by processing servers, one of
				* those will receive a message and proceed to client
				* communication. IF mq_send call failed, run another call if MQ
				* has reached its limit, until one of the processing servers
				* read MQ, allowing main thread to write this client's fd to
				* queue.
				*/
				while (mq_send(tcp_fds_q, &client_fd, SERVER_TCP_Q_MSGSIZE,
						NULL) == -1)
				{
					if (errno != EAGAIN)
					{
						printf("tcp mq_send: %s (%d)\n", strerror(errno), 
								errno);
						exit(EXIT_FAILURE);
					}
				}

				pfds[0].revents = 0;
			}
			if (pfds[1].revents & POLLIN)
			{
				/* Read UDP client's endpoint */
				client_size = sizeof(client_endp);
				if (recvfrom(udp_server_fd, msg, SERVER_MSG_SIZE, 0, 
					(struct sockaddr *)&client_endp, &client_size) == -1)
				{
					printf("udp recvfrom: %s(%d)\n", strerror(errno), errno);
					exit(EXIT_FAILURE);
				}

				/*
				* Check that there is free processing servers available,
				* if not, check that all the processing threads initialized and
				* busy, then allocate more memory to 'udp_threads', increase
				* counter and create new threads, if there are still not
				* initialized threads, wait in a loop, sleeping for duration of
				* SERVER_SLEEP_DUR_NSEC nanoseconds every iteration.
				*/
				while(sem_getvalue(free_udp_threads_sem, &sem_value) != -1 && 
						sem_value == 0)
				{
					if (sem_getvalue(busy_udp_threads_sem, &sem_value) != -1 && 
						sem_value == udp_threads_count)
					{
						struct tcp_server_thread_t *tmp;

						tmp = realloc(udp_threads, 
										(udp_threads_count+SERVER_DEF_ALLOC) * 
										sizeof(struct tcp_server_thread_t));
						if (tmp == NULL)
						{
							printf("udp realloc: %s (%d)\n", strerror(errno), 
									errno);
							exit(EXIT_FAILURE);
						}
						udp_threads = tmp;

						for (index = udp_threads_count; 
								index < (udp_threads_count+SERVER_DEF_ALLOC); 
								index++)
						{
							pthread_create(&udp_threads[index], NULL, 
											_udp_server_thread, NULL);
						}

						udp_threads_count += SERVER_DEF_ALLOC;
						tmp = NULL;

						break;
					}

					usleep(SERVER_SLEEP_DUR_NSEC);
				}

				/*
				* Send client's endpoint to MQ, read by processing servers, one
				* of those will receive a message and proceed to client
				* communication. IF mq_send call failed, run another call if MQ
				* has reached its limit, until one of the processing servers
				* read MQ, allowing main thread to write this client's endpoint
				* to queue.
				*/
				while (mq_send(udp_endpoints_q, &client_endp, SERVER_UDP_Q_MSGSIZE, 
						NULL) == -1)
				{
					if (errno != EAGAIN)
					{
						printf("udp mq_send: %s (%d)\n", strerror(errno), 
								errno);
						exit(EXIT_FAILURE);
					}
				}

				pfds[1].revents = 0;
			}
		}
    }

	/* Close MQs */
    mq_close(tcp_fds_q);
	mq_close(udp_endpoints_q);

    return ret;
}
