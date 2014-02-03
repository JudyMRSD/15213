/*
 * runyunz - Runyun Zhang
 * mm.c
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 *
 * Version: Segregated Free List + First Fit
 * Update: LEVEL; MINILEV; free_listp[LEVEL]; int free_level(size) 
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mm.h"
#include "memlib.h"

/* If you want debugging output, use the following macro.  When you hand
 * in, remove the #define DEBUG line. */
//#define DEBUG
#ifdef DEBUG
# define dbg_printf(...) printf(__VA_ARGS__)
#else
# define dbg_printf(...)
#endif

/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif /* def DRIVER */

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(p) (((size_t)(p) + (ALIGNMENT-1)) & ~0x7)

/*****************************macros start********************************/
/* Set Macros */ 

/* Basic constants and macros */
#define WSIZE       4     
#define DSIZE       8 
#define CHUNKSIZE  (1<<8)    /* Default size for expanding the heap */ 

#define OVERHEAD   WSIZE * 2  /* OVERHEAD: head + foot */
#define MINIBLK    DSIZE * 2 + OVERHEAD 

#define LEVEL      10        /* number of seg list */     
#define MINILEV   (1<<4)    /* minimum of seg list */

/* Return a larger one between x and y */
#define MAX(x, y) ((x) > (y)? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))           
#define PUT(p, val)  (*(unsigned int *)(p) = (val))    

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)                   

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((char *)(bp) - WSIZE)
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Given block ptr bp, compute address of pred and succ free blocks */
#define SUC(bp)(*(char **)(bp + DSIZE))
#define PRD(bp)(*(char **)(bp))

/*****************************macros end**********************************/


/****************global variables and helper functions start**************/
static char *heap_listp = NULL;  /* Pointer to  heap */  
static char *free_header = NULL;  /* Pointer to free list */

static char* free_listp[LEVEL];

inline static int in_heap(const void *p);
inline static int aligned(const void *p);

static void *extend_heap(size_t words);
static void *find_fit(size_t asize);
static void place(void *ptr, size_t asize);

static void *coalesce(void *ptr);
static void insert_freelst(void *bp, int level);
static void remove_freelst(void *bp, int level);
static void set_mark(void *ptr, size_t size, int opt);

static int check_heap(int verbose);
static int check_list(int verbose);
static void check_block(void *ptr);
static void print_block(void *ptr);
static void check_entry(void *ptr);
static void print_entry(void *ptr);

static int free_level(size_t size);
/****************global variables and helper functions end**************/

/*
 * mm_init - Initialize the allocator,
 *           #1: unused padding(4-byte), aligned to a double-word boundary
 *           #2: Prologue block(8-byte), created during init and never freed
 *           #3: Epilogue block(4-byte), consists of only a header
 *           regular blocks: allocated between prologue and epilogue, 
 *                           created by calls to malloc or free
 *           call: set_mark(); extend_heap() 
 */
int mm_init(void) 
{
    char* bp = NULL;
    int i = 0;
    /* Create the initial empty heap */
	if ((void *)-1 == (heap_listp = mem_sbrk(4 * DSIZE)))
	{
		return -1;
	}

    PUT(heap_listp, 0);                          /* Alignment padding */                                
    bp = heap_listp + DSIZE;                     /* Point to prologue bp */                    
    
    set_mark(bp, MINIBLK, 1);                    /* Init prologue block */                          
    PUT(NEXT_BLKP(bp), PACK(0, 1));              /* Init epilogue header */       
    
    free_header = bp;
    for(; i < LEVEL; ++i)
    {
    	free_listp[i] = bp;
    }

    /* Extend the heap */
    if (!extend_heap(CHUNKSIZE/WSIZE)) 
	{
		return -1;
	}

    return 0;
}


/*
 * malloc -  Allocate a block of size bytes of memory,
 *           block size = DSIZE * n + OVERHEAD,
 *           call: find_fit(); extend_heap(); place()
 *           
 */
void *malloc (size_t size) 
{
	size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp = NULL;  

    if (!size)
    {
		return NULL;
	}

	if (!heap_listp)
	{
		mm_init();
    }

    /* Adjust block size to include overhead and alignment reqs. */
	asize = MAX(MINIBLK, ALIGN(size)+ OVERHEAD);

	/* Search the free list for a fit */
	if ((bp = find_fit(asize))) 
	{
		place(bp, asize);
		return bp;
	}
	else
	{
		extendsize = MAX(asize, CHUNKSIZE);
		if (!(bp = extend_heap(extendsize/WSIZE)))
		{
			return NULL;
		}
	
		place(bp, asize);
		return bp;
	}
}

/*
 * free - Free requested block,
 *        merges adjacent free blocks using the boundary-tags coalescing
 *        called: realloc()
 *        call: coalesce(); set_mark()
 */
