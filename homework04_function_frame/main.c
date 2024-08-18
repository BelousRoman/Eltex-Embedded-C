#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>

int main(int argc, char *argv[])
{
    /* Declare variables:
    * - opt - option char, received from getopt;
    * - char_len - length of password field, written to resulting string;
    * - hex_len - number of hex bytes, written to resulting string;
    * - str_len - length of resulting string;
    * - seg_len - length of current hex string segment;
    * - seg_str - hex string segment, for parsing 4 bytes at once;
    * - seg_val - union, containing decimal value and char bytes, used in
    *   parsing hex string;
    * - file_name - name of file;
    * - char_text - raw text, containing unparsed password string;
    * - hex_text - raw text, containing unparsed hex values;
    * - string - text, written to a file;
    * - index & sec_index & thrd_index - index variables, used in 'for' loops.
    */
    int opt;
    int char_len = -1;
    int hex_len = -1;
    int str_len = 0;
    int seg_len;
    char seg_str[(sizeof(int)*2)];
    union {
        char bytes[sizeof(int)];
        long number;
    } seg_val;
    char *file_name = NULL;
    char *char_text = NULL;
    char *hex_text = NULL;
    char *string = NULL;
    int index;
    int sec_index;
    int thrd_index;

    /* Parse options */
    while ((opt = getopt(argc, argv, "f:p:l:h:b:")) != -1)
    {
        switch (opt)
        {
        case 'f':
        {
            if (file_name != NULL)
                free(char_text);
            
            file_name = malloc(strlen(optarg)*sizeof(char));
            if (file_name == NULL)
            {
                perror("malloc");
                exit(EXIT_FAILURE);
            }
            strncpy(file_name, optarg, strlen(optarg));
        }
            break;
        case 'p':
        {
            if (char_text != NULL)
                free(char_text);
            
            char_text = malloc(strlen(optarg)*sizeof(char));
            if (char_text == NULL)
            {
                perror("malloc");
                exit(EXIT_FAILURE);
            }
            strncpy(char_text, optarg, strlen(optarg));
        }
            break;
        case 'l':
            char_len = atoi(optarg);
            break;
        case 'h':
        {
            if (hex_text != NULL)
                free(hex_text);
            
            hex_text = malloc(strlen(optarg)*sizeof(char));
            if (hex_text == NULL)
            {
                perror("malloc");
                exit(EXIT_FAILURE);
            }
            strncpy(hex_text, optarg, strlen(optarg));
        }
            break;
        case 'b':
            hex_len = atoi(optarg);
            break;
        default:
            fprintf(stderr, "Usage: %s [-f filename] [-p password string] " \
                    "[-l password length] [-h hexadecimal string] " \
                    "[-b hex bytes]\n", argv[0]);
            return EXIT_FAILURE;
        }
    }

    /* Check that all the parameter has been filled */
    if (char_len == -1 || hex_len == -1 || file_name == NULL ||
        char_text == NULL || hex_text == NULL)
    {
        printf("One or more parameters missing\n" \
                "Usage: %s [-f filename] [-p password string] " \
                "[-l password length] [-h hexadecimal string] " \
                "[-b hex bytes]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Allocate memory to resulting string */
    str_len = char_len+hex_len;
    string = malloc((str_len)*sizeof(char));
    if (string == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    memset(string, 0, str_len);

    /* Concatenate empty resulting string with raw password string */
    strncat(string, char_text, char_len);

    /* Parse raw hex string to bytes and concatenate with resulting string */
    for (index = 0; index < (hex_len*2);index+=(sizeof(int)*2))
    {
        /*
        * FIll 'seg_str' with 0's, then overwrite them with segment of raw hex
        * string.
        */
        memset(seg_str, 0, sizeof(seg_str));
        memcpy(seg_str, (hex_text+index), sizeof(seg_str));

        /*
        * Count total number of bytes to be parsed, round the division result
        * up.
        */
        seg_len = strlen(seg_str);
        seg_len = (seg_len%2) == 0 ? seg_len / 2 : (seg_len+1) / 2;

        /*
        * Parse hex string segment to decimal value, then concatenate with
        * resulting string byte-by-byte.
        */
        seg_val.number = strtol(seg_str, NULL, 16);
        for (sec_index = seg_len-1, thrd_index = 0; sec_index >= 0; sec_index--, thrd_index++)
        {
            strncat(string+char_len+(index/2)+thrd_index, &seg_val.bytes[sec_index], 1);
        }
    }

    /*
    * Open a file with O_WRONLY | O_CREAT | O_TRUNC flags, write resulting
    * string to it.
    */
    FILE* file = fopen(file_name, "w");
    fwrite(string, sizeof(char), str_len, file);

    printf("Successfully wrote %d bytes to a file \"%s\"!\n", str_len, file_name);

    /* Free allocated memory, close file */
    free(file_name);
    free(char_text);
    free(hex_text);
    free(string);
    fclose(file);

    return 0;
}
