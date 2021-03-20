#include <stdio.h>
#include "sfmm.h"

#define WSIZE 2
#define DSIZE 4

#define PUT(p, val) (*(unsigned int *)(p) = (val))
#define PACK(size, al, pal) ((size) | (al) | (pal << 1))

int main(int argc, char const *argv[])
{
    sf_mem_grow();

    for (int i = 0; i < NUM_FREE_LISTS; i++)
    {
        sf_free_list_heads[i].body.links.next = &sf_free_list_heads[i];
        sf_free_list_heads[i].body.links.prev = &sf_free_list_heads[i];
    }

    PUT(sf_mem_start() + 8, PACK(32, 1, 0));
    PUT(sf_mem_end() - 8, PACK(0, 1, 0));

    sf_show_heap();
    // double *ptr = malloc(sizeof(double));

    // *ptr = 320320320e-320;

    // printf("%f\n", *ptr);

    // free(ptr);

    return EXIT_SUCCESS;
}
