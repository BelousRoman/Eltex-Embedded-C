#include "../hdr/functions.h"

/*
* Declare global variables:
* - tid - dynamic array of thread ids;
* - create_flag - flag for the main thread to continue creating threads;
* - alloc_threads - currently allocated elements in 'tid' array;
* - threads_count - number of created threads;
* - counter_sem - semaphore to store number of clients, served by server;
* - clients_count - variable, passed to sem_getvalue()
*   to get number of clients, served by server.
*/
pthread_t *tid;
int create_flag = 1;
int alloc_threads = CLIENT_DEF_ALLOC;
unsigned int threads_count = 0;
sem_t * counter_sem = NULL;
int clients_count = 0;

/* handler function for SIGINT signal */
static void sigint_handler(int sig, siginfo_t *si, void *unused)
{
    create_flag = 0;
}

/* client thread function */
void *client_ops(void *args)
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
        printf("socket: %s (%d), threads count: %d\n", strerror(errno), errno, threads_count);
        create_flag = 0;
        return;
    }

    /* Connect to server */
    if (connect(server_fd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        printf("connect: %s (%d), threads count: %d\n", strerror(errno), errno, threads_count);
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
    * - tmp_tid - pointerm, used to temporary store pointer to reallocated
    *   'tid' array;
    * 
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

    /* Allocate 'tid' array */
    tid = malloc(alloc_threads*sizeof(pthread_t));
    if (tid == NULL)
    {
        exit(EXIT_FAILURE);
    }

    /* Client 'counter_sem' semaphore to store number of served clients */
    counter_sem = sem_open(CLIENT_COUNTER_SEM_NAME, O_CREAT | O_RDWR, 0666, 0);
    if (counter_sem == SEM_FAILED)
    {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

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
    * and allocates more memory to 'tid' array if needed.
    * */
	while(create_flag)
    {
        if (threads_count >= alloc_threads)
        {
            alloc_threads += CLIENT_DEF_ALLOC;

            tmp_tid = realloc(tid, (alloc_threads*sizeof(pthread_t)));
            if (tmp_tid == NULL)
            {
                perror("realloc");
                break;
            }
            tid  = tmp_tid;
        }
        pthread_create(&tid[threads_count], NULL, client_ops, NULL);

        threads_count++;
    }

    /* Free memory, allocated to 'tid' array */
    if (tid != NULL)
    {
        free(tid);
    }
    puts("memory freed");

    /* Get clients count */
    sem_getvalue(counter_sem, &clients_count);
    sem_close(counter_sem);
    unlink(CLIENT_COUNTER_SEM_NAME);
    
	printf("Shut clients at:\n* %d client,\n* %d threads created\n", clients_count, threads_count);

	exit(EXIT_SUCCESS);
}