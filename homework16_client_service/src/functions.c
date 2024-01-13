// #include "../hdr/functions.h"

// int first_task(void)
// {
// 	puts("First task");

// 	/*
// 	* Declare:
// 	* - server & client - server's and client's endpoints;
// 	* - server_fd - server fd;
// 	* - client_fd - client fd
// 	* - client_size - size of client's endpoint;
// 	* - msg - message buffer;
// 	* - clients_count - number of served clients.
// 	*/
// 	struct sockaddr_in server, client;
// 	int server_fd, client_fd;
// 	int client_size;
// 	char msg[T3_MSG_SIZE];
// 	unsigned int clients_count = 0;

// 	/* Fill 'server' with 0's */
// 	memset(&server, 0, sizeof(server));

// 	/* Set server's endpoint */
// 	server.sin_family = T3_PROTO_FAMILY;
// 	if (inet_pton(T3_PROTO_FAMILY, T3_SERV_ADDR, &server.sin_addr) == -1)
// 	{
// 		perror("inet_pton");
// 		exit(EXIT_FAILURE);
// 	}
// 	server.sin_port = T3_SERV_PORT;

// 	/* Create socket */
// 	server_fd = socket(T3_PROTO_FAMILY, SOCK_STREAM, 0);

// 	/* Bind server's endpoint to socket */
// 	if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) == -1)
// 	{
// 		perror("bind");
// 		exit(EXIT_FAILURE);
// 	}

// 	/*
// 	* TCP protocol only function, mark 'server_fd' socket as passive
// 	* socket.
// 	*/
// 	if (listen(server_fd, T3_LISTEN_BACKLOG) == -1)
// 	{
// 		perror("listen");
// 		exit(EXIT_FAILURE);
// 	}

// 	while(1)
// 	{
// 		/*
// 		* TCP protocol only function, await connection and get client's
// 		* endpoint and fd.
// 		*/
// 		client_size = sizeof(client);
// 		if ((client_fd = accept(server_fd, (struct sockaddr *)&client,
// 								&client_size)) == -1)
// 		{
// 			perror("accept");
// 			exit(EXIT_FAILURE);
// 		}

// 		/* Send message to client */
// 		strncpy(msg, T3_SERVER_MSG, T3_MSG_SIZE);
// 		if (send(client_fd, msg, T3_MSG_SIZE, 0) == -1)
// 		{
// 			perror("send");
// 			exit(EXIT_FAILURE);
// 		}

// 		/* Wait for message from client */
// 		if (recv(client_fd, msg, T3_MSG_SIZE, 0) == -1)
// 		{
// 			perror("recv");
// 			exit(EXIT_FAILURE);
// 		}
// 		else
// 		{
// 			clients_count++;
// 			printf("Clients served: %d\r", clients_count);
// 		}
// 		close(client_fd);
// 	}

// 	return EXIT_SUCCESS;
// }

// int first_task(void)
// {
// 	puts("First task");

// 	/*
// 	* Declare:
// 	* - pid - process id of child process.
// 	*/
// 	pid_t pid;

// 	pid = fork();
// 	if (pid == 0)
// 	{
// 		/*
// 		* Declare:
// 		* - server - server's endpoint;
// 		* - server_fd - socket fd;
// 		* - msg - message buffer;
// 		* - index - index variable;
// 		* - ts - timespec, used in multiple connection attempts.
// 		*/
// 		struct sockaddr_in server;
// 		int server_fd;
// 		char msg[T3_MSG_SIZE];
// 		int index;
// 		struct timespec ts;

// 		/* Set 'ts' time to define-constants */
// 		ts.tv_sec = T3_RETRY_DELAY_SEC;
// 		ts.tv_nsec = T3_RETRY_DELAY_NSEC;

// 		/* Fill 'server' with 0's */
// 		memset(&server, 0, sizeof(server));

// 		/* Set server's endpoint */
// 		server.sin_family = T3_PROTO_FAMILY;
// 		if (inet_pton(T3_PROTO_FAMILY, T3_SERV_ADDR, &server.sin_addr) == -1)
// 		{
// 			perror("inet_pton");
// 			exit(EXIT_FAILURE);
// 		}
// 		server.sin_port = T3_SERV_PORT;

// 		/* Create socket */
// 		server_fd = socket(T3_PROTO_FAMILY, SOCK_STREAM, 0);

