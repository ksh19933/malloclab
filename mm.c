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


// // next_fit 구현
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
// #define CHUNKSIZE (1<<10)
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
// char* last_ptr;
// static void* extend_heap(size_t words);
// static void* coalesce(void *bp);
// static void* next_fit(size_t asize);
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

// /* 
//  * mm_malloc - Allocate a block by incrementing the brk pointer.
//  * Always allocate a block whose size is a multiple of the alignment.
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
//     if ((ptr = next_fit(asize)) != NULL){
//         place(ptr, asize);
//         return ptr;
//     }
//     extendsize = MAX(asize, CHUNKSIZE);
//     if((ptr = extend_heap(extendsize/WSIZE)) == NULL) return NULL;
//     place(ptr, asize);
//     return ptr;
// }

// /* 
//    적당한 크기의 가용블록 검색. 
//    이 때, next_fit 방법을 이용(최근에 할당된 블록을 기준으로 다음 블록 검색)
// */
// static void *next_fit(size_t asize) {
//     char *ptr = last_ptr;

//     for (ptr = NEXT_BLKP(ptr); GET_SIZE(HDRP(ptr)); ptr = NEXT_BLKP(ptr)) {
//         if (!GET_ALLOC(HDRP(ptr)) && GET_SIZE(HDRP(ptr)) >= asize) {
//             // last_ptr = ptr;
//             return ptr;
//         }
//     }
//     ptr = heap_listp;
//     for (ptr = NEXT_BLKP(ptr); GET_SIZE(HDRP(ptr)); ptr = NEXT_BLKP(ptr)) {
//         if (!GET_ALLOC(HDRP(ptr)) && GET_SIZE(HDRP(ptr)) >= asize) {
//             // last_ptr = ptr;
//             return ptr;
//         }
//     }
//     return NULL;
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
//         last_ptr = ptr;
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
//     last_ptr = ptr;
//     return ptr;
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
//  * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
//  */
// void *mm_realloc(void *bp, size_t size) {
//     size_t old_size = GET_SIZE(HDRP(bp));
//     size_t new_size = size + (DSIZE);   // 2*WISE는 헤더와 풋터

//     // new_size가 old_size보다 작거나 같으면 기존 bp 그대로 사용
//     if (new_size <= old_size) {
//         return bp;
//     }
//     // new_size가 old_size보다 크면 사이즈 변경
//     else {
//         size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
//         size_t current_size = old_size + GET_SIZE(HDRP(NEXT_BLKP(bp)));

//         // next block이 가용상태이고 old, next block의 사이즈 합이 new_size보다 크면 그냥 그거 바로 합쳐서 쓰기
//         if (!next_alloc && current_size >= new_size) {
//             PUT(HDRP(bp), PACK(current_size, 1));
//             PUT(FTRP(bp), PACK(current_size, 1));
//             return bp;
//         }
//         // 아니면 새로 block 만들어서 거기로 옮기기
//         else {
//             void *new_bp = mm_malloc(new_size);
//             memcpy(new_bp, bp, new_size);  // 메모리의 특정한 부분으로부터 얼마까지의 부분을 다른 메모리 영역으로 복사해주는 함수(old_bp로부터 new_size만큼의 문자를 new_bp로 복사해라!)
//             mm_free(bp);
//             return new_bp;
//         }
//     }
// }
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"
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
#define ALIGNMENT 8
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


// My additional Macros
#define WSIZE     4          // word and header/footer size (bytes)
#define DSIZE     8          // double word size (bytes)
#define INITCHUNKSIZE (1<<6)
#define CHUNKSIZE (1<<12)//+(1<<7) 

#define LISTLIMIT     20

#define MAX(x, y) ((x) > (y) ? (x) : (y)) 
#define MIN(x, y) ((x) < (y) ? (x) : (y)) 

// Pack a size and allocated bit into a word
#define PACK(size, alloc) ((size) | (alloc))

