#include "../hdr/functions.h"

int server(void)
{
	puts("Server");

	/*
    * Declare:
    * - server_fd - fd of server socket;
    * - server & client - server's and client's endpoints;
    * - client_size - 'client' length, passed to recvfrom as an argument;
    * - msg - message buffer.
    */
	int server_fd = 0;
	struct sockaddr_in server;
	struct sockaddr_in client;
	int client_size;
	char msg[MSG_SIZE];

	/* Fill 'server' and 'client' with nulls */
	memset(&server, NULL, sizeof(server));
	memset(&client, NULL, sizeof(client));

	/* Set server's endpoint */
	server.sin_family = AF_INET;
	if (inet_pton(AF_INET, SERVER_ADDR, &server.sin_addr) == -1)
	{
        printf("inet_pton: %s(%d)\n", strerror(errno), errno);
        return EXIT_FAILURE;
	}
	server.sin_port = htons(SERVER_PORT);

	/* Create socket */
	server_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (server_fd <= 0)
	{
		printf("socket: %s(%d)\n", strerror(errno), errno);
        return EXIT_FAILURE;
	}

	/* Allow reuse of local address */
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

	/* Bind endpoint to socket */
	if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
        printf("bind: %s(%d)\n", strerror(errno), errno);
		return EXIT_FAILURE;
	}

	/* Receive a message from client*/
    client_size = sizeof(client);
    if (recvfrom(server_fd, msg, MSG_SIZE, NULL, &client, &client_size) == -1)
    {
        printf("recvfrom: %s (%d)\n", strerror(errno), errno);
        return EXIT_FAILURE;
    }

	char addr[INET6_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &client.sin_addr, addr, INET6_ADDRSTRLEN) == NULL)
    {
        perror("inet_ntop");
        exit(EXIT_FAILURE);
    }
	printf("Received message <%s> from client %s:%d\n", msg, addr, ntohs(client.sin_port));

	/* Send a message to a client */
    strncpy(msg, SERVER_MSG, MSG_SIZE);
    if (sendto(server_fd, msg, MSG_SIZE, NULL, &client, sizeof(client)) == -1)
    {
        printf("sendto: %s (%d)\n", strerror(errno), errno);
        return EXIT_FAILURE;
    }

	if (inet_ntop(AF_INET, &client.sin_addr, addr, INET6_ADDRSTRLEN) == NULL)
    {
        perror("inet_ntop");
        exit(EXIT_FAILURE);
    }
	printf("Sent message <%s> to client %s:%d\n", msg, addr, ntohs(client.sin_port));

	return EXIT_SUCCESS;
}
