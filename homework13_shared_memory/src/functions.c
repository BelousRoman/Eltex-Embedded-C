#include "../hdr/functions.h"

int first_task()
{
	puts("First task");

	/*
	* Declare:
	* - shm_key & sem_key - unique keys, generated through ftok();
	* - shm_id & sem_id - ids for shared memory segment and semaphore;
	* - pid - process id of child process;
	* - msg - char pointer on shared memory, mapped for two processes.
	* All above variables are later copied to child process using COW.
	*/
	key_t shm_key;
	key_t sem_key;
	int shm_id;
	int sem_id;
	pid_t pid;
	char *msg;

	/* Create child process */
	pid = fork();
	if (pid == 0)
	{
		/*
		* Define sembufs for locking&unlocking semaphore here to not allow
		* parent process to lock semaphore before child does it.
		*/
		struct sembuf lock[1] = {
			{0, -1, 0}
		};
		struct sembuf unlock[2] = {
			{0, 0, 0},
			{0, 2, 0}
		};

		/*
		* Get shm_key, wait for parent process to create shm segment, only then
		* get its id and map into child process.
		*/
		shm_key = ftok("./hw13_shm", T1_SHM_KEY);
		while((shm_id = shmget(shm_key, T1_MSG_SIZE, IPC_PRIVATE)) == -1)
		{
			if (errno != ENOENT)
			{
				perror("child shmget");
				exit(EXIT_FAILURE);
			}
		}
		if ((msg = shmat(shm_id, NULL, 0)) == -1)
		{
			perror("child shmat");
			exit(EXIT_FAILURE);
		}

		/*
		* Get sem_key, wait for parent process to create sem segment, only then
		* get its id.
		*/
		sem_key = ftok("./hw13_shm", T1_SEM_KEY);
		while((sem_id = semget(sem_key, 1, IPC_PRIVATE)) == -1)
		{
			if (errno != ENOENT)
			{
				perror("child semget");
				exit(EXIT_FAILURE);
			}
		}

		/* Wait for parent to unlock semaphore */
		semop(sem_id, &lock, 1);
		printf("Child read <%s>\n", msg);

		/* Rewrite buffer and unlock semaphore */
		snprintf(msg, T1_MSG_SIZE, "Hi");
		semop(sem_id, &unlock, 2);

		/* Detach shm segment from process adress space */
		if (shmdt(msg) == -1)
		{
			perror("child shmdt");
			exit(EXIT_FAILURE);
		}

		exit(EXIT_SUCCESS);
	}
	else
	{
		/*
		* Define sembufs for locking&unlocking semaphore here to not allow
		* parent process to lock semaphore before child does it.
		*/
		struct sembuf lock[1] = {
			{0, -2, 0},
		};
		struct sembuf unlock[2] = {
			{0, 0, 0},
			{0, 1, 0},
		};

		/*
		* Get shm_key, then create shm segment and map it into process adress 
		* space.
		*/
		shm_key = ftok("./hw13_shm", T1_SHM_KEY);
		shm_id = shmget(shm_key, T1_MSG_SIZE, IPC_CREAT | T1_PERMS);
		if ((msg = shmat(shm_id, NULL, 0)) == -1)
		{
			perror("parent shmat");
			exit(EXIT_FAILURE);
		}

		/* Get sem_key, then create sem segment */
		sem_key = ftok("./hw13_shm", T1_SEM_KEY);
		sem_id = semget(sem_key, 1, IPC_CREAT | T1_PERMS | IPC_PRIVATE);
		if (sem_id == -1)
		{
			perror("parent semget");
			exit(EXIT_FAILURE);
		}

		/* Rewrite buffer and unlock semaphore */
		snprintf(msg, T1_MSG_SIZE, "Hello");
		semop(sem_id, &unlock, 2);

		/* Wait for child to unlock semaphore */
		semop(sem_id, &lock, 1);
		printf("Parent read <%s>\n", msg);

		/* Detach shm segment from process adress space */
		if (shmdt(msg) == -1)
		{
			perror("parent shmdt");
			exit(EXIT_FAILURE);
		}

		wait(NULL);
	}

	/* Remove shared memory segment */
	if (shmctl(shm_id, IPC_RMID, NULL) == -1)
	{
		perror("shmctl");
		exit(EXIT_FAILURE);
	}

	/* Remove semaphore segment */
	if(semctl(sem_id, 1, IPC_RMID) == -1)
	{
		perror("semctl");
		exit(EXIT_FAILURE);
	}
	return EXIT_SUCCESS;
}

