#include "../hdr/functions.h"

int first_task(int argc, char *argv[])
{
    puts("First task");

    /*
    * Declare:
    * - opt - return value of getopt, passed as character;
    * - value - value to be changed, can be set by providing an option -v with integer value;
    * - byte_num - value to be changed, can be set by providing an option -b with integer value;
    * - new_val - value to be changed, can be set by providing an option -n with integer value;
    * - val_ptr - 1 byte pointer to value, used to change specific byte in it.
    */
    int opt;
    int value;
    char byte_num;
    char new_val;
    char *val_ptr;

    /* Set initial values */
    value = VALUE;
    byte_num = BYTE_TO_CHANGE;
    new_val = NEW_VALUE;
    val_ptr = (char *)&value;

    /* Get options, provided to program */
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
            return EXIT_FAILURE;
        }
    }

    /* Verify that byte number is within the limits */
    if (byte_num < 1)
        byte_num = 1;
    else if (byte_num > 4)
        byte_num = 4;

    /* Align pointer to the chosen byte */
    val_ptr = val_ptr + (byte_num-1);

    printf("Inital value: %d (0x%x)\n", value, value);

    printf("Change byte #%d from 0x%x to 0x%x\n", byte_num, (*val_ptr & 0xFF), (new_val & 0xFF));

    /* Assign this byte a new value */
    *val_ptr = new_val;

    printf("Changed value: %d (0x%x)\n", value, value);

	return EXIT_SUCCESS;
}