// Read and write a word at address p 
#define GET(p)            (*(unsigned int *)(p))
#define PUT(p, val)       (*(unsigned int *)(p) = (val))

// Store predecessor or successor pointer for free blocks 
#define SET_PTR(p, ptr) (*(unsigned int *)(p) = (unsigned int)(ptr))

// Read the size and allocation bit from address p 
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

// Address of block's header and footer 
#define HDRP(ptr) ((char *)(ptr) - WSIZE)
#define FTRP(ptr) ((char *)(ptr) + GET_SIZE(HDRP(ptr)) - DSIZE)

// Address of (physically) next and previous blocks 
#define NEXT_BLKP(ptr) ((char *)(ptr) + GET_SIZE((char *)(ptr) - WSIZE))
#define PREV_BLKP(ptr) ((char *)(ptr) - GET_SIZE((char *)(ptr) - DSIZE))

// Address of free block's predecessor and successor entries 
#define PRED_PTR(ptr) ((char *)(ptr))
#define SUCC_PTR(ptr) ((char *)(ptr) + WSIZE)

// Address of free block's predecessor and successor on the segregated list 
#define PRED(ptr) (*(char **)(ptr))
#define SUCC(ptr) (*(char **)(SUCC_PTR(ptr)))


// End of my additional macros


// Global var
void *segregated_free_lists[LISTLIMIT]; 

// Functions
static void *extend_heap(size_t size);
static void *coalesce(void *ptr);
static void *place(void *ptr, size_t asize);
static void insert_node(void *ptr, size_t size);
static void delete_node(void *ptr);

/*heap memory 생성*/
int mm_init(void){
    int list;
    char *heap_start;
    //segregated_free_lists를 NULL로 초기화
    for(list = 0; list<LISTLIMIT; list++){
        segregated_free_lists[list] = NULL;
    }
    if((long)(heap_start = mem_sbrk(4*WSIZE)) == -1){
        return -1;
    }

    PUT(heap_start, 0); /* Alignment padding*/
    PUT(heap_start+ (1*WSIZE), PACK(DSIZE, 1)); /*prologue header*/
    PUT(heap_start+ (2*WSIZE), PACK(DSIZE, 1)); /*prologue footer*/
    PUT(heap_start+ (3*WSIZE), 0); /*Epilogue header*/
    /* 1<<6만큼 extend */
    if(extend_heap(INITCHUNKSIZE) == NULL)
        return -1;
    return 0;
}

/*size만큼 heap memory를 할당*/
void *mm_malloc(size_t size){
    size_t asize;
    size_t extendsize;
    void *ptr = NULL;
    /* double word size에 맞게 size를 조정 */
    if(size == 0)
        return NULL;
    
    if(size < DSIZE){
        asize = 2*DSIZE;
    }else{
        asize = ALIGN(size) + DSIZE;
    }
    int list = 0;
    size_t searchsize = asize;
    // search for free block in segregated list
    while(list < LISTLIMIT){
        if ((list == LISTLIMIT - 1) || ((searchsize <= 1) && (segregated_free_lists[list] != NULL))){
            ptr = segregated_free_lists[list];
            /* 찾은 free_list 중에서 aszie보다 크기가 큰 것 중 가장 작은 것을 찾는다. */
            /* 이때 free list는 오름차순으로 정렬되어 있다.*/
            while((ptr != NULL) && ((asize > GET_SIZE(HDRP(ptr))))){
                ptr = PRED(ptr);
            }
            if(ptr != NULL)
                break;
        }
        searchsize = searchsize >> 1;
        list++;
    }
    /*만약 asize보다 큰 free가 존재하지 않는다면 새롭게 extend*/
    if(ptr == NULL){
        extendsize = MAX(asize, CHUNKSIZE);

        if ((ptr = extend_heap(extendsize)) == NULL)
            return NULL;
    }
    ptr = place(ptr, asize);
    return ptr;
}

