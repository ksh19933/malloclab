// /*
//  * mm-naive.c - The fastest, least memory-efficient malloc package.
//  * 
//  * In this naive approach, a block is allocated by simply incrementing
//  * the brk pointer.  A block is pure payload. There are no headers or
//  * footers.  Blocks are never coalesced or reused. Realloc is
//  * implemented directly using mm_malloc and mm_free.
//  *
//  * NOTE TO STUDENTS: Replace this header comment with your own header
//  * comment that gives a high level description of your solution.
//  */
// #include <stdio.h>
// #include <stdlib.h>
// #include <assert.h>
// #include <unistd.h>
// #include <string.h>

// #include "mm.h"
// #include "memlib.h"

// /*********************************************************
//  * NOTE TO STUDENTS: Before you do anything else, please
//  * provide your team information in the following struct.
//  ********************************************************/
// team_t team = {
//     /* Team name */
//     "ateam",
//     /* First member's full name */
//     "Harry Bovik",
//     /* First member's email address */
//     "bovik@cs.cmu.edu",
//     /* Second member's full name (leave blank if none) */
//     "",
//     /* Second member's email address (leave blank if none) */
//     ""
// };

// /* single word (4) or double word (8) alignment */
// #define WSIZE 4
// #define DSIZE 8
// #define CHUNKSIZE (1<<12)
// #define MAX(x,y) ((x)>(y)? (x):(y))

// /*pac a size and allocated bit into a word*/
// #define PACK(size, alloc) ((size)|(alloc))
// /* Read and write a word at address p */
// #define GET(p) (*(unsigned int *)(p))
// #define PUT(p, val) (*(unsigned int *)(p) = (val))
// /* Read the size and allocated fields from address p */
// #define GET_SIZE(p) (GET(p) & ~0x7)
// #define GET_ALLOC(p) (GET(p) & 0x1)
// /* Given block ptr bp, compute address of its header and footer */
// #define HDRP(bp) ((char *)(bp) - WSIZE)
// #define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
// /* Given block ptr bp, compute address of next and previous blocks */
// #define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE((char *)(bp) - WSIZE))
// #define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))

// static void* heap_listp;
// static void* extend_heap(size_t words);
// static void* coalesce(void *bp);
// static void* find_fit(size_t asize);
// static void place(void *bp, size_t asize);
// /* 
//  * mm_init - initialize the malloc package.
//  */
// int mm_init(void)
// {
//     /*empty heap 생성*/
//     //heap_listp는 mem_start_brk, mem_brk += 16
//     if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
//      return -1;
//     PUT(heap_listp, 0); // heap_listp의 첫 주소에 값을 0으로 한다.  *(unsigned int*)mem_start_brk = 0
//     PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1)); //*(unsigned int*)(mem_start_brk+4) = 1001
//     PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1)); //*(unsigned int*)(mem_start_brk+8) = 1001
//     PUT(heap_listp + (3*WSIZE), PACK(0, 1)); //*(unsigned int*)(mem_start_brk+12) = 1
//     heap_listp += (2*WSIZE); // heap_listp = mem_start_brk + 8 
//     if(extend_heap(CHUNKSIZE/WSIZE) == NULL)
//      return -1;
//     return 0;
// }

// static void* extend_heap(size_t words){
//     char *ptr;
//     size_t size;
//     size = (words % 2) ? (words +1) *WSIZE : words * WSIZE; // 짝수일 경우 words * 4 홀수 일 경우 (words+1) * 4
//     if((long)(ptr = mem_sbrk(size)) == -1)
//      return NULL; //heap 사이즈를 초과하는지 check
//     PUT(HDRP(ptr), PACK(size, 0)); // PUT(((char *)(bp)-4), (size | 0)) --> -4를 하는 이유는 epilogue block을 사용하기 때문에
//     PUT(FTRP(ptr), PACK(size, 0)); // PUT(((char *)(bp)+GET_SIZE(HDRP(bp))-8), size| 0)
//                                   // GET_SIZE(HDRP(bp)) = GET_SIZE((char *)(bp)-4) = GET_SIZE((char *)old_brk-4) 
//                                   // = GET(old_brk-4) & ~0x7 = *(unsigned int *)(old_brk-4) & ~0x7
//     PUT(HDRP(NEXT_BLKP(ptr)), PACK(0 , 1)); // NEXT_BLKP(bp) = (char *)(bp) + GET_SIZE(((char *)(bp) - 4))
//     return coalesce(ptr);
// }

