#include "../hdr/functions.h"

int first_task(int argc, char *argv[])
{
    puts("First task");

    int opt;
    struct mask number;
    struct mask bit_mask;

    number.value = VALUE;
    bit_mask.value = BIT_MASK;

    while ((opt = getopt(argc, argv, "v:d:b:h:")) != -1)
    {
        switch (opt)
        {
        case 'v':
            number.value = (unsigned char)atoi(optarg);
            break;
        case 'd':
            bit_mask.value = (unsigned char)atoi(optarg);
            break;
        case 'b':
            bit_mask.value = (unsigned char)strtol(optarg, NULL, 2);
            break;
        case 'h':
            bit_mask.value = (unsigned char)strtol(optarg, NULL, 16);
            break;
        default:
            fprintf(stderr, "Usage: %s [-v value] [-d decimal mask] " \
                    "[-b binary mask] [-h hexadecimal mask]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    printf("Inital value: %d (0b%d%d%d%d%d%d%d%d)\n", number.value, 
            number.byte.b8, number.byte.b7, number.byte.b6,
            number.byte.b5, number.byte.b4, number.byte.b3,
            number.byte.b2, number.byte.b1);

    printf("Aplly bit mask: %d (0b%d%d%d%d%d%d%d%d)\n", bit_mask.value,
            bit_mask.byte.b8, bit_mask.byte.b7, bit_mask.byte.b6,
            bit_mask.byte.b5, bit_mask.byte.b4, bit_mask.byte.b3,
            bit_mask.byte.b2, bit_mask.byte.b1);

    number.value = number.value & bit_mask.value;

    printf("Changed value: %d (0b%d%d%d%d%d%d%d%d)\n", number.value,
            number.byte.b8, number.byte.b7, number.byte.b6,
            number.byte.b5, number.byte.b4, number.byte.b3,
            number.byte.b2, number.byte.b1);

	return EXIT_SUCCESS;
}