/*ptr에 할당된 data를 free*/
void mm_free(void *ptr){
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    insert_node(ptr, size);
    coalesce(ptr);
}


void *mm_realloc(void *ptr, size_t size){
    void *new_ptr = ptr;
    int extendsize = 0;
    int remainder;
    size_t new_size = size;
    size_t next_size = GET_SIZE(HDRP(NEXT_BLKP(ptr)));
    /*size를 double word에 맞게 조정*/
    if(size == 0)
        return NULL;
    if(new_size <= DSIZE){
        new_size = 2*DSIZE;
    }else{
        new_size = ALIGN(size + DSIZE);
    }
    /*뒤에 있는 힙이 free이고 현재 size와 free size의 합이 new_size보다 클 경우*/
    if(!GET_ALLOC(HDRP(NEXT_BLKP(ptr))) && new_size <= next_size + GET_SIZE(HDRP(ptr))){
        PUT(HDRP(ptr), PACK(next_size + GET_SIZE(HDRP(ptr)), 0));
        PUT(FTRP(ptr), PACK(GET_SIZE(HDRP(ptr)), 0));
        delete_node(NEXT_BLKP(ptr));
        place(ptr, new_size);
    /*데이터의 위치가 heap의 최상단일 경우*/
    }else if(!GET_SIZE(HDRP(NEXT_BLKP(ptr)))){
        remainder = GET_SIZE(HDRP(ptr))- new_size;
        if(remainder < 0){
            extendsize = MAX(-remainder, 1<<5);
            if (extend_heap(extendsize) == NULL){
                return NULL;
            }
        }
        remainder += extendsize;
        delete_node(NEXT_BLKP(ptr));
        // Do not split block
        PUT(HDRP(ptr), PACK(new_size + remainder, 1)); 
        PUT(FTRP(ptr), PACK(new_size + remainder, 1));
         
    } 
    else{
        new_ptr = mm_malloc(new_size - DSIZE);
        memcpy(new_ptr, ptr, MIN(size, new_size));
        mm_free(ptr);
    }
    return new_ptr;
}


/*===========================================*/
static void* extend_heap(size_t size){
    void* ptr;
    size_t asize;
    asize = ALIGN(size);

    if((ptr = mem_sbrk(asize)) == (void *)-1)
        return NULL;
    //header와 footer 생성
    PUT(HDRP(ptr), PACK(asize, 0));
    PUT(FTRP(ptr), PACK(asize, 0));
    PUT(HDRP(NEXT_BLKP(ptr)), PACK(0, 1));
    insert_node(ptr, asize);

    return coalesce(ptr);
}
/*새롭게 형성된 free 노드를 free_list에 넣는 과정*/
/*각 free_list는 오름차순으로 정렬*/
static void insert_node(void *ptr, size_t size){
    int list = 0;
    void *search_ptr;
    void *before_ptr = NULL;
    //사이즈에 맞는 list idx 선택
    while((list < LISTLIMIT-1) && (size > 1)){
        size >>= 1;
        list++;
    }
    // 오름차수로 정렬된 ptr을 움직이며 search
    search_ptr = segregated_free_lists[list];
    while((search_ptr != NULL) && (size > GET_SIZE(HDRP(search_ptr)))){
        before_ptr = search_ptr;
        search_ptr= PRED(search_ptr);
    }
    // Set predecessor and successor
    if (search_ptr != NULL){
        if (before_ptr != NULL){
            SET_PTR(PRED_PTR(ptr), search_ptr);
            SET_PTR(SUCC_PTR(search_ptr), ptr);
            SET_PTR(SUCC_PTR(ptr), before_ptr);
            SET_PTR(PRED_PTR(before_ptr), ptr);
        } else{
            SET_PTR(PRED_PTR(ptr), search_ptr);
            SET_PTR(SUCC_PTR(search_ptr), ptr);
            SET_PTR(SUCC_PTR(ptr), NULL);
            segregated_free_lists[list] = ptr;
        }
    }else{
        if (before_ptr != NULL){
            SET_PTR(PRED_PTR(ptr), NULL);
            SET_PTR(SUCC_PTR(ptr), before_ptr);
            SET_PTR(PRED_PTR(before_ptr), ptr);
        } else{
            SET_PTR(PRED_PTR(ptr), NULL); // SET_PTR(p, ptr) (*(unsigned int *)(p) = (unsigned int)(ptr))
            SET_PTR(SUCC_PTR(ptr), NULL);
            segregated_free_lists[list] = ptr;
        }
    }
}
static void delete_node(void *ptr){
    int list = 0;
    size_t size = GET_SIZE(HDRP(ptr));
    while((list < LISTLIMIT -1) && (size > 1)){
        size >>= 1;
        list++;
    }
    if (PRED(ptr) != NULL) {
        if (SUCC(ptr) != NULL) {
            SET_PTR(SUCC_PTR(PRED(ptr)), SUCC(ptr));
            SET_PTR(PRED_PTR(SUCC(ptr)), PRED(ptr));
        } else {
            SET_PTR(SUCC_PTR(PRED(ptr)), NULL);
            segregated_free_lists[list] = PRED(ptr);
        }
    } else {
        if (SUCC(ptr) != NULL) {
            SET_PTR(PRED_PTR(SUCC(ptr)), NULL);
        } else {
            segregated_free_lists[list] = NULL;
        }
    }
    return;
}


