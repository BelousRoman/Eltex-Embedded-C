#include "../hdr/functions.h"

struct dictionary *dict = NULL;
int clients_num = 0;

static void sigint_handler(int sig, siginfo_t *si, void *unused)
{
    exit(EXIT_SUCCESS);
}

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
    // sa - sigaction, used to redefine signal handler for SIGINT;
    struct sigaction sa;
    fd_set rfds;
    struct dictionary tmp_dict;
    struct dictionary *tmp_ptr;
    char str[STDIN_STR_LEN];
    char *ch_ptr = NULL;
    struct timeval tv;
    int retval;
    int str_len;

    tv.tv_sec = 0;
    tv.tv_usec = 100;

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);

    sigfillset(&sa.sa_mask);
    sa.sa_sigaction = sigint_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    if (atexit(_cleanup) != 0)
    {
        perror("atexit");
        exit(EXIT_FAILURE);
    }

    dict = malloc(sizeof(struct dictionary) * clients_num);

    if (dict == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        memset(&tmp_dict, 0, sizeof(struct dictionary));

        str_len = NAME_LEN;
        printf("Enter a name: ");
        fgets(str, STDIN_STR_LEN, stdin);
        ch_ptr = strchr(str, '\n');
        str_len = (ch_ptr - str) > str_len ? str_len : (ch_ptr - str);
        *ch_ptr = '\0';
        memcpy(&tmp_dict.name, str, str_len);

        str_len = SURNAME_LEN;
        printf("Enter a surname: ");
        fgets(str, STDIN_STR_LEN, stdin);
        ch_ptr = strchr(str, '\n');
        str_len = (ch_ptr - str) > str_len ? str_len : (ch_ptr - str);
        *ch_ptr = '\0';
        memcpy(&tmp_dict.surname, str, str_len);

        str_len = PHONE_LEN;
        printf("Enter a phone number: ");
        fgets(str, STDIN_STR_LEN, stdin);
        ch_ptr = strchr(str, '\n');
        str_len = (ch_ptr - str) > str_len ? str_len : (ch_ptr - str);
        *ch_ptr = '\0';
        memcpy(&tmp_dict.phone, str, str_len);
        
        clients_num++;
        tmp_ptr = realloc(dict, sizeof(struct dictionary)*clients_num);
        if (tmp_ptr == NULL)
        {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        dict = tmp_ptr;
        memset(&dict[clients_num-1], 0, sizeof(struct dictionary));

        memcpy(&dict[clients_num-1], &tmp_dict, sizeof(struct dictionary));

        printf("Entry %s %s (%s) has been added!\n\r", dict[clients_num-1].name, dict[clients_num-1].surname, dict[clients_num-1].phone);

        if (fflush(stdin) != 0)
        {
            perror("fflush");
            exit(EXIT_FAILURE);
        }
    }

	return EXIT_SUCCESS;
}
