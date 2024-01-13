#include "../hdr/functions.h"

/*
* Declare global variable:
* - server - server's endpoint;
* - tid - dynamic array of thread ids;
* - clients_count - actual size of clients, served by server;
* - alloc_threads - currently allocated size of 'tid' array.
*/
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// struct sockaddr_in server;
pthread_t *tid;
unsigned int threads_count = 0;
unsigned int clients_count = 0;
int alloc_threads = CLIENT_DEF_ALLOC;
int stop_flag = 0;

static void sigint_handler(int sig, siginfo_t *si, void *unused)
{
    printf("%s in %d\n", __func__, getpid());
    stop_flag = 1;

    int index;

    for (index = 0; index < threads_count; ++index)
    {
        // printf("pthread cancel %d/%d\n", index, threads_count);
        if (pthread_cancel(tid[index]) == -1)
        // if (pthread_join(tid[index], NULL) == -1)
        {
            perror("pthread_cancel");
        }
    }

    exit(EXIT_SUCCESS);
}

/* Function provided to atexit() */
void shut_client(void)
{
    printf("%s in %d\n", __func__, getpid());
    
    

    printf("exit fn: %d\n", clients_count);

    if (tid != NULL)
    {
        free(tid);
    }

	printf("Shut clients at %d client\n", clients_count);
}

/*  */
 void *client_ops(void *args)
{
    /*
    * Declare global variable:
    * - server_fd - fd of server socket;
    * - msg - message buffer.
    */
    struct sockaddr_in server;
    int id = (int)args;
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
		exit(EXIT_FAILURE);
	}
	server.sin_port = SERVER_TCP_PORT;

    /* Create socket */
	server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (stop_flag)
        return;

    if (connect(server_fd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        // perror("connect");
        // char *str_ptr = strerror(errno);
        printf("connect: %s at %d\n", strerror(errno), clients_count);
        // stop_flag = 1;
        // printf("Failed at client #%d\n", id);
        // exit(EXIT_FAILURE);
        raise(SIGINT);
        // return EXIT_FAILURE;
    }

	/* Wait for message from server */
	if (recv(server_fd, msg, MSG_SIZE, 0) == -1)
	{
		// perror("recv");
        char *str_ptr = strerror(errno);
        printf("recv: %s at %d\n", str_ptr, clients_count);
        // stop_flag = 1;
		exit(EXIT_FAILURE);
        // return EXIT_FAILURE;
	}

	/* Send message to server */
	strncpy(msg, CLIENT_MSG, MSG_SIZE);
	if (send(server_fd, msg, MSG_SIZE, 0) == -1)
	{
		// perror("send");
        char *str_ptr = strerror(errno);
        printf("send: %s at %d\n", str_ptr, clients_count);
        // stop_flag = 1;
		exit(EXIT_FAILURE);
        // return EXIT_FAILURE;
	}
    pthread_mutex_lock(&mutex);
    clients_count++;
    // printf("client op #%d\n", clients_count);
    pthread_mutex_unlock(&mutex);
}

int client(void)
{
    printf("%s in %d\n", __func__, getpid());
    /*
    * Declare:
    * - sa - sigaction, used to redefine signal handler for SIGINT.
    */
    struct sigaction sa;

    tid = malloc(sizeof(pthread_t)*alloc_threads);

    sigfillset(&sa.sa_mask);
    sa.sa_sigaction = sigint_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    /* Set atexit function */
	atexit(shut_client);

	// /* Fill 'server' with 0's */
	// memset(&server, 0, sizeof(server));

	// /* Set server's endpoint */
	// server.sin_family = AF_INET;
	// if (inet_pton(AF_INET, SERVER_ADDR, &server.sin_addr) == -1)
	// {
	// 	perror("inet_pton");
	// 	exit(EXIT_FAILURE);
	// }
	// server.sin_port = SERVER_TCP_PORT;

	while(1)
    {
        // printf("%d\n", threads_count);
        if (threads_count >= alloc_threads)
        {
            pthread_t *tmp_tid;
            alloc_threads += CLIENT_DEF_ALLOC;

            // printf("realoccing to %d clients (%d)\n", alloc_threads, (sizeof(pthread_t)*(alloc_threads)));
            

            tmp_tid = realloc(tid, (sizeof(pthread_t)*(alloc_threads)));
            if (tmp_tid == NULL)
            {
                perror("realloc1");
                exit(EXIT_FAILURE);
            }
            tid  = tmp_tid;


            
        }

        /* Create thread */
        // if (stop_flag)
        // {
        //     break;
        // }
        pthread_create(&tid[threads_count], NULL, client_ops, (void *)clients_count);

        threads_count++;
    }

	exit(EXIT_SUCCESS);
}