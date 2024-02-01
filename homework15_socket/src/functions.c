#include "../hdr/functions.h"

int first_task(void)
{
	puts("First task");

	/*
	* Declare:
	* - pid - process id of child process.
	*/
	pid_t pid;

	pid = fork();
	if (pid == 0)
	{
		/*
		* Declare:
		* - server - server's endpoint;
		* - server_fd - socket fd;
		* - msg - message buffer;
		* - index - index variable;
		* - ts - timespec, used in multiple connection attempts.
		*/
		struct sockaddr_un server;
		int server_fd;
		char msg[T1_MSG_SIZE];
		int index;
		struct timespec ts;

		/* Set 'ts' time to define-constants */
		ts.tv_sec = T1_RETRY_DELAY_SEC;
		ts.tv_nsec = T1_RETRY_DELAY_NSEC;

		/* Fill 'server' with 0's */
		memset(&server, 0, sizeof(server));

		/* Set server's endpoint */
		server.sun_family = AF_LOCAL;
		strncpy(server.sun_path, T1_SUN_PATH, sizeof(server.sun_path) - 1);

		/* Create socket */
		server_fd = socket(AF_LOCAL, SOCK_STREAM, 0);

		/*
		* Try to connect to server for T1_CONN_RETRIES with delay, set in 'ts',
		* if connect fails with ECONNREFUSED or ENOENT, otherwise exit with
		* failure.
		*/
		for (index = 0; index < T1_CONN_RETRIES; ++index)
		{
			if (connect(server_fd, (struct sockaddr *)&server, sizeof(server))
				== -1)
			{
				perror("client connect");
				if ((errno != ECONNREFUSED && errno != ENOENT) ||
					index == (T1_CONN_RETRIES - 1))
				{
					printf("%d\n", errno);
					exit(EXIT_FAILURE);
				}
			}
			else
			{
				break;
			}
			nanosleep(&ts, NULL);
		}

		/* Wait for message from server */
		if (recv(server_fd, msg, T1_MSG_SIZE, 0) == -1)
		{
			perror("Client recv");
			exit(EXIT_FAILURE);
		}
		else
			printf("Client received: %s from: %s\n", msg, server.sun_path);

		/* Send message to server */
		strncpy(msg, T1_CLIENT_MSG, T1_MSG_SIZE);
		if (send(server_fd, msg, T1_MSG_SIZE, 0) == -1)
		{
			perror("Client send");
			exit(EXIT_FAILURE);
		}

		exit(EXIT_SUCCESS);
	}
	else
	{
		/*
		* Declare:
		* - server & client - server's and client's endpoints;
		* - server_fd - server fd;
		* - client_fd - client fd
		* - client_size - size of client's endpoint;
		* - msg - message buffer.
		*/
		struct sockaddr_un server, client;
		int server_fd, client_fd;
		int client_size;
		char msg[T1_MSG_SIZE];

		/* Fill 'server' with 0's */
		memset(&server, 0, sizeof(server));

		/* Set server's endpoint */
		server.sun_family = AF_LOCAL;
		strncpy(server.sun_path, T1_SUN_PATH, sizeof(server.sun_path) - 1);

		/* Create socket */
		server_fd = socket(AF_LOCAL, SOCK_STREAM, 0);

		/*
		* Try to bind server's endpoint to socket, unlink specified sun_path if
		* errno is EADDRINUSE, otherwise exit with failure.
		*/
		while (bind(server_fd, (struct sockaddr *)&server, sizeof(server))
				== -1)
		{
			if (errno == EADDRINUSE)
			{
				if (unlink(server.sun_path) == -1)
				{
					perror("unlink");
					exit(EXIT_FAILURE);
				}
			}
			else
			{
				perror("bind");
				exit(EXIT_FAILURE);
			}
		}

		/*
		* TCP protocol only function, mark 'server_fd' socket as passive
		* socket.
		*/
		if (listen(server_fd, T1_LISTEN_BACKLOG) == -1)
		{
			perror("listen");
			exit(EXIT_FAILURE);
		}

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
		strncpy(msg, T1_SERVER_MSG, T1_MSG_SIZE);
		if (send(client_fd, msg, T1_MSG_SIZE, 0) == -1)
		{
			perror("server send");
			exit(EXIT_FAILURE);
		}

		/* Wait for message from client */
		if (recv(client_fd, msg, T1_MSG_SIZE, 0) == -1)
		{
			perror("server recv");
			exit(EXIT_FAILURE);
		}
		else
			printf("Server received: %s\n", msg);

		wait(NULL);

		/* Close fds, unlink socket file */
		if (close(client_fd) == -1)
		{
			perror("close(client_fd)");
		}
		if (close(server_fd) == -1)
		{
			perror("close(server_fd)");
		}

		if (unlink(server.sun_path) == -1)
		{
			perror("unlink(server.sun_path)");
		}
	}

	return EXIT_SUCCESS;
}