// // 뒤에 head정보와 앞의 footer 정보에서 free block일 경우 병합
// static void* coalesce(void *ptr){
//     size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(ptr)));
//     size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
//     size_t size = GET_SIZE(HDRP(ptr));
//     // 양쪽 모두 사용되고 있는 경우
//     if (prev_alloc && next_alloc){
//         return ptr;
//     }
//     // 왼쪽은 사용 오른쪽은 free
//     else if (prev_alloc && !next_alloc){
//         size += GET_SIZE(HDRP(NEXT_BLKP(ptr)));
//         PUT(HDRP(ptr), PACK(size, 0));
//         PUT(FTRP(ptr), PACK(size, 0));
//     }
//     // 왼쪽은 free 오른쪽은 사용
//     else if (!prev_alloc && next_alloc){
//         size += GET_SIZE(HDRP(PREV_BLKP(ptr)));
//         PUT(FTRP(ptr), PACK(size, 0));
//         PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
//         ptr = PREV_BLKP(ptr);
//     }
//     // 양쪽 모두 free (!prev_alloc && next_alloc)
//     else{
//         size += GET_SIZE(HDRP(PREV_BLKP(ptr))) + GET_SIZE(FTRP(NEXT_BLKP(ptr)));
//         PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
//         PUT(FTRP(NEXT_BLKP(ptr)), PACK(size, 0));
//         ptr = PREV_BLKP(ptr);
//     }
//     return ptr;
// }

// /* 
//  * mm_malloc - Allocate a block by incrementing the brk pointer.
//  *     Always allocate a block whose size is a multiple of the alignment.
//  */
// void *mm_malloc(size_t size)
// {
//     size_t asize;
//     size_t extendsize;
//     char *ptr;

//     if(size == 0) return NULL;

//     if (size <= DSIZE){
//         asize = 2 * DSIZE;
//     }else{
//         asize = DSIZE * ((size + (DSIZE) + (DSIZE-1))/DSIZE);
//     }
//     if ((ptr = find_fit(asize)) != NULL){
//         place(ptr, asize);
//         return ptr;
//     }
//     extendsize = MAX(asize, CHUNKSIZE);
//     if((ptr = extend_heap(extendsize/WSIZE)) == NULL) return NULL;
//     place(ptr, asize);
//     return ptr;
// }

// static void* find_fit(size_t asize){
//     char *ptr;
//     for (ptr = (char *)heap_listp; GET_SIZE(HDRP(ptr)) > 0; ptr = NEXT_BLKP(ptr)) {
//         if(!GET_ALLOC(HDRP(ptr)) && GET_SIZE(HDRP(ptr)) >= asize){
//             return ptr;
//         }
//     }
//     return NULL;
// }

// static void place(void *ptr, size_t asize){
//     size_t csize = GET_SIZE(HDRP(ptr));
//     if((csize-asize) >= (2*DSIZE)){
//         PUT(HDRP(ptr), PACK(asize, 1));
//         PUT(FTRP(ptr), PACK(asize, 1));
//         ptr = NEXT_BLKP(ptr);
//         PUT(HDRP(ptr), PACK(csize-asize, 0));
//         PUT(FTRP(ptr), PACK(csize-asize, 0));
//     }else{
//         PUT(HDRP(ptr), PACK(csize, 1));
//         PUT(FTRP(ptr), PACK(csize, 1));
//     }
// }

