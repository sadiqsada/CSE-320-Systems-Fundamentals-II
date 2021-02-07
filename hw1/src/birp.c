/*
 * BIRP: Binary decision diagram Image RePresentation
 */

#include "image.h"
#include "bdd.h"
#include "const.h"
#include "debug.h"

int pgm_to_birp(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    return -1;
}

int birp_to_pgm(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    return -1;
}

int birp_to_birp(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    return -1;
}

int pgm_to_ascii(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    return -1;
}

int birp_to_ascii(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    return -1;
}

/*
    function to compare two strings
    if equal, return 0
    else, return -1
*/

int strcompare(char *str1, char* str2){
    while(*str1 == *str2 && *str1 != '\0' && *str2 != '\0') {
        str1++;
        str2++;
    }
    if(*str1 == '\0' && *str2 == '\0') {
        return 0;
    }
    return -1;
}

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specifed will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere int the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 */

int validargs(int argc, char **argv) {
    // TO BE IMPLEMENTED

    // check if number of arguments are valid
    if(argc <= 1 || argc > 10) {
        return -1;
    }

    // invalid ordering of arguments
    int seenoptional = 0;
    char **copyargv = argv;
    for(int i = 0; i < argc; i++) {
        if(strcompare(*copyargv, "-n") == 0 ||
            strcompare(*copyargv, "-r") == 0 ||
            strcompare(*copyargv, "-t") == 0 ||
            strcompare(*copyargv, "-z") == 0 ||
            strcompare(*copyargv, "-Z") == 0) {
            seenoptional = 1;
        }

        else {
            if(seenoptional == 1) {
                printf("%s\n", "bad");
                return -1;
            }
        }

    }

    // check -h flag
    // store copy of argv - if second argument is -h, return success
    copyargv = argv;
    char *firstarg = *copyargv; // first argument

    copyargv++;
    char *secondarg = *copyargv; // second argument

    int firstargmatch = strcompare(firstarg, "bin/birp");
    int secondargmatch = strcompare(secondarg, "-h");

    if(firstargmatch == 0 && secondargmatch == 0) {
        // set global options most significant bit to 1
        int bit = 1;
        bit = bit << 31;
        global_options = global_options | bit;
        return 0;
    }

    // check -i flag
    int foundi = 0;
    copyargv = argv;
    char *token;
    for(int i = 0; i < argc; i++) {
        char *currentcopy = *copyargv;
        if(strcompare(currentcopy, "-i") == 0) {
            // -i found
            foundi = 1;
            copyargv++;
            token = *copyargv;
            if(strcompare(token, "birp") == 0) {
                // set global option 0x2
                int bit = 2;
                global_options = global_options | bit;
                break;
            }
            else if(strcompare(token, "pgm") == 0) {
                // set global option 0x2
                int bit = 1;
                global_options = global_options | bit;
                break;
            }
            else {
                // format not valid, invalid input
                return -1;
            }

        }
        copyargv++;
    }
    // if -i is not provided, enter birp
    if(foundi == 0) {
        int bit = 2;
        global_options = global_options | bit;
    }

    // check -o flag
    int foundo = 0;
    copyargv = argv;
    for(int i = 0; i < argc; i++) {
        char *currentcopy = *copyargv;
        if(strcompare(currentcopy, "-o") == 0) {
            // -o found
            foundo = 1;
            copyargv++;
            token = *copyargv;
            if(strcompare(token, "birp") == 0) {
                // set global option 0x2
                int bit = 2;
                bit = bit << 4;
                global_options = global_options | bit;
                break;
            }
            else if(strcompare(token, "pgm") == 0) {
                // set global option 0x2
                int bit = 1;
                bit = bit << 4;
                global_options = global_options | bit;
                break;
            }
            else if(strcompare(token, "ascii") == 0) {
                // set global option 0x2
                int bit = 3;
                bit = bit << 4;
                global_options = global_options | bit;
                break;
            }
            else {
                // format not valid, invalid input
                return -1;
            }

        }
        copyargv++;
    }
    // if -i is not provided, enter birp
    if(foundo == 0) {
        int bit = 2;
        bit = bit << 4;
        global_options = global_options | bit;

    }
printf("%04x\n", global_options);

    return -1;
}