int second_task(void)
{
	puts("Second task");

	/*
	* Declare:
	* - pid - process id of child process.
	*/
	pid_t pid;

	pid = fork();
	if (pid == 0)
	{
		/*
		* Declare:
		* - server & client - server's and client's endpoints;
		* - server_fd - socket fd;
		* - msg - message buffer;
		* - index - index variable;
		* - ts - timespec, used in multiple connect & bind attempts.
		*/
		struct sockaddr_un server, client;
		int server_fd;
		char msg[T2_MSG_SIZE];
		int index;
		struct timespec ts;

		/* Set 'ts' time to define-constants */
		ts.tv_sec = T2_RETRY_DELAY_SEC;
		ts.tv_nsec = T2_RETRY_DELAY_NSEC;

		/* Fill 'server' & 'client' with 0's */
		memset(&server, 0, sizeof(server));
		memset(&client, 0, sizeof(client));

		/* Set server's and client's endpoints */
		server.sun_family = AF_LOCAL;
		strncpy(server.sun_path, T2_SERVER_SUN_PATH,
				sizeof(server.sun_path) - 1);
		client.sun_family = AF_LOCAL;
		strncpy(client.sun_path, T2_CLIENT_SUN_PATH,
				sizeof(client.sun_path) - 1);

		/* Create socket */
		server_fd = socket(AF_LOCAL, SOCK_DGRAM, 0);

		/* Try to bind client's endpoint to socket */
		for (index = 0; index < T2_CONN_RETRIES; ++index)
		{
			if (bind(server_fd, &client, sizeof(struct sockaddr_un)) == -1)
			{
				perror("client bind");
				if (index >= (T2_CONN_RETRIES-1))
				{
					exit(EXIT_FAILURE);
				}
			}
			else
				break;
			nanosleep(&ts, NULL);
		}

		/*
		* Try to connect to server for T1_CONN_RETRIES with delay, set in 'ts',
		* if connect fails with ECONNREFURES, otherwise exit with failure.
		*/
		for (index = 0; index < T2_CONN_RETRIES; ++index)
		{
			if (connect(server_fd, (struct sockaddr *)&server, sizeof(server))
				== -1)
			{
				perror("client connect");
				if (errno != ECONNREFUSED || index == (T2_CONN_RETRIES - 1))
				{
					exit(EXIT_FAILURE);
				}
			}
			else
			{
				break;
			}
			nanosleep(&ts, NULL);
		}

		/* Send message to server */
		strncpy(msg, T2_CLIENT_MSG, T2_MSG_SIZE);
		if (send(server_fd, msg, T2_MSG_SIZE, 0) == -1)
		{
			perror("Client send");
			exit(EXIT_FAILURE);
		}

		/* Wait for message from server */
		if (recv(server_fd, msg, T2_MSG_SIZE, 0) == -1)
		{
			perror("Client recv");
			exit(EXIT_FAILURE);
		}
		else
			printf("Client received: %s from %s\n", msg, server.sun_path);

		exit(EXIT_SUCCESS);
	}
	else
	{
		/*
		* Declare:
		* - server & client - server's and client's endpoints;
		* - server_fd - socket fd;
		* - client_size - size of client's endpoint;
		* - msg - message buffer.
		*/
		struct sockaddr_un server, client;
		int server_fd;
		int client_size;
		char msg[T2_MSG_SIZE];

		/* Fill 'server' & 'client' with 0's */
		memset(&server, 0, sizeof(server));

		/* Set server's endpoint */
		server.sun_family = AF_LOCAL;
		strncpy(server.sun_path, T2_SERVER_SUN_PATH,
				sizeof(server.sun_path) - 1);

		/* Create socket */
		server_fd = socket(AF_LOCAL, SOCK_DGRAM, 0);

		/* Try to bind server's endpoint to socket */
		while (bind(server_fd, (struct sockaddr *)&server, sizeof(server))
				== -1)
		{
			if (errno == EADDRINUSE)
			{
				if (unlink(server.sun_path) == -1)
				{
					perror("unlink");
					exit(EXIT_FAILURE);
				}
			}
			else
			{
				perror("bind");
				exit(EXIT_FAILURE);
			}
		}

		/* Wait for message from client, get client's endpoint */
		client_size = sizeof(client);
		while (recvfrom(server_fd, msg, T2_MSG_SIZE, 0,
						(struct sockaddr *)&client, &client_size) == -1)
		{
			if (errno != EWOULDBLOCK)
			{
				perror("server recvfrom");
				exit(EXIT_FAILURE);
			}
		}
		printf("Server received: %s from: %s\n", msg, client.sun_path);

		/* Send message to client */
		strncpy(msg, T2_SERVER_MSG, T2_MSG_SIZE);
		if (sendto(server_fd, msg, T2_MSG_SIZE, MSG_DONTWAIT,
					(struct sockaddr *)&client, client_size) == -1)
		{
			perror("server sendto");
			exit(EXIT_FAILURE);
		}

		/* Wait for child process to finish */
		wait(NULL);

		/* Close fds, unlink socket files */
		if (close(server_fd) == -1)
		{
			perror("close(server_fd)");
		}
		if (unlink(server.sun_path) == -1)
		{
			perror("unlink(server.sun_path)");
		}
		if (unlink(client.sun_path) == -1)
		{
			perror("unlink(client.sun_path)");
		}
	}

	return EXIT_SUCCESS;
}

