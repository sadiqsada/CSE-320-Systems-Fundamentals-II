#include <stdio.h>
#include "sfmm.h"

#define WSIZE 2
#define DSIZE 4

#define PUT(p, val) (*(unsigned int *)(p) = (val))
#define PACK(size, al, pal) ((size) | (al) | (pal << 1))

int main(int argc, char const *argv[])
{
    // get page of memory
    sf_mem_grow();

    // initialize free lists
    for (int i = 0; i < NUM_FREE_LISTS; i++)
    {
        sf_free_list_heads[i].body.links.next = &sf_free_list_heads[i];
        sf_free_list_heads[i].body.links.prev = &sf_free_list_heads[i];
    }

    // initialize prologue and epilogue
    sf_block *block = (struct sf_block *)(sf_mem_start() + 8);
    block->header = 32 + THIS_BLOCK_ALLOCATED + PREV_BLOCK_ALLOCATED;

    block = (struct sf_block *)(sf_mem_end() - 8);
    block->header = 0 + THIS_BLOCK_ALLOCATED + PREV_BLOCK_ALLOCATED;

    // put rest of the memory in wilderness block
    block = (struct sf_block *)(sf_mem_start() + 40);
    PUT(&(block->header), PACK(8144, 0, 0));
    PUT(&(block->header) + 8136, PACK(8144, 0, 0));

    sf_block *startingBlock = (struct sf_block *)(&sf_free_list_heads[7]);
    startingBlock->body.links.next = block;
    startingBlock->body.links.prev = block;
    block->body.links.next = startingBlock;
    block->body.links.prev = startingBlock;

    sf_show_heap();
    // double *ptr = sf_malloc(sizeof(double));

    // *ptr = 320320320e-320;

    // printf("%f\n", *ptr);

    // sf_free(ptr);

    return EXIT_SUCCESS;
}
