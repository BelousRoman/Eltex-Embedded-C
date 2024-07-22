#include "../hdr/functions.h"

/*
* Declare global variables:
* - tcp_tids & udp_tids - dynamic arrays of thread ids;
* - create_flag - flag for the main thread to continue creating threads;
* - tcp_alloc_threads & udp_alloc_threads - currently allocated elements in 'tcp_tids' and 'udp_tids' arrays;
* - tcp_threads_count & udp_threads_count - number of created threads;
* - tcp_counter_sem - semaphore to store number of clients, served by server;
* - tcp_clients_count - variable, passed to sem_getvalue()
*   to get number of clients, served by server.
*/
// int create_flag = 1;
pthread_t *tcp_tids = NULL;
pthread_t *udp_tids = NULL;
int tcp_alloc_threads = CLIENT_DEF_ALLOC;
int udp_alloc_threads = CLIENT_DEF_ALLOC;
unsigned int tcp_threads_count = 0;
unsigned int udp_threads_count = 0;
sem_t * tcp_counter_sem = NULL;
sem_t * udp_counter_sem = NULL;

/* handler function for SIGINT signal */
static void sigint_handler(int sig, siginfo_t *si, void *unused)
{
    exit(EXIT_SUCCESS);
}

void shutdown_client(void)
{
	int tcp_clients_count = 0;
	int udp_clients_count = 0;
	int index;

    if (tcp_tids != NULL)
    {
        for (index = 0; index < tcp_threads_count; index++)
        {
            pthread_cancel(tcp_tids[index]);
        }
        free(tcp_tids);
    }

	if (udp_tids != NULL)
    {
        for (index = 0; index < udp_threads_count; index++)
        {
            pthread_cancel(udp_tids[index]);
        }
        free(udp_tids);
    }

    if (tcp_counter_sem != NULL)
		sem_getvalue(tcp_counter_sem, &tcp_clients_count);
	if (udp_counter_sem != NULL)
		sem_getvalue(udp_counter_sem, &udp_clients_count);

    sem_close(tcp_counter_sem);
    sem_close(udp_counter_sem);
    unlink(CLIENT_TCP_COUNTER_SEM_NAME);
    unlink(CLIENT_UDP_COUNTER_SEM_NAME);

    printf("Shut clients at:\n* %d client (%d by TCP and %d by UDP)\n", (tcp_clients_count+udp_clients_count), tcp_clients_count, udp_clients_count);
}

/* TCP Client thread function */
void *tcp_client(void *args)
{
    /*
    * Declare global variable:
    * - server - server's endpoint;
    * - server_fd - fd of server socket;
    * - msg - message buffer.
    */
    struct sockaddr_in server;
    int server_fd;
    char msg[CLIENT_MSG_SIZE];

    /* Set canceltype so thread could be canceled at any time*/
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    /* Fill 'server' with 0's */
	memset(&server, 0, sizeof(server));

	/* Set server's endpoint */
	server.sin_family = AF_INET;
	if (inet_pton(AF_INET, SERVER_ADDR, &server.sin_addr) == -1)
	{
		printf("tcp inet_pton: %s (%d)\n", strerror(errno), errno);
		exit(1);
	}
	server.sin_port = htons(SERVER_TCP_PORT);

    /* Create socket */
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        printf("tcp socket: %s (%d)\n", strerror(errno), errno);
        exit(1);
    }

    /* Connect to server */
    if (connect(server_fd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        printf("tcp connect: %s (%d)\n", strerror(errno), errno);
        exit(1);
    }

    /* Increment served clients counter */
    sem_post(tcp_counter_sem);

    do
    {
        /* Send message to server */
        strncpy(msg, CLIENT_MSG, CLIENT_MSG_SIZE);
        if (send(server_fd, msg, CLIENT_MSG_SIZE, 0) == -1)
        {
            printf("tcp send: %s (%d)\n", strerror(errno), errno);
            exit(1);
        }

        /* Wait for message from server */
        if (recv(server_fd, msg, CLIENT_MSG_SIZE, 0) == -1)
        {
            printf("tcp recv: %s (%d)\n", strerror(errno), errno);
            exit(1);
        }
    } while (CLIENT_MODE);
}

