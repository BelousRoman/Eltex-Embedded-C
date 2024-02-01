#include "../hdr/functions.h"

/*
* Declare global variables:
* - tcp_tids & udp_tids - dynamic arrays of thread ids;
* - create_flag - flag for the main thread to continue creating threads;
* - tcp_alloc_threads & udp_alloc_threads - currently allocated elements in 'tcp_tids' and 'udp_tids' arrays;
* - tcp_threads_count & udp_threads_count - number of created threads;
* - counter_sem - semaphore to store number of clients, served by server;
* - clients_count - variable, passed to sem_getvalue()
*   to get number of clients, served by server.
*/
pthread_t *tcp_tids;
pthread_t *udp_tids;
int create_flag = 1;
int tcp_alloc_threads = CLIENT_DEF_ALLOC;
int udp_alloc_threads = CLIENT_DEF_ALLOC;
unsigned int tcp_threads_count = 0;
unsigned int udp_threads_count = 0;
sem_t * counter_sem = NULL;
int clients_count = 0;

/* handler function for SIGINT signal */
static void sigint_handler(int sig, siginfo_t *si, void *unused)
{
    create_flag = 0;
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
    char msg[MSG_SIZE];

    /* Set canceltype so thread could be canceled at any time*/
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    /* Fill 'server' with 0's */
	memset(&server, 0, sizeof(server));

	/* Set server's endpoint */
	server.sin_family = AF_INET;
	if (inet_pton(AF_INET, SERVER_ADDR, &server.sin_addr) == -1)
	{
		perror("inet_pton");
        create_flag = 0;
		return;
	}
	server.sin_port = SERVER_TCP_PORT;

    /* Create socket */
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        printf("socket: %s (%d), threads count: %d\n", strerror(errno), errno, tcp_threads_count);
        create_flag = 0;
        return;
    }

    /* Connect to server */
    if (connect(server_fd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        printf("connect: %s (%d), threads count: %d\n", strerror(errno), errno, tcp_threads_count);
        create_flag = 0;
        return;
    }

	/* Wait for message from server */
	if (recv(server_fd, msg, MSG_SIZE, 0) == -1)
	{
		perror("recv");
		create_flag = 0;
        return;
	}

	/* Send message to server */
	strncpy(msg, CLIENT_MSG, MSG_SIZE);
	if (send(server_fd, msg, MSG_SIZE, 0) == -1)
	{
		perror("send");
		create_flag = 0;
        return;
	}

    /* Increment served clients counter */
    sem_post(counter_sem);
}

/* UDP Client thread function */
void *udp_client(void *args)
{
    /*
    * Declare global variable:
    * - server - server's endpoint;
    * - server_fd - fd of server socket;
    * - msg - message buffer.
    */
    struct sockaddr_in server;
    int server_fd;
    char msg[MSG_SIZE];

    /* Set canceltype so thread could be canceled at any time*/
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    /* Fill 'server' with 0's */
	memset(&server, 0, sizeof(server));

	/* Set server's endpoint */
	server.sin_family = AF_INET;
	if (inet_pton(AF_INET, SERVER_ADDR, &server.sin_addr) == -1)
	{
		perror("inet_pton");
        create_flag = 0;
		return;
	}
	server.sin_port = SERVER_UDP_PORT;

    /* Create socket */
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        printf("socket: %s (%d), threads count: %d\n", strerror(errno), errno, udp_threads_count);
        create_flag = 0;
        return;
    }

    /* Connect to server */
    if (connect(server_fd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        printf("connect: %s (%d), threads count: %d\n", strerror(errno), errno, udp_threads_count);
        create_flag = 0;
        return;
    }

	/* Wait for message from server */
	if (recv(server_fd, msg, MSG_SIZE, 0) == -1)
	{
		perror("recv");
		create_flag = 0;
        return;
	}

	/* Send message to server */
	strncpy(msg, CLIENT_MSG, MSG_SIZE);
	if (send(server_fd, msg, MSG_SIZE, 0) == -1)
	{
		perror("send");
		create_flag = 0;
        return;
	}

    /* Increment served clients counter */
    sem_post(counter_sem);
}

int client(void)
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

    /* Print current rlimit, set new one */
    if (getrlimit(RLIMIT_NOFILE, &rlim) == -1)
    {
        perror("getrlimit");
        exit(EXIT_FAILURE);
    }
    printf("Current maximum file descriptor: %ld / %ld\n", rlim.rlim_cur, rlim.rlim_max);
    rlim.rlim_cur = rlim.rlim_max;
    printf("New maximum file descriptor: %ld / %ld\n", rlim.rlim_cur, rlim.rlim_max);
    if (setrlimit(RLIMIT_NOFILE, &rlim) == -1)
    {
        perror("setrlimit");
        exit(EXIT_FAILURE);
    }

    /* Allocate 'tcp_tids' array */
    tcp_tids = malloc(tcp_alloc_threads*sizeof(pthread_t));
    if (tcp_tids == NULL)
    {
        exit(EXIT_FAILURE);
    }

    /* Create 'counter_sem' semaphore to store number of served clients */
    counter_sem = sem_open(CLIENT_COUNTER_SEM_NAME, O_CREAT | O_RDWR, 0666, 0);
    if (counter_sem == SEM_FAILED)
    {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    /* Reset sem if it is not equal to 0 */
    sem_getvalue(counter_sem, &clients_count);
	while (clients_count > 0)
	{
		sem_trywait(counter_sem);
		sem_getvalue(counter_sem, &clients_count);
	}

    /* Fill sa_mask, set signal handler, redefine SIGINT with sa */
    sigfillset(&sa.sa_mask);
    sa.sa_sigaction = sigint_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    /*
    * Loop, running while 'create_flag' is not 0, it creates client threads
    * and allocates more memory to 'tcp_tids' array if needed.
    * */
	while(create_flag)
    {
        /*
        * Allocate more memory to 'tcp_tids' array if it's limit has been reached.
        */
        if (tcp_threads_count >= tcp_alloc_threads)
        {
            tcp_alloc_threads += CLIENT_DEF_ALLOC;

            tmp_tid = realloc(tcp_tids, (tcp_alloc_threads*sizeof(pthread_t)));
            if (tmp_tid == NULL)
            {
                perror("realloc");
                break;
            }
            tcp_tids  = tmp_tid;
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
                perror("realloc");
                break;
            }
            udp_tids  = tmp_tid;
        }
        pthread_create(&tcp_tids[tcp_threads_count], NULL, udp_client, NULL);
        udp_threads_count++;
    }
    puts("freeing resources");
    /* Free memory, allocated to 'tcp_tids' array */
    if (tcp_tids != NULL)
    {
        free(tcp_tids);
    }
    if (udp_tids != NULL)
    {
        free(udp_tids);
    }
    puts("freed");
    /* Get clients count */
    sem_getvalue(counter_sem, &clients_count);
    sem_close(counter_sem);
    unlink(CLIENT_COUNTER_SEM_NAME);
    
	printf("Shut clients at:\n* %d client,\n* %d threads created\n", clients_count, (tcp_threads_count+udp_threads_count));

	exit(EXIT_SUCCESS);
}