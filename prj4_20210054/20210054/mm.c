/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

 /*********************************************************
  * NOTE TO STUDENTS: Before you do anything else, please
  * provide your information in the following struct.
  ********************************************************/


#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE  (1<<12)

#define MAX(x, y) ((x) > (y)? (x) : (y))  
#define MIN(x, y) ((x) < (y)? (x) : (y))  

#define PACK(size, alloc)  ((size) | (alloc))
#define GET(p)       (*(unsigned int *)(p))
#define PUT(p, val)  (*(unsigned int *)(p) = (val))

#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define HDRP(bp)       ((char *)(bp) - WSIZE)
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

#define SEGLIST 20
#define MINSIZE 16


#define GET_PREV_PTR(bp) (*(char **)(bp))
#define GET_NEXT_PTR(bp) (*(char **)((char *)(bp) + WSIZE))
#define SET_PREV_PTR(bp, ptr) (*(char **)(bp) = (ptr))
#define SET_NEXT_PTR(bp, ptr) (*(char **)((char *)(bp) + WSIZE) = (ptr))

static void** segFList;

team_t team = {
    /* Your student ID */
    "20210054",
    /* Your full name*/
    "JeongHyun Byun",
    /* Your email address */
    "bjh121232@naver.com",
};
/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

static void* extend_heap(size_t words);
static void* place(void* bp, size_t asize);
static void* coalesce(void* bp);
static void insert_free_block(void* bp, size_t size);
static void remove_from_free_list(void* bp);
//새 함수 추가(인덱스 얻는 것)
static int get_seglist_index(size_t size);
void* search_fit_block(size_t size);

static char* heap_listp = 0;
static void* seglist[SEGLIST];


int mm_init(void)
{
    // 먼저 segFList를 위한 공간을 따로 확보
    void* seglist_base;
    if ((seglist_base = mem_sbrk(SEGLIST * WSIZE)) == (void*)-1)
        return -1;

    segFList = (void**)seglist_base;
    for (int i = 0; i < SEGLIST; i++) {
        segFList[i] = NULL;
    }

    // 그 다음 실제 heap_listp 초기화
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void*)-1)
        return -1;

    PUT(heap_listp, 0);                             // Alignment padding
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));  // Prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));  // Prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));      // Epilogue header
    heap_listp += (2 * WSIZE);

    if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;

    return 0;
}


void* search_fit_block(size_t size) {
    void* curr = NULL;

    for (int i = 0, range = MINSIZE; i < SEGLIST; i++, range *= 2) {
        // 현재 리스트가 조건을 만족하고 비어 있지 않은 경우만 탐색
        if (size <= range || i == SEGLIST - 1) {
            curr = segFList[i];
            while (curr && size > GET_SIZE(HDRP(curr))) {
                curr = GET_NEXT_PTR(curr);
            }
            if (curr != NULL) {
                return curr;
            }
        }
    }
    return NULL;  // 못 찾았으면 NULL
}


static size_t align_request_size(size_t size) {
    if (size <= DSIZE)
        return 2 * DSIZE;
    return DSIZE * ((size + DSIZE + (DSIZE - 1)) / DSIZE);
}

void* mm_malloc(size_t size)
{
    if (size == 0)
        return NULL;

    size_t adjusted_size = align_request_size(size);

  
    void* block = search_fit_block(adjusted_size);


    if (block == NULL) {
        if ((block = extend_heap(adjusted_size)) == NULL) {
            return NULL;
        }
    }


    return place(block, adjusted_size);
}


void mm_free(void* bp)
{

    if (bp == NULL)return;
    ssize_t block_size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(block_size, 0));
    PUT(FTRP(bp), PACK(block_size, 0));
    
    insert_free_block(bp, block_size);
    coalesce(bp);

    return;
}

static void* coalesce(void* bp) {
    void* prev = PREV_BLKP(bp);
    void* next = NEXT_BLKP(bp);

    size_t prev_alloc = GET_ALLOC(HDRP(prev));
    size_t next_alloc = GET_ALLOC(HDRP(next));
    size_t size = GET_SIZE(HDRP(bp));

    
    remove_from_free_list(bp);
    if (!prev_alloc) remove_from_free_list(prev);
    if (!next_alloc) remove_from_free_list(next);

    // case 1: both allocated → nothing to coalesce
    if (prev_alloc && next_alloc) {
        insert_free_block(bp, size);
        return bp;
    }
    // case 2: next free
    else if (prev_alloc && !next_alloc) {
        size += GET_SIZE(HDRP(next));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(next), PACK(size, 0));
        insert_free_block(bp, size);
        return bp;
    }
    // case 3: prev free
    else if (!prev_alloc && next_alloc) {
        size += GET_SIZE(HDRP(prev));
        PUT(HDRP(prev), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        insert_free_block(prev, size);
        return prev;
    }
    // case 4: both free
    else {
        size += GET_SIZE(HDRP(prev)) + GET_SIZE(HDRP(next));
        PUT(HDRP(prev), PACK(size, 0));
        PUT(FTRP(next), PACK(size, 0));
        insert_free_block(prev, size);
        return prev;
    }
}


