#include "../hdr/functions.h"

int first_task()
{
	puts("First task");

	/*
	* Declare:
	* - mq_id - message queue id;
	* - key - unique queue key, generated with ftok;
	* - msg - message, passed through queue, contains long and char array;
	* - pid - variable, storing child pid.
	* All above variables are later copied to child process using COW.
	*/
	int mq_id;
	key_t key;
	struct msgbuf msg;
	pid_t pid;

	/*
	* Create child process, to make sure that communication is provided without
	* dependecy on shared resources.
	*/
	pid = fork();
	if (pid == 0)
	{
		/* Generate unique key */
		key = ftok("./hw12_mq", 1);
		/* Get queue id */
		mq_id = msgget(key, IPC_CREAT | T1_PERMS);

		/* Receive message from parent process */
		if (msgrcv(mq_id, &msg, T1_MSG_SIZE, 1, MSG_NOERROR) == -1)
			perror("msgrcv");

		printf("Child has received message: <%s>\n", msg.mtext);

		/* Form a message with type 2 and then send it to parent process */
		msg.mtype = 2;
		snprintf(msg.mtext, T1_MSG_SIZE, "Hi");
		if (msgsnd(mq_id, &msg, T1_MSG_SIZE, 0) == -1)
			perror("msgsnd");

		exit(EXIT_SUCCESS);
	}
	else
	{
		/* Generate unique key */
		key = ftok("./hw12_mq", 1);
		/* Get queue id */
		mq_id = msgget(key, IPC_CREAT | T1_PERMS);

		/* Form a message with type 1to send to child process*/
		msg.mtype = 1;
		snprintf(msg.mtext, T1_MSG_SIZE, "Hello");
		if (msgsnd(mq_id, &msg, T1_MSG_SIZE, 0) == -1)
			perror("msgsnd");

		/* Receive answner from child process */
		if (msgrcv(mq_id, &msg, T1_MSG_SIZE, 2, MSG_NOERROR) == -1)
			perror("msgrcv");

		printf("Parent has received message: <%s>\n", msg.mtext);

		/* Wait for child process to finish */
		wait(NULL);

		/* Delete queue */
		if (msgctl(mq_id, IPC_RMID, NULL) == -1)
			perror("msgctl");
	}
	
	return EXIT_SUCCESS;
}

int second_task()
{
	puts("Second task");
	
	/*
	* Declare:
	* - parent - queue to transfer messages from parent to child process;
	* - child - queue to transfer messages from child to process process;
	* - attr - queue attributes;
	* - msg_send - char array for sending messages;
	* - msg_rcv - char array for receiving messages;
	* - prio - priority of received message.
	* All above variables are later copied to child process using COW.
	*/
	mqd_t parent;
	mqd_t child;
	struct mq_attr attr;
	char msg_send[T2_MSG_SIZE];
	char msg_rcv[T2_MSG_SIZE];
	unsigned int prio;

	/* Set queue attributes */
	attr.mq_maxmsg = T2_MAX_MSGS;
	attr.mq_msgsize = T2_MSG_SIZE;

	/*
	* Create child process, to make sure that communication is provided without
	* dependecy on shared resources.
	*/
	pid_t pid = fork();
	if (pid == 0)
	{
		/* Get queue fds for child and parent queues */
		child = mq_open("/child", O_CREAT | O_WRONLY, T2_PERMS, &attr);
		if (child == -1)
			perror("in child process: child mq_open");
		parent = mq_open("/parent", O_CREAT | O_RDONLY, T2_PERMS, &attr);
		if (parent == -1)
			perror("in child process: parent mq_open");

		/* Receive a message from parent queue */
		mq_receive(parent, msg_rcv, T2_MSG_SIZE, &prio);
		printf("Child has received message: <%s>(%d)\n", msg_rcv, prio);

		/* Form a message with priority 2 and send it to child queue */
		snprintf(msg_send, T2_MSG_SIZE, "Greetings");
		mq_send(child, msg_send, T2_MSG_SIZE, 2);

		/* Close queue fds */
		mq_close(child);
		mq_close(parent);

		exit(EXIT_SUCCESS);
	}
	else
	{
		/* Get queue fds for parent and child queues */
		parent = mq_open("/parent", O_CREAT | O_WRONLY, T2_PERMS, &attr);
		if (parent == -1)
			perror("in parent process: parent mq_open");
		child = mq_open("/child", O_CREAT | O_RDONLY, T2_PERMS, &attr);
		if (child == -1)
			perror("in parent process: child mq_open");
		
		/* Form a message with priority 1 and send it to parent queue */
		snprintf(msg_send, T2_MSG_SIZE, "Good morning");
		mq_send(parent, msg_send, T2_MSG_SIZE, 1);
		
		/* Receive a message from child queue */
		mq_receive(child, msg_rcv, T2_MSG_SIZE, &prio);

		printf("Parent has received message: <%s>(%d)\n", msg_rcv, prio);

		/* Wait for child process to finish */
		wait(NULL);
	}

	/* Close queue fds, delete files */
	mq_close(parent);
	mq_close(child);

	mq_unlink("/parent");
	mq_unlink("/child");
	
	return EXIT_SUCCESS;
}