// 		/*
// 		* Try to connect to server for T1_CONN_RETRIES with delay, set in 'ts',
// 		* if connect fails with ECONNREFUSED or ENOENT, otherwise exit with
// 		* failure.
// 		*/
// 		for (index = 0; index < T3_CONN_RETRIES; ++index)
// 		{
// 			if (connect(server_fd, (struct sockaddr *)&server, sizeof(server))
// 				== -1)
// 			{
// 				perror("client connect");
// 				if ((errno != ECONNREFUSED && errno != ENOENT) ||
// 					index == (T3_CONN_RETRIES - 1))
// 				{
// 					exit(EXIT_FAILURE);
// 				}
// 			}
// 			else
// 			{
// 				break;
// 			}
// 			nanosleep(&ts, NULL);
// 		}

// 		/* Wait for message from server */
// 		if (recv(server_fd, msg, T3_MSG_SIZE, 0) == -1)
// 		{
// 			perror("Client recv");
// 			exit(EXIT_FAILURE);
// 		}
// 		else
// 			printf("Client received: %s\n", msg);

// 		/* Send message to server */
// 		strncpy(msg, T3_CLIENT_MSG, T3_MSG_SIZE);
// 		if (send(server_fd, msg, T3_MSG_SIZE, 0) == -1)
// 		{
// 			perror("Client send");
// 			exit(EXIT_FAILURE);
// 		}

// 		exit(EXIT_SUCCESS);
// 	}
// 	else
// 	{
// 		/*
// 		* Declare:
// 		* - server & client - server's and client's endpoints;
// 		* - server_fd - server fd;
// 		* - client_fd - client fd
// 		* - client_size - size of client's endpoint;
// 		* - msg - message buffer.
// 		*/
// 		struct sockaddr_in server, client;
// 		int server_fd, client_fd;
// 		int client_size;
// 		char msg[T3_MSG_SIZE];

// 		/* Fill 'server' with 0's */
// 		memset(&server, 0, sizeof(server));

// 		/* Set server's endpoint */
// 		server.sin_family = T3_PROTO_FAMILY;
// 		if (inet_pton(T3_PROTO_FAMILY, T3_SERV_ADDR, &server.sin_addr) == -1)
// 		{
// 			perror("inet_pton");
// 			exit(EXIT_FAILURE);
// 		}
// 		server.sin_port = T3_SERV_PORT;

// 		/* Create socket */
// 		server_fd = socket(T3_PROTO_FAMILY, SOCK_STREAM, 0);

// 		/* Bind server's endpoint to socket */
// 		if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) == -1)
// 		{
// 			perror("bind");
// 			exit(EXIT_FAILURE);
// 		}

// 		/*
// 		* TCP protocol only function, mark 'server_fd' socket as passive
// 		* socket.
// 		*/
// 		if (listen(server_fd, T3_LISTEN_BACKLOG) == -1)
// 		{
// 			perror("listen");
// 			exit(EXIT_FAILURE);
// 		}

// 		/*
// 		* TCP protocol only function, await connection and get client's
// 		* endpoint and fd.
// 		*/
// 		client_size = sizeof(client);
// 		if ((client_fd = accept(server_fd, (struct sockaddr *)&client,
// 								&client_size)) == -1)
// 		{
// 			perror("accept");
// 			exit(EXIT_FAILURE);
// 		}

// 		/* Send message to client */
// 		strncpy(msg, T3_SERVER_MSG, T3_MSG_SIZE);
// 		if (send(client_fd, msg, T3_MSG_SIZE, 0) == -1)
// 		{
// 			perror("server send");
// 			exit(EXIT_FAILURE);
// 		}

// 		/* Wait for message from client */
// 		if (recv(client_fd, msg, T3_MSG_SIZE, 0) == -1)
// 		{
// 			perror("server recv");
// 			exit(EXIT_FAILURE);
// 		}
// 		else
// 			printf("Server received: %s\n", msg);

// 		/* Wait for child process to finish */
// 		wait(NULL);

// 		/* Close socket fds */
// 		if (close(client_fd) == -1)
// 		{
// 			perror("close(client_fd)");
// 		}
// 		if (close(server_fd) == -1)
// 		{
// 			perror("close(server_fd)");
// 		}
// 	}

// 	return EXIT_SUCCESS;
// }