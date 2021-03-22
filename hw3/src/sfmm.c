/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "sfmm.h"

#define WSIZE 2
#define DSIZE 4

#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define PACK(size, alloc) ((size) | (alloc))

#define GET(p) (*(unsigned int *))(p))
#define PUT(p, val) (*(unsigned int *)(p)) = (val))

#define GET_SIZE(p) (GET(p) & ~0xf)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define GET_PREV_ALLOC(p) (GET(p) & 0x2)

#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))

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
sf_block *allocateBlock(int index, size_t size)
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
                size_t remSize = iteratorSize - inputSize;

                sf_header *header = (&((iteratorBlock->body.links.prev)->header));

                int prevAlloc = (*header) & 0x2;

                *(iteratorHeader) = inputSize + THIS_BLOCK_ALLOCATED + prevAlloc;

                if (remSize >= 32)
                {
                    sf_block *address = (struct sf_block *)(iteratorBlock + inputSize);
                    insert_free_block(remSize, address);
                }
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
    sf_block *foundBlock = allocateBlock(index, newSize);

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
