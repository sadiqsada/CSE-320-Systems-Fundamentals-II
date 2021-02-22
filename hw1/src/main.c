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
    if(validargs(argc, argv))
        USAGE(*argv, EXIT_FAILURE);
    if(global_options & HELP_OPTION)
        USAGE(*argv, EXIT_SUCCESS);
    birp_to_birp(stdin, stdout);

    // TO BE IMPLEMENTED

    // least significant four bits = input format
    // int inputmask = 0x000f;
    // int inputformat = inputmask & global_options;

    // // bits 4-7 -> output format
    // int outputmask = inputmask << 4;
    // int outputformat = outputmask & global_options;
    // outputformat = outputformat >> 4;

    // // bits 8-11 -> transformation
    // int transformationmask = outputmask << 4;
    // int transformationtype = transformationmask & global_options;
    // transformationtype = transformationtype >> 8;
    // // specified transform param
    // int parammask = (transformationmask << 4) | (transformationmask << 8);
    // int paramval = parammask & global_options;
    // paramval = paramval >> 16;

    // if(inputformat == 1 && outputformat == 3) {
    // 	int success = pgm_to_ascii(stdin, stdout);
    // 	if(success == -1) return EXIT_FAILURE;
    // }
    // if(inputformat == 1 && outputformat == 2) {
    // 	int success = pgm_to_birp(stdin, stdout);
    // 	if(success == -1) return EXIT_FAILURE;
    // }
    // if(inputformat == 2 && outputformat == 1) {
    // 	int success = birp_to_pgm(stdin, stdout);
    // 	if(success == -1) return EXIT_FAILURE;
    // }
    // if(inputformat == 2 && outputformat == 2) {
    // 	int success = birp_to_birp(stdin, stdout);
    // 	if(success == -1) return EXIT_FAILURE;
    // }
    // if(inputformat == 2 && outputformat == 3) {
    // 	int success = birp_to_ascii(stdin, stdout);
    // 	if(success == -1) return EXIT_FAILURE;
    // }

    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