int third_task(void)
{
	puts("Third task");

	/*
	* Declare:
	* - pid - process id of child process.
	*/
	pid_t pid;

	pid = fork();
	if (pid == 0)
	{
		/*
		* Declare:
		* - server - server's endpoint;
		* - server_fd - socket fd;
		* - msg - message buffer;
		* - index - index variable;
		* - ts - timespec, used in multiple connection attempts.
		*/
		struct sockaddr_in server;
		int server_fd;
		char msg[T3_MSG_SIZE];
		int index;
		struct timespec ts;

		/* Set 'ts' time to define-constants */
		ts.tv_sec = T3_RETRY_DELAY_SEC;
		ts.tv_nsec = T3_RETRY_DELAY_NSEC;

		/* Fill 'server' with 0's */
		memset(&server, 0, sizeof(server));

		/* Set server's endpoint */
		server.sin_family = T3_PROTO_FAMILY;
		if (inet_pton(T3_PROTO_FAMILY, T3_SERV_ADDR, &server.sin_addr) == -1)
		{
			perror("inet_pton");
			exit(EXIT_FAILURE);
		}
		server.sin_port = T3_SERV_PORT;

		/* Create socket */
		server_fd = socket(T3_PROTO_FAMILY, SOCK_STREAM, 0);

		/*
		* Try to connect to server for T1_CONN_RETRIES with delay, set in 'ts',
		* if connect fails with ECONNREFUSED or ENOENT, otherwise exit with
		* failure.
		*/
		for (index = 0; index < T3_CONN_RETRIES; ++index)
		{
			if (connect(server_fd, (struct sockaddr *)&server, sizeof(server))
				== -1)
			{
				perror("client connect");
				if ((errno != ECONNREFUSED && errno != ENOENT) ||
					index == (T3_CONN_RETRIES - 1))
				{
					exit(EXIT_FAILURE);
				}
			}
			else
			{
				break;
			}
			nanosleep(&ts, NULL);
		}

		/* Wait for message from server */
		if (recv(server_fd, msg, T3_MSG_SIZE, 0) == -1)
		{
			perror("Client recv");
			exit(EXIT_FAILURE);
		}
		else
			printf("Client received: %s\n", msg);

		/* Send message to server */
		strncpy(msg, T3_CLIENT_MSG, T3_MSG_SIZE);
		if (send(server_fd, msg, T3_MSG_SIZE, 0) == -1)
		{
			perror("Client send");
			exit(EXIT_FAILURE);
		}

		exit(EXIT_SUCCESS);
	}
	else
	{
		/*
		* Declare:
		* - server & client - server's and client's endpoints;
		* - server_fd - server fd;
		* - client_fd - client fd
		* - client_size - size of client's endpoint;
		* - msg - message buffer.
		*/
		struct sockaddr_in server, client;
		int server_fd, client_fd;
		int client_size;
		char msg[T3_MSG_SIZE];

		/* Fill 'server' with 0's */
		memset(&server, 0, sizeof(server));

		/* Set server's endpoint */
		server.sin_family = T3_PROTO_FAMILY;
		if (inet_pton(T3_PROTO_FAMILY, T3_SERV_ADDR, &server.sin_addr) == -1)
		{
			perror("inet_pton");
			exit(EXIT_FAILURE);
		}
		server.sin_port = T3_SERV_PORT;

		/* Create socket */
		server_fd = socket(T3_PROTO_FAMILY, SOCK_STREAM, 0);

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
		if (listen(server_fd, T3_LISTEN_BACKLOG) == -1)
		{
			perror("listen");
			exit(EXIT_FAILURE);
		}

		struct pollfd *pfds = malloc(2*sizeof(struct pollfd));
		pfds[1].fd = server_fd;
		pfds[1].events = POLLIN;
		pfds[0].fd = server_fd;
		pfds[0].events = POLLIN;
		int ret = poll(pfds, 2, 0);
		printf("poll ret = %d, revents = %d%d\n", ret, pfds[0].revents, pfds[1].revents);
		free(pfds);

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
		strncpy(msg, T3_SERVER_MSG, T3_MSG_SIZE);
		if (send(client_fd, msg, T3_MSG_SIZE, 0) == -1)
		{
			perror("server send");
			exit(EXIT_FAILURE);
		}

		/* Wait for message from client */
		if (recv(client_fd, msg, T3_MSG_SIZE, 0) == -1)
		{
			perror("server recv");
			exit(EXIT_FAILURE);
		}
		else
			printf("Server received: %s\n", msg);

		/* Wait for child process to finish */
		wait(NULL);

		/* Close socket fds */
		if (close(client_fd) == -1)
		{
			perror("close(client_fd)");
		}
		if (close(server_fd) == -1)
		{
			perror("close(server_fd)");
		}
	}

	return EXIT_SUCCESS;
}