/* UDP Client thread function */
void *udp_client(void *args)
{
    struct sockaddr_in server;
    struct sockaddr_in tmp_endpoint;
    int server_size;
    int server_fd;
    char msg[CLIENT_MSG_SIZE];
    // char addr[INET6_ADDRSTRLEN];

    /* Set canceltype so thread could be canceled at any time*/
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    /* Fill 'server' with 0's */
	memset(&server, 0, sizeof(server));

    /* Set server's endpoint */
    server.sin_family = AF_INET;
    if (inet_pton(AF_INET, SERVER_ADDR, &server.sin_addr) == -1)
    {
        printf("udp inet_pton: %s (%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }
    server.sin_port = htons(SERVER_UDP_PORT);

    /* Create socket */
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd == -1)
    {
        printf("udp socket: %s (%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    /* Connect to server */
    memset(msg, 0, CLIENT_MSG_SIZE);
    if (sendto(server_fd, msg, CLIENT_MSG_SIZE, 0, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        printf("udp sendto: %s (%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    /* Wait for message from server */
    server_size = sizeof(server);
    if (recvfrom(server_fd, msg, CLIENT_MSG_SIZE, 0, (struct sockaddr *)&server, &server_size) == -1)
    {
        printf("udp recv: %s (%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    /* Increment served clients counter */
    sem_post(udp_counter_sem);

    do
    {
        /* Send message to server */
        strncpy(msg, CLIENT_MSG, CLIENT_MSG_SIZE);
        if (sendto(server_fd, msg, CLIENT_MSG_SIZE, 0, (struct sockaddr *)&server, server_size) == -1)
        {
            printf("udp sendto: %s (%d)\n", strerror(errno), errno);
            exit(EXIT_FAILURE);
        }

        server_size = sizeof(tmp_endpoint);
        if (recvfrom(server_fd, msg, CLIENT_MSG_SIZE, 0, (struct sockaddr *)&tmp_endpoint, &server_size) == -1)
        {
            printf("udp recvfrom: %s (%d)\n", strerror(errno), errno);
            exit(EXIT_FAILURE);
        }

        if (tmp_endpoint.sin_port != server.sin_port || memcmp(&tmp_endpoint.sin_addr, &server.sin_addr, sizeof(struct in_addr)) != 0)
        {
            puts("Incorrect endpoint");
            exit(EXIT_FAILURE);
        }
    } while (CLIENT_MODE);
}

int multiproto_client(void)
{
    puts("Client");

    /*
    * Declare:
    * - rlim - structure, storing soft and hard limits for maximum number of
    *   file descriptors;
    * - tmp_tid - pointer, used to temporary store pointer to reallocated
    *   'tcp_tids' array;
    * - sa - sigaction, used to redefine signal handler for SIGINT.
    */
    struct rlimit rlim;
    pthread_t *tmp_tid;
    struct sigaction sa;
    int sem_value;

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

    /* Allocate 'tcp_tids' array */
    tcp_tids = malloc(tcp_alloc_threads*sizeof(pthread_t));
    if (tcp_tids == NULL)
    {
        exit(EXIT_FAILURE);
    }
    udp_tids = malloc(udp_alloc_threads*sizeof(pthread_t));
    if (udp_tids == NULL)
    {
        exit(EXIT_FAILURE);
    }

    /* Create 'tcp_counter_sem' semaphore to store number of served clients */
    tcp_counter_sem = sem_open(CLIENT_TCP_COUNTER_SEM_NAME, O_CREAT | O_RDWR, 0666, 0);
    if (tcp_counter_sem == SEM_FAILED)
    {
        printf("sem_open: %s (%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    udp_counter_sem = sem_open(CLIENT_UDP_COUNTER_SEM_NAME, O_CREAT | O_RDWR, 0666, 0);
    if (udp_counter_sem == SEM_FAILED)
    {
        printf("sem_open: %s (%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    /* Reset sem if it is not equal to 0 */
    sem_getvalue(tcp_counter_sem, &sem_value);
	while (sem_value > 0)
	{
		sem_trywait(tcp_counter_sem);
		sem_getvalue(tcp_counter_sem, &sem_value);
	}
    /* Reset sem if it is not equal to 0 */
    sem_getvalue(udp_counter_sem, &sem_value);
	while (sem_value > 0)
	{
		sem_trywait(udp_counter_sem);
		sem_getvalue(udp_counter_sem, &sem_value);
	}

    /* Fill sa_mask, set signal handler, redefine SIGINT with sa */
    sigfillset(&sa.sa_mask); 
    sa.sa_sigaction = sigint_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        printf("sigaction: %s (%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    if (atexit(shutdown_client) != 0)
    {
        printf("atexit: %s (%d)\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    /*
    * Loop, running while 'create_flag' is not 0, it creates client threads
    * and allocates more memory to 'tcp_tids' array if needed.
    * */
	while(1)
    {
        /*
        * Allocate more memory to 'tcp_tids' array if it's limit has been reached.
        */
        if (tcp_threads_count >= tcp_alloc_threads)
        {
            tcp_alloc_threads += CLIENT_DEF_ALLOC;
            tmp_tid = realloc(tcp_tids, ((tcp_alloc_threads+CLIENT_DEF_ALLOC)*sizeof(pthread_t)));
            if (tmp_tid == NULL)
            {
                printf("realloc: %s (%d)\n", strerror(errno), errno);
                break;
            }
            tcp_tids = tmp_tid;
            tmp_tid = NULL;
        }
        pthread_create(&tcp_tids[tcp_threads_count], NULL, tcp_client, NULL);
        tcp_threads_count++;

        /*
        * Allocate more memory to 'udp_tids' array if it's limit has been reached.
        */
        if (udp_threads_count >= udp_alloc_threads)
        {
            udp_alloc_threads += CLIENT_DEF_ALLOC;
            tmp_tid = realloc(udp_tids, (udp_alloc_threads*sizeof(pthread_t)));
            if (tmp_tid == NULL)
            {
                printf("realloc: %s (%d)\n", strerror(errno), errno);
                break;
            }
            udp_tids = tmp_tid;
            tmp_tid = NULL;
        }
        pthread_create(&udp_tids[udp_threads_count], NULL, udp_client, NULL);
        udp_threads_count++;
    }

	exit(EXIT_SUCCESS);
}