void free(void *ptr) 
{
	char* bp = (char*) ptr;
    size_t size = 0;

    if(!bp)
    { 
		return;
	}

	if (!heap_listp)
    {
		mm_init();
    }

    size = GET_SIZE(HDRP(bp));
    set_mark(bp, size, 0);
    coalesce(bp);

    return;
}

/*
 * realloc - Change the size of the block by mallocing a new block,
 *           copying its data, and freeing the old block.
 */
void *realloc(void *ptr, size_t size) 
{
	  char *bp = (char*) ptr;
	  char *newbp = NULL;
	  size_t oldsize = 0;
	  
	  if(!size)
	  {
	    free(bp);
	    return 0;
	  }

	  if(!bp)
	  {
	    return malloc(size);
	  }

	  if(!(newbp = malloc(size)))
	  {
	    return 0;
	  }

	  oldsize = GET_SIZE(HDRP(bp));
	  if(size < oldsize)
	  {
	  	oldsize = size;
	  }
	  memcpy(newbp, bp, oldsize);

	  free(bp);
	  return newbp;
}

/*
 * calloc - you may want to look at mm-naive.c
 * This function is not tested by mdriver, but it is
 * needed to run the traces.
 */
void *calloc (size_t nmemb, size_t size)
{
	size_t bytes = nmemb * size;
	void *newbp = NULL;

	newbp = malloc(bytes);
	memset(newbp, 0, bytes);

	return newbp;
}

/*
 * mm_checkheap - Check every block in heap;
 *                check every entry in free list;
 *                compare counts of free bytes
 *                call: check_heap(); check_list()
 */
void mm_checkheap(int verbose)
{
	int hcount = check_heap(verbose);
	int lcount = check_list(verbose);
	
    if(hcount != lcount)
    {
    	dbg_printf("Error[mm_checkheap]: \
    		Free block size does not match\n");
    }

    return;
}

/******************************helper functions************************************/
/* 
 * extend_heap -  Extends the heap by CHUNKSIZE bytes,
 * 				  and creates the initial free block;
 *                called: mm_init(); malloc()
 *                call: coalesce(); set_mark()              
 */
