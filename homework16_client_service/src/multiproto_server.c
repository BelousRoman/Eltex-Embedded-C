#include "../hdr/functions.h"

int tcp_server_fd;
int udp_server_fd;

sem_t *free_tcp_threads_sem = NULL;
sem_t *free_udp_threads_sem = NULL;
sem_t *busy_tcp_threads_sem = NULL;
sem_t *busy_udp_threads_sem = NULL;
sem_t *served_tcp_clients = NULL;
sem_t *served_udp_clients = NULL;

struct tcp_server_thread_t *tcp_threads = NULL;
int tcp_threads_count = 0;
pthread_mutex_t tcp_threads_mutex = PTHREAD_MUTEX_INITIALIZER;

struct udp_server_thread_t *udp_threads = NULL;
int udp_threads_count = 0;
pthread_mutex_t udp_threads_mutex = PTHREAD_MUTEX_INITIALIZER;

// sem_t *clients_count_sem;

static void sigint_handler(int sig, siginfo_t *si, void *unused)
{
    exit(EXIT_SUCCESS);
}

void shutdown_server(void)
{
	int tcp_clients_count = 0;
	int udp_clients_count = 0;
	int index;

	if (served_tcp_clients != NULL)
		sem_getvalue(served_tcp_clients, &tcp_clients_count);

	if (served_udp_clients != NULL)
		sem_getvalue(served_udp_clients, &udp_clients_count);

	if (tcp_server_fd > 0)
		close(tcp_server_fd);

	if (udp_server_fd > 0)
		close(udp_server_fd);

    if (tcp_threads != NULL)
    {
        for (index = 0; index < tcp_threads_count; index++)
        {
            pthread_cancel(tcp_threads[index].tid);
            if (tcp_threads[index].client_fd > 0)
                close(tcp_threads[index].client_fd);
        }

        free(tcp_threads);
    }

	if (udp_threads != NULL)
    {
        for (index = 0; index < udp_threads_count; index++)
        {
            pthread_cancel(udp_threads[index].tid);
        }

        free(udp_threads);
    }

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

	printf("Shut server at:\n* %d client(s) (%d by TCP and %d by UDP)\n", (tcp_clients_count+udp_clients_count), tcp_clients_count, udp_clients_count);
}

void *_tcp_server_thread(void *args)
{
	char msg[SERVER_MSG_SIZE];
    mqd_t tcp_fds_q;
	int fd = 0;
    sem_t *free_tcp_threads_sem;
    sem_t *busy_tcp_threads_sem;
    sem_t *clients_count_sem;

    int thread_id = (int)args;

    int sem_value;
    int ret = EXIT_SUCCESS;

    tcp_fds_q = mq_open(SERVER_TCP_QUEUE_NAME, O_RDONLY);
    if (tcp_fds_q == -1)
    {
        printf("tcp mq_open: %s (%d)\n", strerror(errno), errno);
    }

	/* Set canceltype so thread could be canceled at any time*/
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    /* Wait for parent to create sem segments, then get it's fds */
	while ((free_tcp_threads_sem = sem_open(SERVER_TCP_FREE_THREADS_SEM_NAME, O_RDWR)) == SEM_FAILED)
	{
		if (errno != ENOENT)
		{
			printf("free_tcp_threads_sem sem_open: %s (%d)\n", strerror(errno), errno);
			exit(EXIT_FAILURE);
		}
	}
	while ((busy_tcp_threads_sem = sem_open(SERVER_TCP_BUSY_THREADS_SEM_NAME, O_RDWR)) == SEM_FAILED)
	{
		if (errno != ENOENT)
		{
			printf("busy_tcp_threads_sem sem_open: %s (%d)\n", strerror(errno), errno);
			exit(EXIT_FAILURE);
		}
	}
    while ((clients_count_sem = sem_open(SERVER_SERVED_TCP_CLIENTS_SEM_NAME, O_RDWR)) == SEM_FAILED)
	{
		if (errno != ENOENT)
		{
			printf("clients_count_sem sem_open: %s (%d)\n", strerror(errno), errno);
			exit(EXIT_FAILURE);
		}
	}

    sem_post(free_tcp_threads_sem);

    while(1)
    {
        if (mq_receive(tcp_fds_q, &fd, sizeof(int), NULL) == -1)
        {
			printf("mq_receive: %s (%d)\n", strerror(errno), errno);
            continue;
        }

        sem_wait(free_tcp_threads_sem);
        sem_post(busy_tcp_threads_sem);

        pthread_mutex_lock(&tcp_threads_mutex);
        pthread_mutex_unlock(&tcp_threads_mutex);

        pthread_mutex_lock(&tcp_threads[thread_id].mutex);
        tcp_threads[thread_id].client_fd = fd;
        pthread_mutex_unlock(&tcp_threads[thread_id].mutex);

        //printf("Processing tcp_server_addr #%d will read the %dth fd\n", thread_id, fd);

        while(1)
        {
            ret = recv(fd, &msg, sizeof(msg), 0);
            if (ret == -1)
            {
				printf("tcp recv: %s (%d)\n", strerror(errno), errno);
				if (errno == ECONNRESET)
					exit(1);
				else
                	break;
            }
            else if (ret == 0)
            {
                break;
            }
			
			break;
        }
        sem_wait(busy_tcp_threads_sem);
        sem_post(free_tcp_threads_sem);

        pthread_mutex_lock(&tcp_threads_mutex);
        pthread_mutex_unlock(&tcp_threads_mutex);

        pthread_mutex_lock(&tcp_threads[thread_id].mutex);
        tcp_threads[thread_id].client_fd = 0;
        pthread_mutex_unlock(&tcp_threads[thread_id].mutex);

        sem_post(clients_count_sem);
    }
	
    mq_close(tcp_fds_q);

	return;
}