int fourth_task(void)
{
	puts("Fourth task");

	/*
	* Declare:
	* - pid - process id of child process.
	*/
	pid_t pid;

	pid = fork();
	if (pid == 0)
	{
		/*
		* Declare:
		* - server & client - server's and client's endpoints;
		* - server_fd - socket fd;
		* - msg - message buffer;
		* - endp_addr - char buffer for server adress;
		* - index - index variable;
		* - ts - timespec, used in multiple connect & bind attempts.
		*/
		struct sockaddr_in server, client;
		int serv_fd;
		char msg[T4_MSG_SIZE], endp_addr[INET6_ADDRSTRLEN];
		int index;
		struct timespec ts;

		/* Set 'ts' time to define-constants */
		ts.tv_sec = T4_RETRY_DELAY_SEC;
		ts.tv_nsec = T4_RETRY_DELAY_NSEC;

		/* Fill 'server' with 0's */
		memset(&server, 0, sizeof(server));

		/* Set server's endpoint */
		server.sin_family = T4_PROTO_FAMILY;
		if (inet_pton(T4_PROTO_FAMILY, T4_SERV_ADDR, &server.sin_addr) == -1)
		{
			perror("Client inet_pton");
			exit(EXIT_FAILURE);
		}
		server.sin_port = htons(T4_SERV_PORT);

		/* Create socket */
		serv_fd = socket(T4_PROTO_FAMILY, SOCK_DGRAM, 0);

		/*
		* Try to connect to server for T1_CONN_RETRIES with delay, set in 'ts',
		* if connect fails with ECONNREFURES, otherwise exit with failure.
		*/
		for (index = 0; index < T4_CONN_RETRIES; ++index)
		{
			if (connect(serv_fd, (struct sockaddr *)&server, sizeof(server))
				== -1)
			{
				perror("Client connect");
				if (errno != ECONNREFUSED || index == (T4_CONN_RETRIES - 1))
				{
					exit(EXIT_FAILURE);
				}
			}
			else
			{
				break;
			}
			nanosleep(&ts, NULL);
		}

		/* Send message to server */
		strncpy(msg, T4_CLIENT_MSG, T4_MSG_SIZE);
		if (send(serv_fd, msg, T4_MSG_SIZE, 0) == -1)
		{
			perror("Client send");
			exit(EXIT_FAILURE);
		}

		/* Wait for message from server */
		if (recv(serv_fd, msg, T4_MSG_SIZE, 0) == -1)
		{
			perror("Client recv");
			exit(EXIT_FAILURE);
		}
		else
		{
			/* Get server adress */
			inet_ntop(T4_PROTO_FAMILY, &server.sin_addr, endp_addr,
						INET6_ADDRSTRLEN);
			printf("Client received: %s from: %s at port %d\n", msg, endp_addr,
					ntohs(server.sin_port));
		}

		exit(EXIT_SUCCESS);
	}
	else
	{
		/*
		* Declare:
		* - server & client - server's and client's endpoints;
		* - server_fd - socket fd;
		* - client_size - size of client's endpoint;
		* - msg - message buffer;
		* - endp_addr - char buffer for client adress.
		*/
		struct sockaddr_in server, client;
		int server_fd;
		int client_size;
		char msg[T4_MSG_SIZE], endp_addr[INET6_ADDRSTRLEN];

		/* Fill 'server' & 'client' with 0's */
		memset(&server, 0, sizeof(server));
		memset(&client, 0, sizeof(client));

		/* Set server's endpoint */
		server.sin_family = T4_PROTO_FAMILY;
		if (inet_pton(T4_PROTO_FAMILY, T4_SERV_ADDR, &server.sin_addr) == -1)
		{
			perror("inet_pton");
			exit(EXIT_FAILURE);
		}
		server.sin_port = htons(T4_SERV_PORT);

		/* Create socket */
		server_fd = socket(T4_PROTO_FAMILY, SOCK_DGRAM, 0);

		/* Try to bind server's endpoint to socket */
		while (bind(server_fd, (struct sockaddr *)&server, sizeof(server))
				== -1)
		{
			perror("bind");
			exit(EXIT_FAILURE);
		}

		struct pollfd *pfds = malloc(2*sizeof(struct pollfd));
		pfds[0].fd = STDIN_FILENO;
		pfds[0].events = POLLIN;
		pfds[1].fd = server_fd;
		pfds[1].events = POLLIN;
		int ret = poll(pfds, 2, 10000);
		printf("poll ret = %d, revents = %d%d\n", ret, pfds[0].revents, pfds[1].revents);
		free(pfds);

		/* Wait for message from client, get client's endpoint */
		client_size = sizeof(client);
		while (recvfrom(server_fd, msg, T4_MSG_SIZE, 0,
						(struct sockaddr *)&client, &client_size) == -1)
		{
			if (errno != EWOULDBLOCK)
			{
				perror("server recvfrom");
				printf("Errno = %d\n", errno);
				exit(EXIT_FAILURE);
			}
		}
		inet_ntop(T4_PROTO_FAMILY, &client.sin_addr, endp_addr,
					INET6_ADDRSTRLEN);
		printf("Server received: %s from: %s at port: %d\n", msg, endp_addr,
				ntohs(client.sin_port));

		/* Send message to client */
		strncpy(msg, T4_SERVER_MSG, T4_MSG_SIZE);
		if (sendto(server_fd, msg, T4_MSG_SIZE, MSG_DONTWAIT,
					(struct sockaddr *)&client, client_size) == -1)
		{
			perror("server sendto");
			exit(EXIT_FAILURE);
		}

		/* Wait for child process to finish */
		wait(NULL);

		/* Close socket fd */
		if (close(server_fd) == -1)
		{
			perror("close(server_fd)");
		}
	}

	return EXIT_SUCCESS;
}
