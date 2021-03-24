#include <stdio.h>
#include "sfmm.h"

void initializeHeap2()
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
    (block->header) = 8144 + PREV_BLOCK_ALLOCATED;
    // (sf_header *)(block + 8136) = 8144 + PREV_BLOCK_ALLOCATED;
    // update footer here

    sf_block *startingBlock = (struct sf_block *)(&sf_free_list_heads[7]);
    startingBlock->body.links.next = block;
    startingBlock->body.links.prev = block;
    block->body.links.next = startingBlock;
    block->body.links.prev = startingBlock;
}

int main(int argc, char const *argv[])
{
    void *p = sf_malloc(48);
    sf_free(p);
    // sf_malloc(38);
    // sf_malloc(50);
    // sf_malloc(8990);
    // sf_malloc(8990);
    // initializeHeap2();
    sf_show_heap();
    // double *ptr = sf_malloc(sizeof(double));

    // *ptr = 320320320e-320;

    // printf("%f\n", *ptr);

    // sf_free(ptr);

    return EXIT_SUCCESS;
}