// TODO: Refine logic, guess each thread should create its own socket and after mq_receive call sendto, providing received endpoint as an argument
void *_udp_server_thread(void *args)
{
	char msg[SERVER_MSG_SIZE];
    mqd_t udp_endpoints_q;
	struct sockaddr_in endpoint;
	struct sockaddr_in server;
    sem_t *free_udp_threads_sem;
    sem_t *busy_udp_threads_sem;
    sem_t *clients_count_sem;

	int thread_id;
	int fd = 0;
	int sem_value;
    int ret = EXIT_SUCCESS;

    thread_id = (int)args;

    udp_endpoints_q = mq_open(SERVER_UDP_QUEUE_NAME, O_RDONLY);
    if (udp_endpoints_q == -1)
    {
		printf("udp mq_open: %s (%d)\n", strerror(errno), errno);
    }

	/* Set canceltype so thread could be canceled at any time*/
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    /* Wait for parent to create sem segments, then get it's fds */
	while ((free_udp_threads_sem = sem_open(SERVER_UDP_FREE_THREADS_SEM_NAME, O_RDWR)) == SEM_FAILED)
	{
		if (errno != ENOENT)
		{
			printf("tcp free_tcp_threads_sem sem_open: %s (%d)\n", strerror(errno), errno);
			exit(EXIT_FAILURE);
		}
	}
	while ((busy_udp_threads_sem = sem_open(SERVER_UDP_BUSY_THREADS_SEM_NAME, O_RDWR)) == SEM_FAILED)
	{
		if (errno != ENOENT)
		{
			printf("busy_tcp_threads_sem sem_open: %s (%d)\n", strerror(errno), errno);
			exit(EXIT_FAILURE);
		}
	}
    while ((clients_count_sem = sem_open(SERVER_SERVED_UDP_CLIENTS_SEM_NAME, O_RDWR)) == SEM_FAILED)
	{
		if (errno != ENOENT)
		{
			printf("tcp clients_count_sem sem_open: %s (%d)\n", strerror(errno), errno);
			exit(EXIT_FAILURE);
		}
	}

	server.sin_family = AF_INET;
	if (inet_pton(AF_INET, SERVER_ADDR, &server.sin_addr) == -1)
	{
        printf("inet_pton: %s(%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
	}
	server.sin_port = 0;

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

    sem_post(free_udp_threads_sem);

    while(1)
    {
		if (mq_receive(udp_endpoints_q, &endpoint, SERVER_UDP_Q_MSGSIZE, NULL) == -1) // sendto after this
        {
			printf("udp mq_receive: %s (%d)\n", strerror(errno), errno);
            continue;
        }

		strncpy(msg, SERVER_MSG, CLIENT_MSG_SIZE);
		if (sendto(fd, msg, SERVER_MSG_SIZE, NULL, &endpoint, sizeof(endpoint)) == -1)
		{
			printf("udp sendto: %s (%d)\n", strerror(errno), errno);
			return;
		}

        sem_wait(free_udp_threads_sem);
        sem_post(busy_udp_threads_sem);

        pthread_mutex_lock(&udp_threads_mutex);
        pthread_mutex_unlock(&udp_threads_mutex);

        pthread_mutex_lock(&udp_threads[thread_id].mutex);
        udp_threads[thread_id].client_endp = endpoint;
        pthread_mutex_unlock(&udp_threads[thread_id].mutex);

        do
        {
			struct sockaddr_in tmp;
            int tmp_size = sizeof(tmp);
            if (recvfrom(fd, msg, SERVER_MSG_SIZE, NULL, &tmp, &tmp_size) == -1)
            {
                printf("udp recvfrom: %s (%d)\n", strerror(errno), errno);
                break;
            }

            if (tmp.sin_port != endpoint.sin_port || memcmp(&tmp.sin_addr, &endpoint.sin_addr, sizeof(struct in_addr)) != 0)
            {
                puts("Incorrect endpoint");
                break;
            }

            strncpy(msg, SERVER_MSG, CLIENT_MSG_SIZE);
            if (sendto(fd, msg, CLIENT_MSG_SIZE, NULL, &endpoint, sizeof(endpoint)) == -1)
            {
                printf("udp sendto: %s (%d)\n", strerror(errno), errno);
                break;
            }
        } while (CLIENT_MODE);

        sem_wait(busy_udp_threads_sem);
        sem_post(free_udp_threads_sem);

        pthread_mutex_lock(&udp_threads_mutex);
        pthread_mutex_unlock(&udp_threads_mutex);

        pthread_mutex_lock(&udp_threads[thread_id].mutex);
		memset(&udp_threads[thread_id].client_endp, 0, sizeof(udp_threads[thread_id].client_endp));
        pthread_mutex_unlock(&udp_threads[thread_id].mutex);

        sem_post(clients_count_sem);

		printf("UDP #%d: post client to counter\n", thread_id);
    }
	
    mq_close(udp_endpoints_q);

	return;
}

int multiproto_server()
{
	puts("Multiprotocol server");

	struct rlimit rlim;
	struct pollfd pfds[2];
    struct sockaddr_in tcp_server_addr;
	struct sockaddr_in udp_server_addr;
    int tmp_fd = 0;
    struct sockaddr_in client;
    int client_size;
	char msg[SERVER_MSG_SIZE];

    struct sigaction sa;

    int sem_value = 0;

    mqd_t tcp_fds_q;
	mqd_t udp_endpoints_q;
    struct mq_attr tcp_attr;
	struct mq_attr udp_attr;

    int index;
    int ret = 0;

	/* Print current rlimit, set new one */
    if (getrlimit(RLIMIT_NOFILE, &rlim) == -1)
    {
        printf("getrlimit: %s (%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }
    printf("Current maximum file descriptor: %ld / %ld\n", rlim.rlim_cur, rlim.rlim_max);
    rlim.rlim_cur = rlim.rlim_max;
    printf("New maximum file descriptor: %ld / %ld\n", rlim.rlim_cur, rlim.rlim_max);
    if (setrlimit(RLIMIT_NOFILE, &rlim) == -1)
    {
        printf("setrlimit: %s (%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = sigint_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
		printf("sigaction: %s (%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    if (atexit(shutdown_server) != 0)
    {
        printf("atexit: %s (%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    free_tcp_threads_sem = sem_open(SERVER_TCP_FREE_THREADS_SEM_NAME, O_CREAT | O_RDWR, 0666, 0);
	if (free_tcp_threads_sem == SEM_FAILED)
	{
		printf("free_tcp_threads_sem sem_open: %s (%d)\n", strerror(errno), errno);
		exit(EXIT_FAILURE);
	}
	free_udp_threads_sem = sem_open(SERVER_UDP_FREE_THREADS_SEM_NAME, O_CREAT | O_RDWR, 0666, 0);
	if (free_udp_threads_sem == SEM_FAILED)
	{
		printf("free_udp_threads_sem sem_open: %s (%d)\n", strerror(errno), errno);
		exit(EXIT_FAILURE);
	}

	busy_tcp_threads_sem = sem_open(SERVER_TCP_BUSY_THREADS_SEM_NAME, O_CREAT | O_RDWR, 0666, 0);
	if (busy_tcp_threads_sem == SEM_FAILED)
	{
		printf("busy_tcp_threads_sem sem_open: %s (%d)\n", strerror(errno), errno);
		exit(EXIT_FAILURE);
	}
	busy_udp_threads_sem = sem_open(SERVER_UDP_BUSY_THREADS_SEM_NAME, O_CREAT | O_RDWR, 0666, 0);
	if (busy_udp_threads_sem == SEM_FAILED)
	{
		printf("busy_udp_threads_sem sem_open: %s (%d)\n", strerror(errno), errno);
		exit(EXIT_FAILURE);
	}

    served_tcp_clients = sem_open(SERVER_SERVED_TCP_CLIENTS_SEM_NAME, O_CREAT | O_RDWR, 0666, 0);
	if (served_tcp_clients == SEM_FAILED)
	{
		printf("served_tcp_clients sem_open: %s (%d)\n", strerror(errno), errno);
		exit(EXIT_FAILURE);
	}
	served_udp_clients = sem_open(SERVER_SERVED_UDP_CLIENTS_SEM_NAME, O_CREAT | O_RDWR, 0666, 0);
	if (served_udp_clients == SEM_FAILED)
	{
		printf("served_udp_clients sem_open: %s (%d)\n", strerror(errno), errno);
		exit(EXIT_FAILURE);
	}

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

    tcp_attr.mq_maxmsg = SERVER_Q_MAXMSG;
	tcp_attr.mq_msgsize = SERVER_TCP_Q_MSGSIZE;

	udp_attr.mq_maxmsg = SERVER_Q_MAXMSG;
	udp_attr.mq_msgsize = SERVER_UDP_Q_MSGSIZE;

    tcp_fds_q = mq_open(SERVER_TCP_QUEUE_NAME, O_CREAT | O_RDWR, 0666, &tcp_attr);
    if (tcp_fds_q == -1)
    {
		printf("tcp_fds_q mq_open: %s (%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

	udp_endpoints_q = mq_open(SERVER_UDP_QUEUE_NAME, O_CREAT | O_RDWR, 0666, &udp_attr);
    if (udp_endpoints_q == -1)
    {
		printf("udp_endpoints_q mq_open: %s (%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    pthread_mutex_lock(&tcp_threads_mutex);
    tcp_threads_count = SERVER_DEF_ALLOC;
    tcp_threads = malloc(tcp_threads_count * sizeof(struct tcp_server_thread_t));
    if (tcp_threads == NULL)
    {
        printf("malloc: %s(%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    for (index = 0; index < tcp_threads_count; index++)
    {
        tcp_threads[index].tid = NULL;
        pthread_create(&tcp_threads[index].tid, NULL, _tcp_server_thread, index);
        tcp_threads[index].client_fd = 0;
        pthread_mutex_init(&tcp_threads[index].mutex, NULL);
    }
    pthread_mutex_unlock(&tcp_threads_mutex);

	pthread_mutex_lock(&udp_threads_mutex);
    udp_threads_count = SERVER_DEF_ALLOC;
    udp_threads = malloc(udp_threads_count * sizeof(struct udp_server_thread_t));
    if (udp_threads == NULL)
    {
        printf("malloc: %s(%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    for (index = 0; index < udp_threads_count; index++)
    {
        udp_threads[index].tid = NULL;
        pthread_create(&udp_threads[index].tid, NULL, _udp_server_thread, index);
		memset(&udp_threads[index].client_endp, 0, sizeof(udp_threads[index].client_endp));
        pthread_mutex_init(&udp_threads[index].mutex, NULL);
    }
    pthread_mutex_unlock(&udp_threads_mutex);

    /* Fill 'tcp_server_addr' and 'udp_server_addr' with 0's */
	memset(&tcp_server_addr, 0, sizeof(tcp_server_addr));
	memset(&udp_server_addr, 0, sizeof(udp_server_addr));

	/* Set tcp_server_addr's endpoint */
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

	setsockopt(udp_server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	setsockopt(tcp_server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    if (bind(tcp_server_fd, (struct sockaddr *)&tcp_server_addr, sizeof(tcp_server_addr)) == -1)
	{
        printf("bind: %s(%d)\n", strerror(errno), errno);
		exit(EXIT_FAILURE);
	}

	if (bind(udp_server_fd, (struct sockaddr *)&udp_server_addr, sizeof(udp_server_addr)) == -1)
	{
        printf("bind: %s(%d)\n", strerror(errno), errno);
		exit(EXIT_FAILURE);
	}

    if (listen(tcp_server_fd, SERVER_LISTEN_BACKLOG) == -1)
	{
        printf("listen: %s(%d)\n", strerror(errno), errno);
		exit(EXIT_FAILURE);
	}

	pfds[0].fd = tcp_server_fd;
	pfds[1].fd = udp_server_fd;
	pfds[0].events = POLLIN;
	pfds[1].events = POLLIN;

    while(1)
    {
		int ret = poll(pfds, 2, 0);
		if (ret > 0)
		{
			if (pfds[0].revents & POLLIN)
			{
				client_size = sizeof(client);
				if ((tmp_fd = accept(tcp_server_fd, (struct sockaddr *)&client,
										&client_size)) == -1)
				{
					printf("tcp accept: %s(%d)\n", strerror(errno), errno);
					exit(EXIT_FAILURE);
				}

				while(sem_getvalue(free_tcp_threads_sem, &sem_value) != -1 && sem_value == 0)
				{
					if (sem_getvalue(busy_tcp_threads_sem, &sem_value) != -1 && sem_value == tcp_threads_count)
					{
						struct tcp_server_thread_t *tmp;

						pthread_mutex_lock(&tcp_threads_mutex);
						tmp = realloc(tcp_threads, (tcp_threads_count+SERVER_DEF_ALLOC) * sizeof(struct tcp_server_thread_t));
						if (tmp == NULL)
						{
							printf("tcp realloc: %s (%d)\n", strerror(errno), errno);
							exit(EXIT_FAILURE);
						}

						tcp_threads = tmp;

						for (index = tcp_threads_count; index < (tcp_threads_count+SERVER_DEF_ALLOC); index++)
						{
							tcp_threads[index].tid = NULL;
							pthread_create(&tcp_threads[index].tid, NULL, _tcp_server_thread, index);
							tcp_threads[index].client_fd = 0;
							pthread_mutex_init(&tcp_threads[index].mutex, NULL);
						}

						tcp_threads_count += SERVER_DEF_ALLOC;
						pthread_mutex_unlock(&tcp_threads_mutex);

						tmp = NULL;

						break;
					}
				}

				while (mq_send(tcp_fds_q, &tmp_fd, SERVER_TCP_Q_MSGSIZE, NULL) == -1)
				{
					if (errno != EAGAIN)
					{
						printf("tcp mq_send: %s (%d)\n", strerror(errno), errno);
						exit(EXIT_FAILURE);
					}
				}

				pfds[0].revents = 0;
			}
			if (pfds[1].revents & POLLIN)
			{
				client_size = sizeof(client);
				if (recvfrom(udp_server_fd, msg, SERVER_MSG_SIZE, 0, (struct sockaddr *)&client, &client_size) == -1)
				{
					printf("udp recvfrom: %s(%d)\n", strerror(errno), errno);
					exit(EXIT_FAILURE);
				}
				while(sem_getvalue(free_udp_threads_sem, &sem_value) != -1 && sem_value == 0)
				{
					if (sem_getvalue(busy_udp_threads_sem, &sem_value) != -1 && sem_value == udp_threads_count)
					{
						struct tcp_server_thread_t *tmp;

						pthread_mutex_lock(&udp_threads_mutex);
						tmp = realloc(udp_threads, (udp_threads_count+SERVER_DEF_ALLOC) * sizeof(struct tcp_server_thread_t));
						if (tmp == NULL)
						{
							printf("udp realloc: %s (%d)\n", strerror(errno), errno);
							exit(EXIT_FAILURE);
						}
						udp_threads = tmp;

						for (index = udp_threads_count; index < (udp_threads_count+SERVER_DEF_ALLOC); index++)
						{
							udp_threads[index].tid = NULL;
							pthread_create(&udp_threads[index].tid, NULL, _udp_server_thread, index);
							memset(&udp_threads[index].client_endp, 0, sizeof(udp_threads[index].client_endp));
							pthread_mutex_init(&udp_threads[index].mutex, NULL);
						}

						udp_threads_count += SERVER_DEF_ALLOC;
						pthread_mutex_unlock(&tcp_threads_mutex);

						tmp = NULL;

						break;
					}
				}

				while (mq_send(udp_endpoints_q, &client, SERVER_UDP_Q_MSGSIZE, NULL) == -1)
				{
					if (errno != EAGAIN)
					{
						printf("udp mq_send: %s (%d)\n", strerror(errno), errno);
						exit(EXIT_FAILURE);
					}
				}

				pfds[1].revents = 0;
			}
		}
    }

    mq_close(tcp_fds_q);
	mq_close(udp_endpoints_q);

    return ret;
}
