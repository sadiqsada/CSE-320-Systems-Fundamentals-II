/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "sfmm.h"

#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define HSIZE // header size = 8
#define WSize // word size = 2

#define GET(p) (*(unsigned int *))(p))
#define PUT(p, val) (*(unsigned int *)(p)) = (val))
#define PACK(size, al, pal) ((size) | (al) | (pal << 1))

#define GET_SIZE(p) (GET(p) & ~0xf)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define GET_PREV_ALLOC(p) (GET(p) & 0x2)

#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE

#define NEXT_BLK(bp) (bp->body.links.next)
#define PREV_BLK(bp) (bp->body.links.prev)

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))

#define SET_NEXT_BLK(bp, next) (NEXT_BLK(bp) = next)
#define SET_PREV_BLK(bp, prev) (PREV_BLK(bp) = prev)

int firstMallocDone = 0; // tracks whether a malloc is the first one

// finds next multiple of 16 for num input
size_t find_multiple(int num)
{
    return (num % 16 != 0) ? num + (16 - (num % 16)) : num;
}

// finds the appropriate index in free list to search
int find_free_list_index(size_t size)
{
    int answer = 0;
    int pow = 32;
    int target = 32 * 32;
    while (pow <= target)
    {
        if (size <= pow)
        {
            return answer;
        }
        pow = pow * 2;
        answer = answer + 1;
    }
    return 7;
}

void initializeHeap()
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
    sf_footer *blockFooter = (sf_footer *)(block + 8136);
    *blockFooter = block->header;

    sf_block *startingBlock = (struct sf_block *)(&sf_free_list_heads[7]);
    startingBlock->body.links.next = block;
    startingBlock->body.links.prev = block;
    block->body.links.next = startingBlock;
    block->body.links.prev = startingBlock;
}

// insert a block of size (size)
void insert_free_block(size_t size, sf_block *block)
{
    int index = find_free_list_index(size);

    // size = size, prev = 1, alloc = 0
    sf_header *blockHeader = (sf_header *)(&(block->header));
    *blockHeader = size + PREV_BLOCK_ALLOCATED;

    sf_footer *blockFooter = (sf_footer *)(&(block->header) + size - 8);
    *blockFooter = *blockHeader;

    // new free block should be inserted to the front of the list

    sf_block *startingBlock = (struct sf_block *)(&sf_free_list_heads[index]); // dummy node index

    block->body.links.next = startingBlock->body.links.next;
    block->body.links.prev = startingBlock;
    (startingBlock->body.links.next)->body.links.prev = block;
    startingBlock->body.links.next = block;
}

// allocates a block at index (index) of size (size)
sf_block *allocate_block(int index, size_t size)
{
    while (index < 8)
    {
        // find the address of the block to be searched first
        sf_block *startingBlock = (sf_block *)(&sf_free_list_heads[index]);

        size_t inputSize = size; // redundant

        sf_block *iteratorBlock = startingBlock->body.links.next;
        while (iteratorBlock != startingBlock)
        {
            sf_header *iteratorHeader = (sf_header *)(&(iteratorBlock->header));
            size_t iteratorSize = *(iteratorHeader) & ~0xf;
            if (iteratorSize >= inputSize)
            {
                // found block to allocate to
                // size_t remSize = iteratorSize - inputSize;

                sf_header *prevHeader = (&((PREV_BLK(iteratorBlock))->header));

                int prevAlloc = (*prevHeader) & 0x1;

                *(iteratorHeader) = inputSize + THIS_BLOCK_ALLOCATED + prevAlloc;

                int padding = 0;

                if (inputSize % 16 == 0)
                {
                    padding = 8;
                }

                sf_block *address = (struct sf_block *)(iteratorBlock + inputSize + padding);

                printf("Address: %p\n", address);

                // SET_NEXT_BLK(PREV_BLK(iteratorBlock), address);

                // SET_PREV_BLK(address, PREV_BLK(iteratorBlock));

                // if (find_multiple(remSize) >= 32)
                // {
                //     insert_free_block(remSize, address);
                // }
                return iteratorBlock;
            }
            iteratorBlock = iteratorBlock->body.links.next;
        }
        index++;
    }
    return NULL;
}

void *sf_malloc(size_t size)
{
    // this is the first malloc
    if (!firstMallocDone)
    {
        firstMallocDone = 1;
        initializeHeap();
    }
    // if size is 0, return null
    if (size == 0)
        return NULL;

    // determine padding size

    size_t newSize = 0;

    if (size <= 24)
    {
        // allocate block of size 32
        newSize = 32;
    }

    else
    {
        // allocate block of size x + 8 + (padding)
        newSize = size + 8;
        newSize = find_multiple(newSize);
    }

    // determine the index of the free list to be searched
    int index = find_free_list_index(newSize);
    // search the list for an appropriate free block
    sf_block *foundBlock = allocate_block(index, newSize);

    return foundBlock;
}

void sf_free(void *pp)
{
    return;
}

void *sf_realloc(void *pp, size_t rsize)
{
    return NULL;
}

void *sf_memalign(size_t size, size_t align)
{
    return NULL;
}
