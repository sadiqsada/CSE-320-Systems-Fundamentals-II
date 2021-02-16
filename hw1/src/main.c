#include <stdio.h>
#include <stdlib.h>

#include "const.h"
#include "debug.h"
// #include "image.h" // remove this line

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif


int main(int argc, char **argv)
{
	// printf("%d\n", bdd_lookup(1,3,5));
	// printf("%d\n", bdd_lookup(1,4,5));
	// printf("%d\n", bdd_lookup(1,5,5));
	// printf("%d\n", bdd_lookup(1,3,5));
 //    printf("%d\n", bdd_lookup(1,4,5));

	pgm_to_birp(stdin, stdout);
    if(validargs(argc, argv))
        USAGE(*argv, EXIT_FAILURE);
    if(global_options & HELP_OPTION)
        USAGE(*argv, EXIT_SUCCESS);
    // TO BE IMPLEMENTED
    return EXIT_FAILURE;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
