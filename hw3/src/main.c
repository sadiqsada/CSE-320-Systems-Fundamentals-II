#include <stdio.h>
#include "sfmm.h"

#define WSIZE 2
#define DSIZE 4

#define PUT(p, val) (*(unsigned int *)(p) = (val))
#define PACK(size, al, pal) ((size) | (al) | (pal << 1))

int main(int argc, char const *argv[])
{
    sf_malloc(32);

    sf_show_heap();
    // double *ptr = sf_malloc(sizeof(double));

    // *ptr = 320320320e-320;

    // printf("%f\n", *ptr);

    // sf_free(ptr);

    return EXIT_SUCCESS;
}
