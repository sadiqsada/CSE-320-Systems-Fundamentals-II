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

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))

// finds next multiple of 16 for num input
int find_multiple(int num)
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

void *sf_malloc(size_t size)
{
    // if size is 0, return null
    if (size == 0)
        return NULL;

    // determine padded size

    int newSize = 0;

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
    int index = find_free_list_index(size);

    return NULL;
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
