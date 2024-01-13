#include "../hdr/functions.h"

/*
* Declare:
* - tid - dynamic array of thread ids;
* - clients_count - actual size of clients, served by server;
* - alloc_servers - currently allocated size of 'tid' array;
* - server_fd - fd of server socket.
*/
pthread_t *tid = NULL;
unsigned int clients_count = 0;
unsigned int alloc_servers = SERVER_DEF_ALLOC;
int server_fd;

/* Signal handler for SIGINT */
static void sigint_handler(int sig, siginfo_t *si, void *unused)
{
    exit(EXIT_SUCCESS);
}

/* Function provided to atexit() */
void shutdown_server(void)
{
    close(server_fd);

	puts("Server shutdown");
}

int second_task(void)
{
	puts("Second task");

	/*
	* Declare:
    * - sa - sigaction, used to redefine signal handler for SIGINT;
	* - server & client - server's and client's endpoints;
	* - client_fd - fd of client socket;
	* - client_size - size of client's endpoint;
	* - msg - message buffer;
	* - clients_count - number of served clients.
	*/
    struct sigaction sa;
	struct sockaddr_in server, client;
    int client_fd;
	int client_size;
	char msg[MSG_SIZE];
	unsigned int clients_count = 0;

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

	/* Create socket */
	server_fd = socket(AF_INET, SOCK_STREAM, 0);

	/* Bind server's endpoint to socket */
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

		/* Send message to client */
		strncpy(msg, SERVER_MSG, MSG_SIZE);
		if (send(client_fd, msg, MSG_SIZE, 0) == -1)
		{
			perror("send");
			exit(EXIT_FAILURE);
		}

		/* Wait for message from client */
		if (recv(client_fd, msg, MSG_SIZE, 0) == -1)
		{
			perror("recv");
			exit(EXIT_FAILURE);
		}
		else
		{
			clients_count++;
			printf("Clients served: %d\r", clients_count);
		}
		close(client_fd);
	}

	return EXIT_SUCCESS;
}