static void *extend_heap(size_t words) {
    char *bp = NULL;
    size_t size = 0;

    /* 
     * Allocate an even number of words to maintain alignment,
     * rounds up requested size to nearest multiple of dword (8 bytes)
     */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    
    if (size < MINIBLK)
    {
		size = MINIBLK;
	}

    /*
     * Calls to mem_sbrk returns a dword aligned chunk ofmemory 
     * following the header of the epilogue block.
     */
    if ((long)(bp = mem_sbrk(size)) == -1) 
    { 
		return NULL;                                        
	}

    /* Initialize free block header/footer and the epilogue header */
    set_mark(bp, size, 0);
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/*
 * coalesce - Boundary tag coalescing;
 *            return ptr to coalesced block.
 *            called by: extend_heap(), free()
 *            call: remove_freelst(), insert_freelst(), set_mark() 
 *            constraint: GET_ALLOCED = 0  								
 */
static void *coalesce(void *ptr) 
{
	char* bp = (char*) ptr;
	int level = 0;
	int fsize = 0;

	size_t prev_alloc = (GET_ALLOC(FTRP(PREV_BLKP(bp))) || 
							PREV_BLKP(bp) == bp);
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	size_t size = GET_SIZE(HDRP(bp));

	/* Case 1, extend the block leftward */
	if (prev_alloc == 1 && next_alloc == 0) 
	{			
		fsize = GET_SIZE(HDRP(NEXT_BLKP(bp)));
		level = free_level(fsize);
		size += fsize;
		
		remove_freelst(NEXT_BLKP(bp), level);
		set_mark(bp, size, 0);
	}
	/* Case 2, extend the block rightward */
	else if (prev_alloc == 0 && next_alloc == 1) 
	{		
		fsize = GET_SIZE(HDRP(PREV_BLKP(bp)));
		level = free_level(fsize);
		size += fsize;
		bp = PREV_BLKP(bp);

		remove_freelst(bp, level);
		set_mark(bp, size, 0);		
	}
	/* Case 3, extend the block in both directions */
	else if (prev_alloc == 0 && next_alloc == 0) 
	{		
		fsize = GET_SIZE(HDRP(PREV_BLKP(bp)));
		level = free_level(fsize);
		size += fsize;
		remove_freelst(PREV_BLKP(bp), level);

		fsize = GET_SIZE(HDRP(NEXT_BLKP(bp)));
		level = free_level(fsize);
		size += fsize;
		remove_freelst(NEXT_BLKP(bp), level);

		bp = PREV_BLKP(bp);
		set_mark(bp, size, 0);
	}

	/* add into free list after coalescing */
	level = free_level(size);
	insert_freelst(bp, level);

	return bp;
}

/* 
 * place - Place block of asize bytes at start of free block bp,
 *         split if remainder would be at least minimum block size
 *         called by: malloc()
 *         call: remove_freelst(), insert_freelst(), set_mark()
 */
static void place(void *ptr, size_t asize) 
{
    char* bp = (char*) ptr;
    int level = 0;

    size_t csize = GET_SIZE(HDRP(bp));
    size_t dif = csize - asize;

	level = free_level(csize);
	remove_freelst(bp, level);

    if (dif >= MINIBLK)
    { 
    	/* remainder affords a split */
		set_mark(bp, asize, 1);
		bp = NEXT_BLKP(bp);
		set_mark(bp, dif, 0);
		coalesce(bp);
	}
    else
    { 
		/* no split happens*/
		set_mark(bp, csize, 1);
    }

    return;
}

/*
 * set_mark - Set header and footer
 *            called: extend_heap(), free(), 
 *                    place(), coalesce(),remove_lst() 								
 */
static void set_mark(void *ptr, size_t size, int opt) 
{
	char* bp = (char*) ptr;

    PUT(HDRP(bp), PACK(size, opt));
	PUT(FTRP(bp), PACK(size, opt));

    return;
}

/* 
 * find_fit - Find a fit for a block with asize bytes
 *            called: malloc()
 */
static void *find_fit(size_t asize) 
{
	char* bp = NULL;
	int level = 0;

	level = free_level(asize);

	for(; level < LEVEL; ++level)
	{
		bp = free_listp[level];

		while(!GET_ALLOC(HDRP(bp)))
		{
			if(asize <= GET_SIZE(HDRP(bp)))
			{
				return bp;
			}

			bp = PRD(bp);
		}
	}	

	return NULL;
}

/* 
 * insert_freelst - Insert into free list using LIFO,
 *                  called: place(), coalesce()
 */
static void insert_freelst(void *ptr, int level) 
{
	char* bp = (char*) ptr;

	SUC(free_listp[level]) = bp;      /* set successor for bp's predecessor */
	PRD(bp) = free_listp[level];      /* set bp's predecessor */
	SUC(bp) = NULL;                   /* set bp's successor */
	free_listp[level] = bp;           /* set free_header */

	return;
}

/*
 * remove_freelst - Deal with free list, nothing to do with hd or ft
 *                  called: coalesce()
 */
static void remove_freelst(void *ptr, int level) {
	char* bp = (char*) ptr;
	//check_entry(bp);
	
	char* suc = SUC(bp);
	char* prd = PRD(bp);
	
	if (suc)
	{
		PRD(suc) = prd;
	}
	else
	{
		free_listp[level] = prd; 
	}

	SUC(prd) = suc;

	return;
}

/*
 *  check_heap - check every blocks in heap
 *               call: check_block()
 *               called: mm_checkheap()
 */
static int check_heap(int verbose){
	int count = 0;
	char* bp = heap_listp;

	if (verbose)
	{
		dbg_printf("Heap (%p):\n", bp);
	}

	/* check prologue header */
	if ((GET_SIZE(HDRP(bp)) != (4 * DSIZE)) || !GET_ALLOC(HDRP(bp)))
	{
		dbg_printf("Error[check_heap]: Block %p \
			- Bad prologue header\n", bp);
		dbg_printf("Prologue Header = %d; ALLOC = %d\n",
			(int)GET_SIZE(HDRP(bp)), (int)GET_ALLOC(HDRP(bp))); 	
	}

	/* check blocks */
	while (GET_SIZE(HDRP(bp)) > 0)
	{
		check_block(bp);

		if (verbose) 
	    {
	    	print_block(bp);
	    }
		
		if (!GET_ALLOC(HDRP(bp)))
		{
			count += GET_SIZE(HDRP(bp));
		} 

		bp = NEXT_BLKP(bp);
	}

	/* check epilogue header */
	if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp))))
    {
		dbg_printf("Error[check_heap]: Block %p \
			- Bad epilogue header\n", bp);
	}
	if (verbose)
	{
		print_block(bp);
    }

	return count;
}

/*
 *  check_list - check every entry in free list
 *               call: check_entry()
 *               called: mm_checkheap()
 */
static int check_list(int verbose) {
	int count = 0;
	int level = 0;
    char* bp = NULL;
    


	for (; level < LEVEL; ++level)
	{
		bp = free_listp[level];
		
		if (verbose)
		{
			dbg_printf("Level %d Segregated List: (%p)\n", 
				level, bp);
		}

		/* check entries */
    	while(!GET_ALLOC(HDRP(bp)))
    	{
	    	check_entry(bp);
	    	if (verbose) 
		    {
		    	print_entry(bp);
		    }
			count += GET_SIZE(HDRP(bp));
	    	bp = PRD(bp);
    	}
	}


    return count;
}