// /*
//  * mm_free - Freeing a block does nothing.
//  */
// void mm_free(void *ptr)
// {
//     size_t size = GET_SIZE(HDRP(ptr));
//     PUT(HDRP(ptr), PACK(size, 0));
//     PUT(FTRP(ptr), PACK(size, 0));
//     coalesce(ptr);
// }

// /*
//  * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
//  */
// void *mm_realloc(void *ptr, size_t size)
// {
//     void *oldptr = ptr;
//     void *newptr;
//     size_t copySize;
    
//     newptr = mm_malloc(size);
//     if (newptr == NULL)
//       return NULL;
//     copySize = GET_SIZE(HDRP(oldptr));
//     if (size < copySize)
//       copySize = size;
//     memcpy(newptr, oldptr, copySize);
//     mm_free(oldptr);
//     return newptr;
// }


// next_fit 구현
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)
#define MAX(x,y) ((x)>(y)? (x):(y))

/*pac a size and allocated bit into a word*/
#define PACK(size, alloc) ((size)|(alloc))
/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))
/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE((char *)(bp) - WSIZE))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))

static void* heap_listp;
char* last_ptr;
static void* extend_heap(size_t words);
static void* coalesce(void *bp);
static void* next_fit(size_t asize);
static void place(void *bp, size_t asize);
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /*empty heap 생성*/
    //heap_listp는 mem_start_brk, mem_brk += 16
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
     return -1;
    PUT(heap_listp, 0); // heap_listp의 첫 주소에 값을 0으로 한다.  *(unsigned int*)mem_start_brk = 0
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1)); //*(unsigned int*)(mem_start_brk+4) = 1001
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1)); //*(unsigned int*)(mem_start_brk+8) = 1001
    PUT(heap_listp + (3*WSIZE), PACK(0, 1)); //*(unsigned int*)(mem_start_brk+12) = 1
    heap_listp += (2*WSIZE); // heap_listp = mem_start_brk + 8 
    if(extend_heap(CHUNKSIZE/WSIZE) == NULL)
     return -1;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 * Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *ptr;

    if(size == 0) return NULL;

    if (size <= DSIZE){
        asize = 2 * DSIZE;
    }else{
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1))/DSIZE);
    }
    if ((ptr = next_fit(asize)) != NULL){
        place(ptr, asize);
        return ptr;
    }
    extendsize = MAX(asize, CHUNKSIZE);
    if((ptr = extend_heap(extendsize/WSIZE)) == NULL) return NULL;
    place(ptr, asize);
    return ptr;
}

/* 
   적당한 크기의 가용블록 검색. 
   이 때, next_fit 방법을 이용(최근에 할당된 블록을 기준으로 다음 블록 검색)
*/
static void *next_fit(size_t asize) {
    char *ptr = last_ptr;

    for (ptr = NEXT_BLKP(ptr); GET_SIZE(HDRP(ptr)); ptr = NEXT_BLKP(ptr)) {
        if (!GET_ALLOC(HDRP(ptr)) && GET_SIZE(HDRP(ptr)) >= asize) {
            return ptr;
        }
    }
    ptr = heap_listp;
    for (ptr = NEXT_BLKP(ptr); GET_SIZE(HDRP(ptr)); ptr = NEXT_BLKP(ptr)) {
        if (!GET_ALLOC(HDRP(ptr)) && GET_SIZE(HDRP(ptr)) >= asize) {
            return ptr;
        }
    }
    return NULL;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
}

