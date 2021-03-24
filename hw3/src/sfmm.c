/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "sfmm.h"
#include "header.h"

#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define HSIZE 8 // header size = 8
#define WSIZE 8 // word size = 2

#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))
#define PACK(size, al, pal) ((size) | (al) | (pal << 1))

#define GET_SIZE(p) (GET(p) & ~0xf)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define GET_PREV_ALLOC(p) (GET(p) & 0x2) >> 1;

#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - 2 * WSIZE)

#define NEXT_BLK(bp) (bp->body.links.next)
#define PREV_BLK(bp) (bp->body.links.prev)

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE((char *)(bp)-2 * WSIZE))

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

// insert a block of size (size)
void insert_free_block(size_t size, sf_block *block)
{
    int index = find_free_list_index(size);

    // size = size, prev = 1, alloc = 0
    sf_header *blockHeader = (sf_header *)((void *)(block));
    *blockHeader = size + PREV_BLOCK_ALLOCATED;

    sf_footer *blockFooter = (sf_footer *)((void *)(block) + size - 8);
    *blockFooter = size + PREV_BLOCK_ALLOCATED;

    // new free block should be inserted to the front of the list

    sf_block *startingBlock = (struct sf_block *)(&sf_free_list_heads[index]); // dummy node index

    block->body.links.next = startingBlock->body.links.next;
    block->body.links.prev = startingBlock;
    (startingBlock->body.links.next)->body.links.prev = block;
    startingBlock->body.links.next = block;
}

void initialize_heap()
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
    PUT((void *)(sf_mem_start() + 8), PACK(32, 1, 1));
    PUT((void *)(sf_mem_end() - 8), PACK(0, 1, 1));

    // put rest of the memory in wilderness block
    sf_block *block = (struct sf_block *)(sf_mem_start() + 40);
    insert_free_block(8144, block);
}

void *place(int index, size_t size)
{
    while (index < 8)
    {
        // find the address of the block to be searched first
        sf_block *startingBlock = (sf_block *)(&sf_free_list_heads[index]);

        sf_block *iteratorBlock = startingBlock->body.links.next;
        while (iteratorBlock != startingBlock)
        {
            sf_header *iteratorHeader = (sf_header *)((void *)(iteratorBlock));
            size_t iteratorSize = GET_SIZE(iteratorBlock);
            if (iteratorSize >= size)
            {
                // found block to allocate to
                size_t remSize = iteratorSize - size;

                sf_footer *prevFooter = (sf_footer *)((void *)(iteratorBlock)-8);

                sf_header *prevHeader = (sf_header *)((void *)(iteratorBlock)-GET_SIZE(prevFooter));

                int prevAlloc = GET_PREV_ALLOC(prevHeader);

                if (prevAlloc != 1)
                    prevAlloc = 0;

                PUT(iteratorHeader, PACK(size, THIS_BLOCK_ALLOCATED, prevAlloc));

                int alignedSize = find_multiple(remSize);

                if (alignedSize >= 32)
                {
                    sf_block *address = (struct sf_block *)((void *)(iteratorBlock) + size);

                    insert_free_block(alignedSize, address);

                    sf_block *prev = (struct sf_block *)(iteratorBlock->body.links.prev);
                    sf_block *next = (struct sf_block *)(iteratorBlock->body.links.next);

                    prev->body.links.next = next;
                    next->body.links.prev = prev;
                }
                return &(iteratorBlock->body.payload);
            }
            iteratorBlock = iteratorBlock->body.links.next;
        }
        index++;
    }
    return NULL;
}

void set_mem_grow_header(void *start)
{

    sf_footer *prevFooter = (sf_footer *)((start)-16);

    sf_header *prevHeader = (sf_header *)((start)-8 - GET_SIZE(prevFooter));

    int prevAlloc = GET_ALLOC(prevHeader);
    PUT((void *)(start - 8), PACK(PAGE_SZ, 0, prevAlloc));
    PUT((void *)(sf_mem_end() - 16), PACK(PAGE_SZ, 0, prevAlloc));
}

void set_mem_grow_epilogue()
{
    PUT((void *)(sf_mem_end() - 8), PACK(0, 1, 1));
}

// allocates a block at index (index) of size (size)
void *allocate_block(int index, size_t size)
{
    void *placedSuccess = place(index, size);

    if (placedSuccess == NULL)
    {
        // could not place block in current heap, grow heap
        void *bp = sf_mem_grow();
        while (bp != NULL)
        {
            set_mem_grow_header(bp);
            set_mem_grow_epilogue();
            void *coalescedBlock = coalesce(bp);
            size_t cSize = GET_SIZE(HDRP(coalescedBlock));
            size_t bpSize = GET_SIZE(HDRP(bp));
            if (cSize == bpSize) // didn't coalesce anything, add new block to free list
            {
                size_t blockSize = GET_SIZE(HDRP(coalescedBlock));
                sf_block *blockP = (sf_block *)(HDRP(coalescedBlock));
                insert_free_block(blockSize, blockP);
            }
            placedSuccess = place(index, size);
            if (placedSuccess != NULL)
            {
                return placedSuccess;
            }
            bp = sf_mem_grow();
        }
    }

    return NULL;
}

void *sf_malloc(size_t size)
{
    // this is the first malloc
    if (!firstMallocDone)
    {
        firstMallocDone = 1;
        initialize_heap();
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
    char *foundBlock = allocate_block(index, newSize);

    return foundBlock;
}

int validatePointer(void *pp)
{
    // check if null
    if (pp == NULL)
    {
        return 1;
    }

    // check if divisible by 16
    // if (GET(pp) % 16 != 0)
    // {
    //     return 1;
    // }

    // size of the block is not a multiple of 16
    if (GET_SIZE(pp) % 16 != 0)
    {
        return 1;
    }

    // size of block is less than minimum block size
    if (GET_SIZE(pp) < 32)
    {
        return 1;
    }

    // allocated bit in the header is 0
    if (GET_ALLOC(pp) == 0)
    {
        return 1;
    }

    // some or all of the block lies outside of the current heap

    // The header of the next block lies outside the current heap bounds

    // the prev_alloc bit does not match alloc of prev block

    return 0;
}

void *coalesce(void *bp)
{
    size_t prev_alloc = GET_PREV_ALLOC(HDRP((bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    if (prev_alloc && next_alloc)
    { /* Case 1 */
        return bp;
    }
    else if (prev_alloc && !next_alloc)
    { /* Case 2 */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0, 1));
        PUT(FTRP(bp), PACK(size, 0, 1));
    }
    else if (!prev_alloc && next_alloc)
    { /* Case 3 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        size_t prevAlloc = GET_PREV_ALLOC(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0, prevAlloc));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0, prevAlloc));
        bp = PREV_BLKP(bp);
    }
    else
    { /* Case 4 */
        size_t prevAlloc = GET_PREV_ALLOC(HDRP(PREV_BLKP(bp)));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
                GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0, prevAlloc));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0, prevAlloc));
        bp = PREV_BLKP(bp);
    }
    return bp;
}

void sf_free(void *pp)
{
    int validationSuccess = validatePointer(pp);
    if (validationSuccess)
        return;

    size_t size = GET_SIZE(HDRP(pp));
    PUT(HDRP(pp), PACK(size, 0, 0));
    PUT(HDRP(pp), PACK(size, 0, 0));

    coalesce(pp);

    sf_block *block = (sf_block *)(HDRP(pp));
    insert_free_block(GET_SIZE(HDRP(pp)), block);
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
