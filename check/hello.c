int mm_init(void)
{
    int list;         
    char *heap_start; // Pointer to beginning of heap
    
    // Initialize segregated free lists
    for (list = 0; list < LISTLIMIT; list++) {
        segregated_free_lists[list] = NULL;
    }
    
    // Allocate memory for the initial empty heap 
    if ((long)(heap_start = mem_sbrk(4 * WSIZE)) == -1)
        return -1;
    
    PUT_NOTAG(heap_start, 0);                            /* Alignment padding */
    PUT_NOTAG(heap_start + (1 * WSIZE), PACK(DSIZE, 1)); /* Prologue header */
    PUT_NOTAG(heap_start + (2 * WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
    PUT_NOTAG(heap_start + (3 * WSIZE), PACK(0, 1));     /* Epilogue header */
    
    if (extend_heap(INITCHUNKSIZE) == NULL)
        return -1;
    
    return 0;
}

void *mm_malloc(size_t size)
{
    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    void *ptr = NULL;  /* Pointer */
    
    // Ignore size 0 cases
    if (size == 0)
        return NULL;
    
    // Align block size
    if (size <= DSIZE) {
        asize = 2 * DSIZE;
    } else {
        asize = ALIGN(size+DSIZE);
    }
    
    int list = 0; 
    size_t searchsize = asize;
    // Search for free block in segregated list
    while (list < LISTLIMIT) {
        if ((list == LISTLIMIT - 1) || ((searchsize <= 1) && (segregated_free_lists[list] != NULL))) {
            ptr = segregated_free_lists[list];
            // Ignore blocks that are too small or marked with the reallocation bit
            while ((ptr != NULL) && ((asize > GET_SIZE(HDRP(ptr))) || (GET_TAG(HDRP(ptr)))))
            {
                ptr = PRED(ptr);
            }
            if (ptr != NULL)
                break;
        }
        
        searchsize >>= 1;
        list++;
    }
    
    // if free block is not found, extend the heap
    if (ptr == NULL) {
        extendsize = MAX(asize, CHUNKSIZE);
        
        if ((ptr = extend_heap(extendsize)) == NULL)
            return NULL;
    }
    
    // Place and divide block
    ptr = place(ptr, asize);
    
    
    // Return pointer to newly allocated block 
    return ptr;
}

/*
 * mm_free - Freeing a block does nothing.
 *
 * Role : The mm_free routine frees the block pointed to by ptr
 *
 * Return value : returns nothing
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
 
    REMOVE_RATAG(HDRP(NEXT_BLKP(ptr)));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    
    insert_node(ptr, size);
    coalesce(ptr);
    
    return;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 *
 * Role : The mm_realloc routine returns a pointer to an allocated 
 *        region of at least size bytes with constraints.
 *
 *  I used https://github.com/htian/malloc-lab/blob/master/mm.c source idea to maximize utilization
 *  by using reallocation tags
 *  in reallocation cases (realloc-bal.rep, realloc2-bal.rep)
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *new_ptr = ptr;    /* Pointer to be returned */
    size_t new_size = size; /* Size of new block */
    int remainder;          /* Adequacy of block sizes */
    int extendsize;         /* Size of heap extension */
    int block_buffer;       /* Size of block buffer */
    
    // Ignore size 0 cases
    if (size == 0)
        return NULL;
    
    // Align block size
    if (new_size <= DSIZE) {
        new_size = 2 * DSIZE;
    } else {
        new_size = ALIGN(size+DSIZE);
    }
    
    /* Add overhead requirements to block size */
    new_size += REALLOC_BUFFER;
    
    /* Calculate block buffer */
    block_buffer = GET_SIZE(HDRP(ptr)) - new_size;
    
    /* Allocate more space if overhead falls below the minimum */
    if (block_buffer < 0) {
        /* Check if next block is a free block or the epilogue block */
        if (!GET_ALLOC(HDRP(NEXT_BLKP(ptr))) || !GET_SIZE(HDRP(NEXT_BLKP(ptr)))) {
            remainder = GET_SIZE(HDRP(ptr)) + GET_SIZE(HDRP(NEXT_BLKP(ptr))) - new_size;
            if (remainder < 0) {
                extendsize = MAX(-remainder, CHUNKSIZE);
                if (extend_heap(extendsize) == NULL)
                    return NULL;
                remainder += extendsize;
            }
            
            delete_node(NEXT_BLKP(ptr));
            
            // Do not split block
            PUT_NOTAG(HDRP(ptr), PACK(new_size + remainder, 1)); 
            PUT_NOTAG(FTRP(ptr), PACK(new_size + remainder, 1)); 
        } else {
            new_ptr = mm_malloc(new_size - DSIZE);
            memcpy(new_ptr, ptr, MIN(size, new_size));
            mm_free(ptr);
        }
        block_buffer = GET_SIZE(HDRP(new_ptr)) - new_size;
    }
    
    // Tag the next block if block overhead drops below twice the overhead 
    if (block_buffer < 2 * REALLOC_BUFFER)
        SET_RATAG(HDRP(NEXT_BLKP(new_ptr)));
    
    // Return the reallocated block 
    return new_ptr;
}
