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

int run_cli(FILE *in, FILE *out)
{
    // TO BE IMPLEMENTED
    char *s = sf_readline("imp>");
    while (*s == '\0')
    {
        sf_readline("imp>");
    }
    // fprintf(stderr, "You have to implement run_cli() before the application will function.\n");
    // abort();
    return 0;
}