int second_task()
{
	puts("Second task");
	/*
	* Declare:
	* - shm_fd - shared memory segment fd;
	* - ptoc_sem & ctop_sem - pointers to sem_t, both contains a semaphore for
	*   locking parent and child processes;
	* - pid - process id of child process;
	* - msg - char pointer on shared memory, mapped for two processes.
	* All above variables are later copied to child process using COW.
	*/
	int shm_fd;
	sem_t *ptoc_sem;
	sem_t *ctop_sem;
	char *msg;
	pid_t pid;

	/* Create child process */
	pid = fork();
	if (pid == 0)
	{
		/*
		* Wait for parent to create shm segment, then get it's fd and map into
		* process adress space.
		*/
		while ((shm_fd = shm_open(T2_SHM_NAME, O_RDWR, NULL)) == -1)
		{
			if (errno != ENOENT)
			{
				perror("child shm_open");
				exit(EXIT_FAILURE);
			}
		}
		if ((msg = mmap(NULL, T2_MSG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
						shm_fd, 0)) == -1)
		{
			perror("child mmap");
			exit(EXIT_FAILURE);
		}

		/* Wait for parent to create sem segments, then get it's fds */
		while ((ptoc_sem = sem_open(T2_PTOC_SEM_NAME, O_RDWR)) == SEM_FAILED)
		{
			if (errno != ENOENT)
			{
				perror("child ptoc_sem sem_open");
				exit(EXIT_FAILURE);
			}
		}
		while ((ctop_sem = sem_open(T2_CTOP_SEM_NAME, O_RDWR)) == SEM_FAILED)
		{
			if (errno != ENOENT)
			{
				perror("child ctop_sem sem_open");
				exit(EXIT_FAILURE);
			}
		}
		
		/* Wait for parent to unlock semaphore */
		if (sem_wait(ptoc_sem) == -1)
		{
			perror("child sem_wait");
			exit(EXIT_FAILURE);
		}
		printf("Child read <%s>\n", msg);

		/* Rewrite buffer and unlock semaphore */
		snprintf(msg, T2_MSG_SIZE, "Hi");
		if (sem_post(ctop_sem) == -1)
		{
			perror("child sem_post");
			exit(EXIT_FAILURE);
		}

		/* Unmap shm segment from process adress space */
		if (munmap(msg, T2_MSG_SIZE) == -1)
		{
			perror("child munmap");
			exit(EXIT_FAILURE);
		}

		/* Close sem segments fds */
		if (sem_close(ptoc_sem) == -1)
		{
			perror("child ptoc_sem sem_close");
			exit(EXIT_FAILURE);
		}
		if (sem_close(ctop_sem) == -1)
		{
			perror("child ctop_sem sem_close");
			exit(EXIT_FAILURE);
		}

		exit(EXIT_SUCCESS);
	}
	else
	{
		/*
		* Create shared memory segment, change it's size and map into process
		* adress space.
		*/
		if ((shm_fd = shm_open(T2_SHM_NAME, O_CREAT | O_RDWR, T2_PERMS)) == -1)
		{
			perror("parent shm_open");
			exit(EXIT_FAILURE);
		}
		if (ftruncate(shm_fd, T2_MSG_SIZE) == -1)
		{
			perror("parent ftruncate");
			exit(EXIT_FAILURE);
		}
		if ((msg = mmap(NULL, T2_MSG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
						shm_fd, 0)) == -1)
		{
			perror("parent mmap");
			exit(EXIT_FAILURE);
		}

		/* Create sem segments */
		ptoc_sem = sem_open(T2_PTOC_SEM_NAME, O_CREAT | O_RDWR, T2_PERMS, 0);
		if (ptoc_sem == SEM_FAILED)
		{
			perror("parent ptoc_sem sem_open");
			exit(EXIT_FAILURE);
		}
		ctop_sem = sem_open(T2_CTOP_SEM_NAME, O_CREAT | O_RDWR, T2_PERMS, 0);
		if (ctop_sem == SEM_FAILED)
		{
			perror("parent ctop_sem sem_open");
			exit(EXIT_FAILURE);
		}

		/* Rewrite buffer and unlock semaphore */
		snprintf(msg, T2_MSG_SIZE, "Hello");
		if (sem_post(ptoc_sem) == -1)
		{
			perror("parent sem_post");
			exit(EXIT_FAILURE);
		}
		
		/* Wait for child to unlock semaphore */
		if (sem_wait(ctop_sem) == -1)
		{
			perror("parent sem_wait");
			exit(EXIT_FAILURE);
		}
		printf("Parent read <%s>\n", msg);

		/* Unmap shm segment from process adress space */
		if (munmap(msg, T2_MSG_SIZE) == -1)
		{
			perror("parent munmap");
			exit(EXIT_FAILURE);
		}

		/* Close sem segments fds */
		if (sem_close(ptoc_sem) == -1)
		{
			perror("parent ptoc_sem sem_close");
			exit(EXIT_FAILURE);
		}
		if (sem_close(ctop_sem) == -1)
		{
			perror("parent ctop_sem sem_close");
			exit(EXIT_FAILURE);
		}
		wait(NULL);
	}

	/* Remove shm segment */
	if (shm_unlink(T2_SHM_NAME) == -1)
	{
		perror("shm_unlink");
		exit(EXIT_FAILURE);
	}

	/* Remove sem segments */
	if (sem_unlink(T2_PTOC_SEM_NAME) == -1)
	{
		perror("ptoc_sem sem_unlink");
		exit(EXIT_FAILURE);
	}
	if (sem_unlink(T2_CTOP_SEM_NAME) == -1)
	{
		perror("ctop_sem sem_unlink");
		exit(EXIT_FAILURE);
	}
	
	return EXIT_SUCCESS;
}
