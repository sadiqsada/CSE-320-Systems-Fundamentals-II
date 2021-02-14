/*
 * BIRP: Binary decision diagram Image RePresentation
 */

#include "image.h"
#include "bdd.h"
#include "const.h"
#include "debug.h"

int pgm_to_birp(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    int wp = 0, hp = 0;
    size_t size = RASTER_SIZE_MAX;
    int result = img_read_pgm(in, &wp, &hp, raster_data, size);

    if(result == 0) {
        BDD_NODE *node = bdd_from_raster(wp, hp, raster_data);
        int serializedresult = bdd_serialize(node, out);
        // if(serializedresult == 0) {
        //     return 0;
        // }
    }
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
    int wp = 0, hp = 0;
    size_t size = RASTER_SIZE_MAX;
    int result = img_read_pgm(in, &wp, &hp, raster_data, size);

    if(result == 0) {
        for(int i = 0; i < hp; i++) {
            for(int j = 0; j < wp; j++) {
                int val = *(raster_data + (i * wp) + j);
                if(val >= 0 && val <= 63) {
                    printf("%c", ' ');
                }
                else if(val >= 64 && val <= 127) {
                    printf("%c", '.');
                }
                else if(val >= 128 && val <= 191) {
                    printf("%c", '*');
                }
                else if(val >= 192 && val <= 255) {
                    printf("%c", '@');
                }
            }

            printf("%c", '\n');
        }
    }

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

int convertstrtoint(char *str) {
    int num = *str - '0';
    while(*(str + 1) != '\0') {
        num *= 10;
        int strNum = *(str + 1) - '0';
        num += strNum;
        str++;
    }

    return num;
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
    global_options = 0;

    int isinputbirp = 0;
    int isoutputbirp = 0;

    // first argument is not bin/birp
    char *binbirp = *argv;
    if(strcompare(binbirp, "bin/birp") != 0) {
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

        else if(strcompare(*copyargv, "-i") == 0 ||
            strcompare(*copyargv, "-o") == 0) {
            if(seenoptional == 1) {
                return -1;
            }
        }
        copyargv++;
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

    // check if number of arguments are valid
    if(argc <= 1 || argc > 8) {
        return -1;
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

            // nothing is provided after -i
            if(i == argc - 1) return -1;

            token = *copyargv;
            if(strcompare(token, "birp") == 0) {
                // set global option 0x2
                isinputbirp = 1;
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
        isinputbirp = 1;
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

            // nothing is provided after -o
            if(i == argc - 1) return -1;

            token = *copyargv;
            if(strcompare(token, "birp") == 0) {
                // set global option 0x2
                isoutputbirp = 1;
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
        isoutputbirp = 1;
        int bit = 2;
        bit = bit << 4;
        global_options = global_options | bit;

    }

    copyargv = argv;
    int foundoptional = 0;
    for(int i = 0; i < argc; i++) {
        char *currentcopy = *copyargv;
        if(isinputbirp == 1 && isoutputbirp == 1) {
            if(strcompare(currentcopy, "-n") == 0) {
                // manage -n tag
                // store 0x1 in global_options
                if(foundoptional == 1) return -1;
                int bit = 1;
                bit = bit << 8;
                global_options = global_options | bit;
                foundoptional = 1;
            }
            else if(strcompare(currentcopy, "-r") == 0) {
                // handle -r tag
                // set 0x4 in global_options
                if(foundoptional == 1) return -1;
                int bit = 4;
                bit = bit << 8;
                global_options = global_options | bit;
                foundoptional = 1;
            }
            else if(strcompare(currentcopy, "-t") == 0) {
                // handle -t tag
                // set 0x2
                if(foundoptional == 1) return -1;
                int bit = 2;
                bit = bit << 8;
                global_options = global_options | bit;
                // get threshold param
                char *nextcopy = *(copyargv + 1);
                int threshold = convertstrtoint(nextcopy);
                if(threshold < 0 || threshold > 255) {
                    return -1;
                }
                bit = threshold;
                bit = bit << 16;
                global_options = global_options | bit;
                foundoptional = 1;

            }
            else if(strcompare(currentcopy, "-Z") == 0) {
                // handle -Z tag
                // set 0x3
                int bit = 3;
                bit = bit << 8;
                global_options = global_options | bit;
                // get factor param
                char *nextcopy = *(copyargv + 1);
                int factor = convertstrtoint(nextcopy);
                if(factor < 0 || factor > 16) {
                    return -1;
                }
                bit = factor;
                bit = bit << 16;
                global_options = global_options | bit;
                foundoptional = 1;
            }
            else if(strcompare(currentcopy, "-z") == 0) {
                // handle -Z tag
                // set 0x3
                if(foundoptional == 1) return -1;
                int bit = 3;
                bit = bit << 8;
                global_options = global_options | bit;

                // get factor param
                char *nextcopy = *(copyargv + 1);
                int factor = convertstrtoint(nextcopy);
                if(factor < 0 || factor > 16) {
                    return -1;
                }
                bit = factor;
                // flip all bits
                int flipper = 0xff;
                bit = bit ^ flipper;
                // add 1
                bit++;
                bit = bit << 16;
                global_options = global_options | bit;
                foundoptional = 1;

            }
        }
        else {
            if(strcompare(currentcopy, "-n") == 0 ||
                strcompare(currentcopy, "-r") == 0 ||
                strcompare(currentcopy, "-z") == 0 ||
                strcompare(currentcopy, "-Z") == 0 ||
                strcompare(currentcopy, "-t") == 0) {
                return -1;
            }
        }
        copyargv++;
    }

    return 0;
}
