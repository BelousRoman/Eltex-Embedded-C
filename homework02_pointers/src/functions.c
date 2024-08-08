#include "../hdr/functions.h"

int first_task(int argc, char *argv[])
{
    puts("First task");

    int opt;
    int value = VALUE;
    char byte_num = BYTE_TO_CHANGE;
    char new_val = NEW_VALUE;
    char *val_ptr = (char *)&value;

    while ((opt = getopt(argc, argv, "v:b:n:")) != -1)
    {
        switch (opt)
        {
        case 'v':
            value = atoi(optarg);
            break;
        case 'b':
            byte_num = (char)atoi(optarg);
            break;
        case 'n':
            new_val = (char)atoi(optarg);
            break;
        default:
            fprintf(stderr, "Usage: %s [-v value] [-b byte to change] [-n new byte value]\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (byte_num < 1)
        byte_num = 1;
    else if (byte_num > 4)
        byte_num = 4;

    val_ptr = val_ptr + (byte_num-1);

    printf("Inital value: %d (0x%x)\n", value, value);

    printf("Change byte #%d from 0x%x to 0x%x\n", byte_num, (*val_ptr & 0xFF), (new_val & 0xFF));

    *val_ptr = new_val;

    printf("Changed value: %d (0x%x)\n", value, value);

	return EXIT_SUCCESS;
}
