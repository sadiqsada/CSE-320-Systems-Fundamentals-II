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
#include "header.h"

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

    if (count == 0)
        return count + 1;

    int increment = 0;

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

void print_arg_error(int expectedArgs, int argCount)
{
    printf("Wrong number of args for command: help; Expected Args: %d, Provided Args: %d\n", expectedArgs, argCount - 1);
    sf_cmd_error("arg count");
}

// FILE_TYPE *define_type(char *name)
// {
// }

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
                // Miscellaneous commands

                // help command
                if (strcmp(token, "help") == 0)
                {
                    if (argCount != 1)
                    {
                        print_arg_error(0, argCount);
                        break;
                    }
                    printf("%s\n", helpMessage);
                    sf_cmd_ok();
                }
                // quit command
                if (strcmp(token, "quit") == 0)
                {
                    if (argCount != 1)
                    {
                        print_arg_error(0, argCount);
                        break;
                    }
                    quit = 1;
                    sf_cmd_ok();
                    break;
                }

                // Configuration commands

                // type
                if (strcmp(token, "type") == 0)
                {
                    if (argCount != 2)
                    {
                        print_arg_error(1, argCount);
                        break;
                    }
                    token = strtok(NULL, delim);
                    printf("TYPE ARGUMENT: %s\n", token);
                    quit = 1;
                    sf_cmd_ok();
                    break;
                }

                // printer
                if (strcmp(token, "printer") == 0)
                {
                    if (argCount != 3)
                    {
                        print_arg_error(2, argCount);
                        break;
                    }
                    sf_cmd_ok();
                    break;
                }

                // conversion
                if (strcmp(token, "conversion") == 0)
                {
                    if (argCount != 4 || argCount != 5)
                    {
                        print_arg_error(5, argCount);
                        break;
                    }
                    sf_cmd_ok();
                    break;
                }

                // informational commands
                // printers
                if (strcmp(token, "printers") == 0)
                {
                    if (argCount != 1)
                    {
                        print_arg_error(0, argCount);
                        break;
                    }
                    sf_cmd_ok();
                    break;
                }
                // jobs
                if (strcmp(token, "jobs") == 0)
                {
                    if (argCount != 1)
                    {
                        print_arg_error(0, argCount);
                        break;
                    }
                    sf_cmd_ok();
                    break;
                }

                // spooling commands
                // print
                if (strcmp(token, "print") == 0)
                {
                    if (argCount != 2 || argCount != 3)
                    {
                        print_arg_error(2, argCount);
                        break;
                    }
                    sf_cmd_ok();
                    break;
                }

                // cancel
                if (strcmp(token, "cancel") == 0)
                {
                    if (argCount != 2)
                    {
                        print_arg_error(1, argCount);
                        break;
                    }
                    sf_cmd_ok();
                    break;
                }

                // pause
                if (strcmp(token, "pause") == 0)
                {
                    if (argCount != 2)
                    {
                        print_arg_error(1, argCount);
                        break;
                    }
                    sf_cmd_ok();
                    break;
                }

                // resume
                if (strcmp(token, "resume") == 0)
                {
                    if (argCount != 2)
                    {
                        print_arg_error(1, argCount);
                        break;
                    }
                    sf_cmd_ok();
                    break;
                }

                // disbale
                if (strcmp(token, "disable") == 0)
                {
                    if (argCount != 2)
                    {
                        print_arg_error(1, argCount);
                        break;
                    }
                    sf_cmd_ok();
                    break;
                }

                // enable
                if (strcmp(token, "enable") == 0)
                {
                    if (argCount != 2)
                    {
                        print_arg_error(1, argCount);
                        break;
                    }
                    sf_cmd_ok();
                    break;
                }

                token = strtok(NULL, delim);
            }

            if (quit)
            {
                free(input);
                break;
            }

            free(input);
        }
    }

    // fprintf(stderr, "You have to implement run_cli() before the application will function.\n");
    // abort();
    return 0;
}