/*
 * check_block - Check block status:
 *               heap boundaries, address alignment, 
 *               header and footer consistency, coalescing, etc.
 *               call: check_heap()                              
 */
static void check_block(void *ptr) {
	char* bp = (char*) ptr;

	/* check boundaries */
	if (!in_heap(bp))
	{
		dbg_printf("Error[check_block]: Block %p \
			- exceeds heap boundaries\n", bp);
	}

	/* check address alignment */
	if (!aligned(bp))
	{
		dbg_printf("Error[check_block]: Block %p \
			- not doubleword aligned\n", bp);
	}

	/* check header and footer consistency */
    if (GET(HDRP(bp)) != GET(FTRP(bp)))
	{
		dbg_printf("Error[check_block]: Block %p \
			- header does not match footer\n", bp);
	}

	/* check minimum size*/
	if (GET_SIZE(HDRP(bp)) < MINIBLK)
	{
		dbg_printf("Error[check_block]: Block %p \
			- too small for allocation\n", bp);
	}

	/* check coalescing */
	size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	size_t alloc = GET_ALLOC(HDRP(bp));
	
	if (!prev_alloc && !alloc)
	{
		dbg_printf("Error[check_block]: Block %p \
			- violates coalescing\n", bp);
		dbg_printf("Error[check_block]: Prev Block %p \
			is not alloced\n",PREV_BLKP(bp));
	}
	
	if (!next_alloc && !alloc)
	{
		dbg_printf("Error[check_block]: Block %p \
			- violates coalescing\n", bp);
		dbg_printf("Error[check_block]: Next Block \
			is not alloced%p\n", NEXT_BLKP(bp));
	}

	return;
}

/*
 * check_block - Check block status:
 *               allocation status, 
 *               pred and succ consistency, etc.
 *               call: check_list()                                         
 */
static void check_entry(void* ptr)
{
	char* bp = (char*)ptr;

	char* suc = NULL;
	char* prd = NULL;

	/* check free */
	if (GET_ALLOC(HDRP(bp)))
	{
		dbg_printf("Error[check_entry]: Block %p \
			- still allocated\n", bp);
	}

	/* check consistency */
	if ((suc = SUC(bp)))
	{
		if(PRD(suc) != bp)
		{
			dbg_printf("Error[check_entry]: Block %p \
				- consistency check failed\n", bp);
			dbg_printf("successor: %p; its prd: %p\n", suc, PRD(suc));
		}
	}
	if ((prd = PRD(bp)))
	{
		if (SUC(prd) != bp)
		{
			dbg_printf("Error[check_entry]: Block %p \
				- consistency check failed\n", bp);
			dbg_printf("predecessor: %p; its suc: %p\n", prd, SUC(prd));
		}
	}

	/* check seg list size */

	return;
}

/*
 * print_block - print block info
 *               call: check_heap()
 */
static void print_block(void *ptr) 
{
    char* bp = (char*) ptr;

    if (!GET_SIZE(HDRP(bp)))
    {
		dbg_printf("Block %p: EOL\n", bp);
		return;
    }

    dbg_printf("Block %p: Head = %u|%u; Foot = %u|%u\n",
    	bp, GET_SIZE(HDRP(ptr)), GET_ALLOC(HDRP(ptr)),
    	GET_SIZE(FTRP(ptr)),GET_ALLOC(FTRP(ptr)));

	return;
}

/*
 * print_entry - print block info
 *               call: check_heap()
 */
static void print_entry(void *ptr) 
{
    char* bp = (char*) ptr;

    dbg_printf("Block %p: [predecessor: %p successor: %p]\n", 
    	bp, PRD(bp), SUC(bp));

    if (free_header == bp)
    {
		dbg_printf("Free List EOL: %p\n", bp);
    }

	return;
}

/*
 * free_level - get seg list level
 *              called: coalesce(); place(); find_fit()
 */
static int free_level(size_t size)
{
	int i = 0;
    for(; i < LEVEL; ++i)
	{
		if(size < MINILEV)
		{
			break;
		}

		size = (size >> 1);
	}

	if(i == LEVEL)
	{
		i--;
	}

	return i;
}

/*
 * in_heap - Return whether the pointer is in the heap.
 *           May be useful for debugging.
 */
static int in_heap(const void *p) {
    return p <= mem_heap_hi() && p >= mem_heap_lo();
}

/*
 * aligned - Return whether the pointer is aligned.
 *           May be useful for debugging.
 */
static int aligned(const void *p) {
    return (size_t)ALIGN(p) == (size_t)p;
}



