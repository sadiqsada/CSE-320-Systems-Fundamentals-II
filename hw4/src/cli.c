/*
 * Imprimer: Command-line interface
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "imprimer.h"
#include "conversions.h"
#include "sf_readline.h"

// counts the number of arguments in given input
int count_args(char *input, char *delim)
{
    int count = 0;
    char *ptr = input;
    char *prevPtr;
    while ((ptr = strchr(ptr, ' ')) != NULL)
    {
        count++;
        prevPtr = ptr;
        while (*ptr == ' ')
        {
            ptr++;
        }
    }

    int increment = 0;

    if (count == 0)
        return count + 1;

    while (*prevPtr != '\0')
    {
        if (*prevPtr != ' ')
        {
            increment = 1;
            break;
        }
        prevPtr++;
    }

    return count + increment;
}

void print_arg_error(int argCount)
{
    printf("Wrong number of args for command: help; Expected Args: 0, Provided Args: %d\n", argCount);
    sf_cmd_error("arg count");
}

int run_cli(FILE *in, FILE *out)
{
    // TO BE IMPLEMENTED
    while (1)
    {
        char *input = sf_readline("imp>");
        char *delim = " ";
        char *helpMessage = "Commands are: help quit type printer conversion printers jobs print cancel disable enable pause resume";

        // non-empty input
        if (*input != '\0')
        {
            int argCount = count_args(input, delim);
            char *token = strtok(input, delim);
            int quit = 0;
            while (token != NULL)
            {
                // help command
                if (strcmp(token, "help") == 0)
                {
                    if (argCount > 1)
                    {
                        print_arg_error(argCount);
                        break;
                    }
                    printf("%s\n", helpMessage);
                    sf_cmd_ok();
                }
                // quit command
                if (strcmp(token, "quit") == 0)
                {
                    if (argCount > 1)
                    {
                        print_arg_error(argCount);
                        break;
                    }
                    quit = 1;
                    sf_cmd_ok();
                    break;
                }
                token = strtok(NULL, delim);
            }

            if (quit)
                break;
        }
    }

    // fprintf(stderr, "You have to implement run_cli() before the application will function.\n");
    // abort();
    return 0;
}
