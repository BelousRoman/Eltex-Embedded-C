#include "../hdr/functions.h"

/*
* Declare global variables:
* - dict - pointer to dynamic array;
* - clients_num - size of dynamic array.
*/
struct dictionary *dict = NULL;
int clients_num = 0;

/* Signal handler for SIGINT */
static void sigint_handler(int sig, siginfo_t *si, void *unused)
{
    exit(EXIT_SUCCESS);
}

/* Function provided to atexit() */
void _cleanup(void)
{
    int index;

    if (dict != NULL)
    {
        printf("\nDictionary contains following entries (%d in total):\n", clients_num);

        for (index = 0; index < clients_num; ++index)
        {
            printf("#%d - %s %s (%s)\n", (index+1), dict[index].name, dict[index].surname, dict[index].phone);
        }
        free(dict);
    }
    
    puts("Exit program");
}

int first_task()
{
    puts("First task");

    /*
    * Declare:
    * - sa - sigaction, used to redefine signal handler for SIGINT;
    * - tmp_dict - dictionary entry, temporary storing new person's info;
    * - tmp_ptr - temporary pointer to reallocated dictionary;
    * - str - string, passed to fgets function;
    * - ch_ptr - address of \n in 'str' string;
    * - tv - timeval variable to contain timeout value for ;
    * - str_len - length of current field.
    */
    struct sigaction sa;
    struct dictionary tmp_dict;
    struct dictionary *tmp_ptr;
    char str[STDIN_STR_LEN];
    char *ch_ptr = NULL;
    // struct timeval tv;
    int str_len;

    // tv.tv_sec = 0;
    // tv.tv_usec = 0;

    /* Fill sa_mask, set signal handler, redefine SIGINT with sa */
    sigfillset(&sa.sa_mask);
    sa.sa_sigaction = sigint_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    /* Set atexit function */
    if (atexit(_cleanup) != 0)
    {
        perror("atexit");
        exit(EXIT_FAILURE);
    }

    /* Allocate memory to dictionary array */
    dict = malloc(sizeof(struct dictionary) * clients_num);

    if (dict == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    /* Endless loop, filling dictionary, */
    while(1)
    {
        /* Fill 'tmp_dict' with 0's */
        memset(&tmp_dict, 0, sizeof(struct dictionary));

        /* Read user input for name field */
        str_len = NAME_LEN;
        printf("Enter a name: ");
        fgets(str, STDIN_STR_LEN, stdin);
        ch_ptr = strchr(str, '\n');
        str_len = (ch_ptr - str) > str_len ? str_len : (ch_ptr - str);
        *ch_ptr = '\0';
        memcpy(&tmp_dict.name, str, str_len);

        /* Read user input for surname field */
        str_len = SURNAME_LEN;
        printf("Enter a surname: ");
        fgets(str, STDIN_STR_LEN, stdin);
        ch_ptr = strchr(str, '\n');
        str_len = (ch_ptr - str) > str_len ? str_len : (ch_ptr - str);
        *ch_ptr = '\0';
        memcpy(&tmp_dict.surname, str, str_len);

        /* Read user input for phone number field */
        str_len = PHONE_LEN;
        printf("Enter a phone number: ");
        fgets(str, STDIN_STR_LEN, stdin);
        ch_ptr = strchr(str, '\n');
        str_len = (ch_ptr - str) > str_len ? str_len : (ch_ptr - str);
        *ch_ptr = '\0';
        memcpy(&tmp_dict.phone, str, str_len);

        /* Allocate more memory to dictionary*/
        clients_num++;
        tmp_ptr = realloc(dict, sizeof(struct dictionary)*clients_num);
        if (tmp_ptr == NULL)
        {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        dict = tmp_ptr;
        memset(&dict[clients_num-1], 0, sizeof(struct dictionary));

        /* Copy 'tmp_dict' to newly allocated entry in dictionary */
        memcpy(&dict[clients_num-1], &tmp_dict, sizeof(struct dictionary));

        printf("Entry %s %s (%s) has been added!\n\r", dict[clients_num-1].name, dict[clients_num-1].surname, dict[clients_num-1].phone);
    }

	return EXIT_SUCCESS;
}