static void* extend_heap(size_t words){
    char *ptr;
    size_t size;
    size = (words % 2) ? (words +1) *WSIZE : words * WSIZE; // 짝수일 경우 words * 4 홀수 일 경우 (words+1) * 4
    if((long)(ptr = mem_sbrk(size)) == -1)
     return NULL; //heap 사이즈를 초과하는지 check
    PUT(HDRP(ptr), PACK(size, 0)); // PUT(((char *)(bp)-4), (size | 0)) --> -4를 하는 이유는 epilogue block을 사용하기 때문에
    PUT(FTRP(ptr), PACK(size, 0)); // PUT(((char *)(bp)+GET_SIZE(HDRP(bp))-8), size| 0)
                                  // GET_SIZE(HDRP(bp)) = GET_SIZE((char *)(bp)-4) = GET_SIZE((char *)old_brk-4) 
                                  // = GET(old_brk-4) & ~0x7 = *(unsigned int *)(old_brk-4) & ~0x7
    PUT(HDRP(NEXT_BLKP(ptr)), PACK(0 , 1)); // NEXT_BLKP(bp) = (char *)(bp) + GET_SIZE(((char *)(bp) - 4))
    return coalesce(ptr);
}

// 뒤에 head정보와 앞의 footer 정보에서 free block일 경우 병합
static void* coalesce(void *ptr){
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(ptr)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
    size_t size = GET_SIZE(HDRP(ptr));
    // 양쪽 모두 사용되고 있는 경우
    if (prev_alloc && next_alloc){
        last_ptr = ptr;
        return ptr;
    }
    // 왼쪽은 사용 오른쪽은 free
    else if (prev_alloc && !next_alloc){
        size += GET_SIZE(HDRP(NEXT_BLKP(ptr)));
        PUT(HDRP(ptr), PACK(size, 0));
        PUT(FTRP(ptr), PACK(size, 0));
    }
    // 왼쪽은 free 오른쪽은 사용
    else if (!prev_alloc && next_alloc){
        size += GET_SIZE(HDRP(PREV_BLKP(ptr)));
        PUT(FTRP(ptr), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
        ptr = PREV_BLKP(ptr);
    }
    // 양쪽 모두 free (!prev_alloc && next_alloc)
    else{
        size += GET_SIZE(HDRP(PREV_BLKP(ptr))) + GET_SIZE(FTRP(NEXT_BLKP(ptr)));
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(size, 0));
        ptr = PREV_BLKP(ptr);
    }
    last_ptr = ptr;
    return ptr;
}

static void place(void *ptr, size_t asize){
    size_t csize = GET_SIZE(HDRP(ptr));
    if((csize-asize) >= (2*DSIZE)){
        PUT(HDRP(ptr), PACK(asize, 1));
        PUT(FTRP(ptr), PACK(asize, 1));
        ptr = NEXT_BLKP(ptr);
        PUT(HDRP(ptr), PACK(csize-asize, 0));
        PUT(FTRP(ptr), PACK(csize-asize, 0));
    }else{
        PUT(HDRP(ptr), PACK(csize, 1));
        PUT(FTRP(ptr), PACK(csize, 1));
    }
}


/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *bp, size_t size) {
    size_t old_size = GET_SIZE(HDRP(bp));
    size_t new_size = size + (2 * WSIZE);   // 2*WISE는 헤더와 풋터

    // new_size가 old_size보다 작거나 같으면 기존 bp 그대로 사용
    if (new_size <= old_size) {
        return bp;
    }
    // new_size가 old_size보다 크면 사이즈 변경
    else {
        size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
        size_t current_size = old_size + GET_SIZE(HDRP(NEXT_BLKP(bp)));

        // next block이 가용상태이고 old, next block의 사이즈 합이 new_size보다 크면 그냥 그거 바로 합쳐서 쓰기
        if (!next_alloc && current_size >= new_size) {
            PUT(HDRP(bp), PACK(current_size, 1));
            PUT(FTRP(bp), PACK(current_size, 1));
            return bp;
        }
        // 아니면 새로 block 만들어서 거기로 옮기기
        else {
            void *new_bp = mm_malloc(new_size);
            place(new_bp, new_size);
            memcpy(new_bp, bp, new_size);  // 메모리의 특정한 부분으로부터 얼마까지의 부분을 다른 메모리 영역으로 복사해주는 함수(old_bp로부터 new_size만큼의 문자를 new_bp로 복사해라!)
            mm_free(bp);
            return new_bp;
        }
    }
}

