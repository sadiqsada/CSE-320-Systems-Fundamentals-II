#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[])
{
    char *ptrA = sf_malloc(0); // Want a 50 byte block that is 128 aligned
    *(ptrA) = 'A';

    sf_show_heap();
    return EXIT_SUCCESS;
}