void* mm_realloc(void* ptr, size_t size)
{
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }

    if (ptr == NULL) {
        return mm_malloc(size);
    }

    size_t new_size = align_request_size(size);
    size_t curr_size = GET_SIZE(HDRP(ptr));

  
    if (new_size <= curr_size) {
        return ptr;
    }


    void* next_blk = NEXT_BLKP(ptr);
    size_t next_size = GET_SIZE(HDRP(next_blk));
    int next_alloc = GET_ALLOC(HDRP(next_blk));

 
    if (!next_alloc || next_size == 0) {
        size_t total_size = curr_size + next_size;

        if (total_size < new_size) {
            size_t extra_needed = new_size - total_size;
            if (extend_heap(extra_needed) == NULL)
                return NULL;
            total_size = curr_size + GET_SIZE(HDRP(next_blk)); 
        }

        remove_from_free_list(next_blk);
        PUT(HDRP(ptr), PACK(total_size, 1));
        PUT(FTRP(ptr), PACK(total_size, 1));

        return ptr;
    }

    void* new_ptr = mm_malloc(size);
    if (new_ptr == NULL)
        return NULL;

    memcpy(new_ptr, ptr, MIN(new_size, curr_size));
    mm_free(ptr);

    return new_ptr;
}

static void* extend_heap(size_t words)
{
    size_t byte_size = (words % 2 == 0) ? words * WSIZE : (words + 1) * WSIZE;

    char* block_ptr = mem_sbrk(byte_size);
    if ((long)block_ptr == -1) {
        return NULL;
    }


    PUT(HDRP(block_ptr), PACK(byte_size, 0));                 // Free block header
    PUT(FTRP(block_ptr), PACK(byte_size, 0));                 // Free block footer
    PUT(HDRP(NEXT_BLKP(block_ptr)), PACK(0, 1));              // New epilogue header

    insert_free_block(block_ptr, byte_size);

    return coalesce(block_ptr);  // 병합 후 반환
}


static void* place(void* bp, size_t alloc_size)
{
    size_t free_size = GET_SIZE(HDRP(bp));
    size_t remainder = free_size - alloc_size;

    remove_from_free_list(bp);

    // Case 1: 나눌 수 없을 만큼 작은 경우 → 전체 할당
    if (remainder <= 2 * DSIZE) {
        PUT(HDRP(bp), PACK(free_size, 1));
        PUT(FTRP(bp), PACK(free_size, 1));
        return bp;
    }

    // Case 2: 일반적인 분할
    PUT(HDRP(bp), PACK(alloc_size, 1));
    PUT(FTRP(bp), PACK(alloc_size, 1));

    void* split_bp = NEXT_BLKP(bp);
    PUT(HDRP(split_bp), PACK(remainder, 0));
    PUT(FTRP(split_bp), PACK(remainder, 0));

    insert_free_block(split_bp, remainder);
    return bp;
}

static void insert_free_block(void* bp, size_t size) {
    int list_idx = get_seglist_index(size);
    void* curr = segFList[list_idx];
    void* prev = NULL;

    while ((curr != NULL) && (GET_SIZE(HDRP(bp)) > GET_SIZE(HDRP(curr)))) {
        prev = curr;
        curr = GET_NEXT_PTR(curr);
    }

    SET_PREV_PTR(bp, prev);
    SET_NEXT_PTR(bp, curr);

    if (prev)
        SET_NEXT_PTR(prev, bp);
    else
        segFList[list_idx] = bp;

    if (curr)
        SET_PREV_PTR(curr, bp);

    return;
}

static void remove_from_free_list(void* bp) {
    ssize_t block_size = GET_SIZE(HDRP(bp));
    void* next = GET_NEXT_PTR(bp);
    void* prev = GET_PREV_PTR(bp);
    int list_index = get_seglist_index(block_size);

    if (prev == NULL) {
        segFList[list_index] = next;
        if (next != NULL)
            SET_PREV_PTR(next, NULL);
    }
    else {
        SET_NEXT_PTR(prev, next);
        if (next != NULL)
            SET_PREV_PTR(next, prev);
    }
}

// 인자로 주어진 size에 대해 적절한 segregated list index 반환
static int get_seglist_index(size_t size) {
    int index = 0;
    ssize_t range = MINSIZE;
    while (index < SEGLIST - 1 && size > range) {
        index++;
        range *= 2;
    }
    return index;
}