static void *coalesce(void *ptr){
    size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(ptr)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
    size_t size = GET_SIZE(HDRP(ptr));
    // Do not coalesce with previous block if the previous block is tagged with Reallocation tag
    if(prev_alloc && next_alloc){
        return ptr;
    }
    else if(prev_alloc && !next_alloc){
        delete_node(ptr);
        delete_node(NEXT_BLKP(ptr));
        size += GET_SIZE(HDRP(NEXT_BLKP(ptr)));
        PUT(HDRP(ptr), PACK(size, 0));
        PUT(FTRP(ptr), PACK(size, 0));
    }
    else if(!prev_alloc && next_alloc){
        delete_node(ptr);
        delete_node(PREV_BLKP(ptr));
        size += GET_SIZE(HDRP(PREV_BLKP(ptr)));
        PUT(FTRP(ptr), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
        ptr = PREV_BLKP(ptr);
    }
    else {
        delete_node(ptr);
        delete_node(PREV_BLKP(ptr));
        delete_node(NEXT_BLKP(ptr));
        size += GET_SIZE(HDRP(PREV_BLKP(ptr))) + GET_SIZE(HDRP(NEXT_BLKP(ptr)));
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(size, 0));
        ptr = PREV_BLKP(ptr);
    }
    insert_node(ptr, size);
    return ptr;
}

static void *place(void *ptr, size_t asize){
    size_t ptr_size = GET_SIZE(HDRP(ptr));
    size_t remainder = ptr_size - asize;

    delete_node(ptr);
    
    if(remainder <= DSIZE*2){
        PUT(HDRP(ptr), PACK(ptr_size, 1));
        PUT(FTRP(ptr), PACK(ptr_size, 1));
    }
    else if(asize >= 96){
        PUT(HDRP(ptr), PACK(remainder, 0));
        PUT(FTRP(ptr), PACK(remainder, 0));
        PUT(HDRP(NEXT_BLKP(ptr)), PACK(asize, 1));
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(asize, 1));
        insert_node(ptr, remainder);
        return NEXT_BLKP(ptr);
    }
    else{
        PUT(HDRP(ptr), PACK(asize, 1));
        PUT(FTRP(ptr), PACK(asize, 1));
        PUT(HDRP(NEXT_BLKP(ptr)), PACK(remainder, 0));
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(remainder, 0));
        insert_node(NEXT_BLKP(ptr), remainder);
    }
    return ptr;
}