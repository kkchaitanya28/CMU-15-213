/*
 * mm.c
 *
 * Ram Verma, andreid: ramv
 *
 * The code adapted the implicit list given on the CS:APP website.
 *
 * This code has 32 global variables (start_of_dllx) 
 * for the 32 buckets for the allocator, where x is the bucket number.
 *
 * Each bucket has its own size group (2^(x-1))to((2^x)-1) which is 
 * done as a explicit list.
 *
 * Each free block in the explicit list has 2 pointers to point to the
 * previous and next blocks.
 *
 * The allocations(add/delete) are done using a global int variable (count) 
 * to find the appropriate bucket.
 *
 * I added the free blocks to the appropriate list such that the list is in
 * ascending order.
 *
 * To find an appropriate free block, I used first-fit but stopped after 3
 * and go onto the next bigger sized bucket. 
 */
 
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mm.h"
#include "memlib.h"

//#define DEBUG /* Took out the debug flag */
#ifdef DEBUG
# define dbg_printf(...) printf(__VA_ARGS__); fflush(stdout)
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


/* $begin mallocmacros */
/* Basic constants and macros */
#define WSIZE       4       /* Word and header/footer size (bytes) */ 
#define DSIZE       8       /* Doubleword size (bytes) */
#define CHUNKSIZE  (1<<9)   /* Extend heap by this amount (bytes) */  
#define min_size    24      /* This is the minimum required size for free block */ 

#define MAX(x, y) ((x) > (y)? (x) : (y))  

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc)) 

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))            
#define PUT(p, val)  (*(unsigned int *)(p) = (val))    

/* Put ptr address at the address p */
#define PUT_ADDRESS(p, ptr)  (*((unsigned long *)(p)) = ((unsigned long)(ptr)))   

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)                   
#define GET_ALLOC(p) (GET(p) & 0x1)                    

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((char *)(bp) - WSIZE)                      
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of its pred and succ in linked list of free blocks */
#define PDRP(bp)    ((unsigned long *)(bp)) 
#define SCRP(bp)    (((unsigned long *)(bp)) + 1)

/* To traverse list of free blocks */
#define GET_NEXT(bp)  ((unsigned long *)(*(SCRP(bp))))
#define GET_PREV(bp)  ((unsigned long *)(*(PDRP(bp))))

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))
/* $end mallocmacros */

/* Global variables */
static char *heap_listp = 0;  /* Pointer to first block */  

/* The 32 segregated lists */
unsigned long *start_of_dll1 = NULL;   
unsigned long *start_of_dll2 = NULL;  
unsigned long *start_of_dll3 = NULL;   
unsigned long *start_of_dll4 = NULL;
unsigned long *start_of_dll5 = NULL;
unsigned long *start_of_dll6 = NULL;
unsigned long *start_of_dll7 = NULL;
unsigned long *start_of_dll8 = NULL;
unsigned long *start_of_dll9 = NULL;
unsigned long *start_of_dll10 = NULL;
unsigned long *start_of_dll11 = NULL;
unsigned long *start_of_dll12 = NULL;
unsigned long *start_of_dll13 = NULL;
unsigned long *start_of_dll14 = NULL;
unsigned long *start_of_dll15 = NULL;
unsigned long *start_of_dll16 = NULL;
unsigned long *start_of_dll17 = NULL;
unsigned long *start_of_dll18 = NULL;
unsigned long *start_of_dll19 = NULL;
unsigned long *start_of_dll20 = NULL;
unsigned long *start_of_dll21 = NULL;
unsigned long *start_of_dll22 = NULL;
unsigned long *start_of_dll23 = NULL;
unsigned long *start_of_dll24 = NULL;
unsigned long *start_of_dll25 = NULL;
unsigned long *start_of_dll26 = NULL;
unsigned long *start_of_dll27 = NULL;
unsigned long *start_of_dll28 = NULL;
unsigned long *start_of_dll29 = NULL;
unsigned long *start_of_dll30 = NULL;
unsigned long *start_of_dll31 = NULL;
unsigned long *start_of_dll32 = NULL;
/* Count is the index as to which bucket it is, given by function 'dealing_with_lists */
int count=0;


/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
/* The third argument tells whether the block bp exists in the bucket list */
static void place(void *bp, size_t asize, int d_number);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void printblock(void *bp); 
static void add_node(char *ptr);
static void delete_node(char *ptr);
unsigned long *dealing_with_lists(int counter, size_t size, unsigned long *ptr, int option);

/* Given the count (index of appropriate bucket), returns the next non-empty bucket */
unsigned long *next_list(int count);


/* 
 * mm_init - Initialize the memory manager
 * Initialize: return -1 on error, 0 on success.
 * Initialize all global variables 
 */
int mm_init(void) 
{  
  /* Create the initial empty heap */
  if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
  {
    return -1;
  }
  
  /* Initialise the 32 buckets */
  start_of_dll1 = NULL;   
  start_of_dll2 = NULL;  
  start_of_dll3 = NULL;   
  start_of_dll4 = NULL;
  start_of_dll5 = NULL;
  start_of_dll6 = NULL;
  start_of_dll7 = NULL;
  start_of_dll8 = NULL;
  start_of_dll9 = NULL;
  start_of_dll10 = NULL;
  start_of_dll11 = NULL;
  start_of_dll12 = NULL;
  start_of_dll13 = NULL;
  start_of_dll14 = NULL;
  start_of_dll15 = NULL;
  start_of_dll16 = NULL;
  start_of_dll17 = NULL;
  start_of_dll18 = NULL;
  start_of_dll19 = NULL;
  start_of_dll20 = NULL;
  start_of_dll21 = NULL;
  start_of_dll22 = NULL;
  start_of_dll23 = NULL;
  start_of_dll24 = NULL;
  start_of_dll25 = NULL;
  start_of_dll26 = NULL;
  start_of_dll27 = NULL;
  start_of_dll28 = NULL;
  start_of_dll29 = NULL;
  start_of_dll30 = NULL;
  start_of_dll31 = NULL;
  start_of_dll32 = NULL;
  /* Initialise the index of the buckets */
  count=0;
  
  /* Pointer used to pass in argument to coalesce if extend_heap returns non-null */
  unsigned long *tem;
  
  /* Gotten from CS:APP */
  PUT(heap_listp, 0);                          /* Alignment padding */
  PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1)); /* Prologue header */ 
  PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1)); /* Prologue footer */  
  PUT(heap_listp + (3*WSIZE), PACK(0, 1));     /* Epilogue header */
  heap_listp += (2*WSIZE);                      

  
  /* Extend the empty heap with a free block of CHUNKSIZE/WSIZE bytes */
  if ((tem=extend_heap(CHUNKSIZE/WSIZE)) == NULL) 
  {
    return -1;
  } 
  /* Extending the heap was successful, just coalesce it(which adds it to bucket) */
  coalesce(tem);
  return 0;
}



/*
 * mm_malloc: Almost same implementation as book, 
 * Changed how the place function was called based on whether 
 * extend_heap is called
 */
void *mm_malloc (size_t size) 
{
  size_t asize;      /* Adjusted block size */
  size_t extendsize; /* Amount to extend heap if no fit */
  char *bp;          /* Variable for giving the free blocks */

  /* Initialize heap if no heap */
  if (heap_listp == 0)
  {
	mm_init();
  }
  
  /* Ignore spurious requests */
  if (size == 0)
  { 
    return NULL;
  }
  
  
  /* Adjust block size to include overhead and alignment reqs. */
  if (size <= DSIZE) 
  {                                     
    asize = 2*DSIZE;                                       
  }  
  else
  {
	asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE); 
  }	
  /* Need space for a free-block, so check if at least equal to min_size */
  asize = (asize>=min_size) ? asize:min_size;

  /* Search the free list for a fit */
  if ((bp = find_fit(asize)) != NULL)   
  {
	place(bp, asize,0);  /* The third argument is 0 to show the block exists in bucket */                 
	return bp;
  }

  /* No fit found. Get more memory and place the block */
  extendsize = MAX(asize,CHUNKSIZE);                 
  if ((bp = extend_heap(extendsize/WSIZE)) == NULL)  
  { 
	return NULL;                                 
  }
  /* The third argument is since right after extending, the block does not exist in bucket */
  place(bp, asize,1);                        
  return bp;
} 


/*
 * mm_free: Same code as from the book, 
 * Coalesce is called since there is a newly freed node
 */
void mm_free (void *ptr) 
{
  /* Pointer to be freed is NULL */
  if(ptr == NULL) 
  {
	return;
  }	
  /* If non-NULL pointer, can get its size */
  size_t size = GET_SIZE(HDRP(ptr));
    
  /* The heap has not been initilized */  
  if (heap_listp == 0)
  {
	mm_init();
  }
 
  /* Set header & footer for the newly freed block */
  PUT(HDRP(ptr), PACK(size, 0));
  PUT(FTRP(ptr), PACK(size, 0)); 
  /* Call coalesce since we have a new free block */
  coalesce(ptr);
}




/*
 * Based on the 4 cases, if a free block is adjacent to newly
 * freed block, delete the block if need be.
 *
 * Add the new, possibly bigger free block by getting new pointer
 * based on the updated size
 */
static void *coalesce(void *bp) 
{
  /* Variables to check adjacent blocks if free */
  size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
  size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
  size_t size = GET_SIZE(HDRP(bp));

  if (prev_alloc && next_alloc)            // Case 1 
  {	
    add_node(bp); /* Added here */
    return bp;
  }

  else if (prev_alloc && !next_alloc)      // Case 2
  {
	delete_node(NEXT_BLKP(bp)); /* Deleted here */
	size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size,0));
	add_node(bp); /* Added here */
	
  }

  else if (!prev_alloc && next_alloc)      // Case 3 
  { 
    delete_node(PREV_BLKP(bp)); /* Deleted here */
	size += GET_SIZE(HDRP(PREV_BLKP(bp)));
	PUT(FTRP(bp), PACK(size, 0));
	PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
	bp = PREV_BLKP(bp);
	add_node(bp); /* Added here */
	
  }

  else                                     // Case 4 
  {
	delete_node(NEXT_BLKP(bp)); /* Deleted here */
    delete_node(PREV_BLKP(bp)); /* Deleted here */
	size += GET_SIZE(HDRP(PREV_BLKP(bp))) + 
		    GET_SIZE(FTRP(NEXT_BLKP(bp)));
	PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
	PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
	bp = PREV_BLKP(bp);
	add_node(bp); /* Added here */
	
  }
  return bp;
}



/*
 * Similar to implicit file,
 * no changes were made
 */
void *mm_realloc(void *oldptr, size_t size) 
{
  size_t oldsize;
  void *newptr;
  
  /* If size == 0 then this is just free, and we return NULL. */
  if(size == 0) 
  {
	mm_free(oldptr);
	return 0;
  }
  
  /* If oldptr is NULL, then this is just malloc. */
  if(oldptr == NULL) 
  {
	return mm_malloc(size);
  }
  
  newptr = mm_malloc(size);
  
  /* If realloc() fails the original block is left untouched  */
  if(!newptr) 
  {
	return 0;
  }
  
  /* Copy the old data. */
  oldsize = GET_SIZE(HDRP(oldptr));
  if(size < oldsize) oldsize = size;
    memcpy(newptr, oldptr, oldsize);

  /* Free the old block. */
  mm_free(oldptr);

  return newptr;
}

/*
 * calloc - Refer to mm-native.c,
 * copied from mm-naive.c
 */
void *mm_calloc (size_t nmemb, size_t size) 
{
  size_t bytes = nmemb * size;
  void *newptr;

  newptr = malloc(bytes);
  memset(newptr, 0, bytes);

  return newptr;
}


/*
 * Returns whether the pointer is in the heap. Used in check_heap.
*/ 
static int in_heap(const void *p) 
{
  return p <= mem_heap_hi() && p >= mem_heap_lo();
}


/*
 * Returns whether the pointer is aligned. Used in check_heap.
*/
static int aligned(const void *p) 
{
  return (size_t)ALIGN(p) == (size_t)p;
}

 
/*
 * mm_checkheap: Checks various things in the heap
 */
void mm_checkheap(int lineno) 
{
  lineno = lineno;
  char *bp = heap_listp;  
  /* Variable to store the pointer to free blocks */
  unsigned long *hare = NULL;
  unsigned long *tortoise = NULL;

  hare = dealing_with_lists(count,0,NULL,1);
  tortoise = dealing_with_lists(count,0,NULL,1);;
    
  /* To check for pointer consistency in bucket*/  
  while(hare!=NULL && (GET_NEXT(hare)!=NULL))
  {
    if (GET_PREV(GET_NEXT(hare)) != hare)
    {
      printf("Pointer inconsistency\n");
      exit(1);
    }
    hare = GET_NEXT(hare);
  }
    
  /* To check for cycles in the double linked list */
  while(tortoise != NULL)
  {
    if (((GET_NEXT(tortoise))!=NULL) && 
    (GET_NEXT(hare) !=NULL) && (GET_NEXT(GET_NEXT(hare)))!=NULL)
    {
      tortoise = GET_NEXT(tortoise);
      hare = GET_NEXT(GET_NEXT(hare));
      if (hare == tortoise) 
      {
        printf("HARE = [%p] TORTOISE = [%p]\n",hare,tortoise);
        exit(1);
      }
    }
    else
    {
      break;
    }
  }

  /*Checks if the block is in the heap */
  if ((in_heap(bp))==0)
    printf("Current pointer is not in heap!!!\n");
    
  /*Checks if the block is aligned */
  if((aligned(bp))==0)
    printf("Current pointer is not aligned!!!\n");  
    
  /*Checks if the block prologue is right */
  if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || !GET_ALLOC(HDRP(heap_listp)))
    printf("Bad prologue header\n");
    
  /*Checks if the block epilogue is right*/
  if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp))))
    printf("Bad epilogue header\n");
 
   /*Checks if the block is aligned*/
  if ((size_t)bp % 8)
    printf("Error: %p is not doubleword aligned\n", bp);
  /*Checks if the block header matches footer*/  
  if (GET(HDRP(bp)) != GET(FTRP(bp)))
    printf("Error: header does not match footer\n");

  /* Prints out the whole heap and checkes headers,footers,size,alignment */
  for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) 
  {
    printblock(bp);        /* This function prints the heap at the bp block */
    if ((size_t)bp % 8)
	  printf("Error: %p is not doubleword aligned\n", bp);
    if (GET(HDRP(bp)) != GET(FTRP(bp)))
	  printf("Error: header does not match footer\n");
    if (GET_SIZE(HDRP(bp)) != GET_SIZE(FTRP(bp)))
	  printf("Error: Sizes dont match\n");
  }
  return;
}


/*
 * This function is used by check_heap to print the block specifications,
 * and if block is empty, it prints its next and previous pointers.
 *
 * This helps in checking if the pointers are valid.
 *
*/
static void printblock(void *bp) 
{
  if (GET_SIZE(HDRP(bp)) == 0) 
  {
    printf("%p: EOL\n", bp);
 	return;
  }
  
  printf("%p: header: [%d:%d] footer: [%d:%d]\n", bp, 
	     GET_SIZE(HDRP(bp)), GET_ALLOC(HDRP(bp)), 
	     GET_SIZE(FTRP(bp)), GET_ALLOC(FTRP(bp)));
  if (GET_ALLOC(HDRP(bp))== 0)
  {
    printf("GET_PREV(bp)=%p, GET_NEXT(bp)=%p\n",GET_PREV(bp),GET_NEXT(bp));
  }	     
}



/*
 * This uses the mem_sbrk function to extend the heap
 * and return the address of the newly allocated block
 * with head and footer and epilogue set
*/ 
static void *extend_heap(size_t words) 
{
  /* Variables for calculations */
  char *bp;
  size_t size;

  /* Allocate an even number of words to maintain alignment */
  size = (words % 2) ? (words+1) * WSIZE : words * WSIZE; 
  if ((long)(bp = mem_sbrk(size)) == -1)  
	return NULL;                                       

  /* Initialize free block header/footer and the epilogue header */
  PUT(HDRP(bp), PACK(size, 0));         /* Free block header */   
  PUT(FTRP(bp), PACK(size, 0));         /* Free block footer */   
  PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */ 

  /* Just return the pointer and let the function which called it coalesce it */
  return bp;
}


/* 
 * place - Place block of asize bytes at start of free block bp 
 *         and split if remainder would be at least minimum block size
 * 
 * The third variable (d_number) is set to 1 if the block bp is already 
 * added to a bucket, and thus needs to be deleted
 * Else, there is no need to delete it
 *
 * The left over free space in the block is added to list
 * if it meets the min_size criteria
 */
static void place(void *bp, size_t asize, int d_number)
{
  /* Set the variables */
  void *temp = bp;
  size_t csize = GET_SIZE(HDRP(bp));  
  
  /* Check if the block has leftover space */
  if ((csize - asize) >= (2*DSIZE)) 
  { 
    if (d_number==0) delete_node(bp);
    
    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));
    bp = NEXT_BLKP(bp); 
    
    /* If leftover space is enough to form a free block */
    if ((csize-asize)>=min_size)
    {
	  PUT(HDRP(bp), PACK(csize-asize, 0));
	  PUT(FTRP(bp), PACK(csize-asize, 0));
	  add_node(bp);
    } 
    /* Else we use the free space as padding(bad util?) */
    else
    {
	  PUT(HDRP(temp), PACK(csize, 1));
	  PUT(FTRP(temp), PACK(csize, 1));
    }
  }
  /* If no free space */
  else 
  { 
    if (d_number==0) delete_node(bp);
    
    PUT(HDRP(bp), PACK(csize, 1));
    PUT(FTRP(bp), PACK(csize, 1));
  }
}


/* 
 * find_fit - Find a fit for a block with asize bytes 
 *
 * Since the buckets are ordered, we just go through the bucket 
 * at most counter_max amount of times before moving on to the
 * next bucket.
 *
 * If nothing found after checking all buckets, return NULL
 *
 * Since buckets and each explicit list in bucket increase in size,
 * just traversing through the list is good to find block
 */
static void *find_fit(size_t asize)
{
  /* Set up variables to be used */
  size_t size=0;
  unsigned long *tem_ptr;
  int counter=0;
  /* This is the number of times to travers bucket before moving to next bucket */
  int counter_max = 3; 
  
  /* Call dealing_with_lists to give appropriate list based on size,
     the first call sets the global count (index),
     the second call gives the appropriate bucket
  */  
  dealing_with_lists(0,asize,NULL,0);
  tem_ptr = dealing_with_lists(count,asize,NULL,1);

  
  if (tem_ptr != NULL)                   /*The bucket is non-empty */
  { 
    while ((tem_ptr != NULL))
    { 
      counter++;
      size = GET_SIZE(HDRP(tem_ptr));
      if (size >= asize)                 /* Appropriate block found */
      {
	    return tem_ptr;  
      }
      tem_ptr = GET_NEXT(tem_ptr);       /* Keeping checking next block */
      if ((tem_ptr == NULL)||(counter==counter_max)) 
      /* Need to go to next bucket since current bucket empty*/ 
      {
        tem_ptr = next_list(count);     /* Gives next non empty bucket, if any */
        return tem_ptr;
      }
    }  
  }
  /* The bucket for the size of the block is empty */
  else
  {
    tem_ptr = next_list(count);          /* Gives next non empty bucket, if any */
    return tem_ptr;   
  }
  return NULL;                           /* Else returns NULL */
}



/*
 * This function adds a node into appropriate bucket which is found
 * through the function dealing_with_lists.
 *
 * Adds block according to size, the buckets are ordered.
 *
 * If bucket empty, just add to the bucket.
 *
 * If non empty, traverse through bucket till finds the appropriate place.
*/ 
static void add_node(char *ptr)
{
  /* Initialise the count(index) and appropriate bucket */
  dealing_with_lists(0, GET_SIZE(HDRP(ptr)), NULL,0);  /* Sec count */
  unsigned long *start_of_dll = dealing_with_lists(count, 0, NULL,1);
  /* Initialize variable to be used */
  unsigned long *tem_ptr = start_of_dll;
   
   /* Empty bucket */
  if (start_of_dll==NULL) 
  {
    /* Set the PRED & SUCC pointers accordingly */
    PUT_ADDRESS(PDRP(ptr),NULL);
    PUT_ADDRESS(SCRP(ptr),NULL); 
    /* Set the bucket to point accordingly as well */
    dealing_with_lists(count, 0, (unsigned long*)ptr,2);
    return;
  }
  
  /* Bucket is not empty */
  
  /* Size of block being added to be used to compare where to add block*/
  size_t size_of_ptr = GET_SIZE(HDRP(ptr));
  while(tem_ptr != NULL)
  {
    /* To be added before first block in bucket            Case 1*/ 
  	if ((size_of_ptr < GET_SIZE(HDRP(tem_ptr))) && (tem_ptr==start_of_dll))
  	{ 
  	  /* Set the PRED & SUCC pointers accordingly */
  	  PUT_ADDRESS(PDRP(ptr),NULL);
  	  PUT_ADDRESS(SCRP(ptr),start_of_dll);
  	  PUT_ADDRESS(PDRP(start_of_dll),(unsigned long *)ptr);
  	  dealing_with_lists(count, 0, (unsigned long*)ptr,2);
  	  return;
  	}
  	
  	/* To be added after first block in bucket             Case 2*/
  	else if ((size_of_ptr >= GET_SIZE(HDRP(tem_ptr))) && (tem_ptr==start_of_dll))
  	{
  	  /* Only one other block in bucket */
  	  if (GET_NEXT(start_of_dll) == NULL)
  	  {
  	    /* Set the PRED & SUCC pointers accordingly */
  	    PUT_ADDRESS(SCRP(ptr),NULL);
  	    PUT_ADDRESS(PDRP(ptr),start_of_dll);
  	    PUT_ADDRESS(SCRP(start_of_dll),ptr);
  	  }
  	  /* Other blocks in bucket */
  	  else
  	  {
  	    /* Set the PRED & SUCC pointers accordingly */
  	    PUT_ADDRESS(SCRP(ptr),GET_NEXT(tem_ptr));
  	    PUT_ADDRESS(PDRP(ptr),tem_ptr);
  	    PUT_ADDRESS(PDRP(GET_NEXT(tem_ptr)),ptr);
  	    PUT_ADDRESS(SCRP(tem_ptr),ptr);
  	  }
  	  return;
  	}
  	
  	/* To be added at end of bucket                        Case 3*/
  	else if ((size_of_ptr >= GET_SIZE(HDRP(tem_ptr))) && ((GET_NEXT(tem_ptr)) ==NULL))
    {
       /* Set the PRED & SUCC pointers accordingly */
       PUT_ADDRESS(SCRP(ptr),NULL);
       PUT_ADDRESS(PDRP(ptr),tem_ptr);
       PUT_ADDRESS(SCRP(tem_ptr),ptr);
       return;
    }
    
    /* To be added at middle of bucket                     Case 4*/
    else if ((size_of_ptr >= GET_SIZE(HDRP(tem_ptr))) && ((GET_NEXT(tem_ptr)) !=NULL))
    {
       /* Set the PRED & SUCC pointers accordingly */
       PUT_ADDRESS(SCRP(ptr),GET_NEXT(tem_ptr));
       PUT_ADDRESS(PDRP(ptr), tem_ptr);
       PUT_ADDRESS(SCRP(tem_ptr),ptr);
       PUT_ADDRESS(PDRP(GET_NEXT(tem_ptr)),ptr);
       return;
    }
    
    /* Keep going through list, havent found appropriate place yet */
    else
    {
     tem_ptr = GET_NEXT(tem_ptr);
    }
  }
  return;
}



/*
 * Just has four cases to delete node from appropriate bucket,
 * just delete based off that and set the pointers
*/ 
static void delete_node(char *ptr)
{
  /* TO set the count (index) for the bucket in case it is needed */
  dealing_with_lists(0, GET_SIZE(HDRP(ptr)), NULL,0);

  
  if (GET_PREV(ptr)!=NULL)
  {
    /*There is a block behind it and block in front of it         Case 1*/
    if (GET_NEXT(ptr) != NULL)
    {
      //set previous block's succ to be the succ of deleting block
      PUT_ADDRESS(SCRP(GET_PREV(ptr)),GET_NEXT(ptr));
      //set next block's prev to be prev of deleting block
      PUT_ADDRESS(PDRP(GET_NEXT(ptr)),GET_PREV(ptr));
    }
    /*There is a block behind it and no block in front of it       Case 2*/
    else
    {
      PUT_ADDRESS(SCRP(GET_PREV(ptr)),NULL);
    }
  }
  else 
  {
    /*There is no block behind it and block in front of it         Case 3*/
    if (GET_NEXT(ptr) != NULL)
    {
      dealing_with_lists(count,0,(unsigned long *)(GET_NEXT(ptr)),2);
      PUT_ADDRESS(PDRP(GET_NEXT(ptr)),NULL);
      PUT_ADDRESS(SCRP(ptr),NULL);
    }
    /*There is no block behind it and no block in front of it      Case 4*/
    else
    {
      dealing_with_lists(count, 0, NULL,2); 
    }  
  }
  return;
}


/* This function has 3 functions depending on input option:
   Option==0: Takes in size and sets the global variable count 
              to the appropriate bucket index
   Option==1: Takes in the count and gives the appropriate bucket address
   Option==2: Takes the bucket and sets its value to that of ptr
*/ 
unsigned long *dealing_with_lists(int counter, size_t size, unsigned long *ptr,int option)
{
  if (option==0)
  {
    /* The count (index) is set on the size range,
     * each bucket size range is (2^(x-1)) to ((2^x)-1)
    */ 
    if ((size >= (1<<(1-1))) && (size <= ((1<<1)-1))) count=1;
    else if ((size >= (1<<(2-1))) && (size <= ((1<<2)-1))) count=2;
    else if ((size >= (1<<(3-1))) && (size <= ((1<<3)-1))) count=3;
    else if ((size >= (1<<(4-1))) && (size <= ((1<<4)-1))) count=4;
    else if ((size >= (1<<(5-1))) && (size <= ((1<<5)-1))) count=5;
    else if ((size >= (1<<(6-1))) && (size <= ((1<<6)-1))) count=6;
    else if ((size >= (1<<(7-1))) && (size <= ((1<<7)-1))) count=7;
    else if ((size >= (1<<(8-1))) && (size <= ((1<<8)-1))) count=8;
    else if ((size >= (1<<(9-1))) && (size <= ((1<<9)-1))) count=9;
    else if ((size >= (1<<(10-1))) && (size <= ((1<<10)-1))) count=10;
    else if ((size >= (1<<(11-1))) && (size <= ((1<<11)-1))) count=11;
    else if ((size >= (1<<(12-1))) && (size <= ((1<<12)-1))) count=12;
    else if ((size >= (1<<(13-1))) && (size <= ((1<<13)-1))) count=13;
    else if ((size >= (1<<(14-1))) && (size <= ((1<<14)-1))) count=14;
    else if ((size >= (1<<(15-1))) && (size <= ((1<<15)-1))) count=15;
    else if ((size >= (1<<(16-1))) && (size <= ((1<<16)-1))) count=16;
    else if ((size >= (1<<(17-1))) && (size <= ((1<<17)-1))) count=17;
    else if ((size >= (1<<(18-1))) && (size <= ((1<<18)-1))) count=18;
    else if ((size >= (1<<(19-1))) && (size <= ((1<<19)-1))) count=19;
    else if ((size >= (1<<(20-1))) && (size <= ((1<<20)-1))) count=20;
    else if ((size >= (1<<(21-1))) && (size <= ((1<<21)-1))) count=21;
    else if ((size >= (1<<(22-1))) && (size <= ((1<<22)-1))) count=22;
    else if ((size >= (1<<(23-1))) && (size <= ((1<<23)-1))) count=23;
    else if ((size >= (1<<(24-1))) && (size <= ((1<<24)-1))) count=24;
    else if ((size >= (1<<(25-1))) && (size <= ((1<<25)-1))) count=25;
    else if ((size >= (1<<(26-1))) && (size <= ((1<<26)-1))) count=26;
    else if ((size >= (1<<(27-1))) && (size <= ((1<<27)-1))) count=27;
    else if ((size >= (1<<(28-1))) && (size <= ((1<<28)-1))) count=28;
    else if ((size >= (1<<(29-1))) && (size <= ((1<<29)-1))) count=29;
    else if ((size >= (1<<(30-1))) && (size <= ((1<<30)-1))) count=30;
    else if ((size >= (1<<(31-1))) && (size <= ((unsigned int)(1<<31)-1))) count=31;
    else count = 32;
    return NULL;
  } 
  /* Size is 0 and ptr is NULL, based on count, gives back appropriate seg list*/
  else if (option==1)
  {
  switch (counter)
  {
      case (1): return start_of_dll1;
      case (2): return start_of_dll2;
      case (3): return start_of_dll3;
      case (4): return start_of_dll4;
      case (5): return start_of_dll5;
      case (6): return start_of_dll6;
      case (7): return start_of_dll7;
      case (8): return start_of_dll8;
      case (9): return start_of_dll9;
      case (10): return start_of_dll10;
      case (11): return start_of_dll11;
      case (12): return start_of_dll12;
      case (13): return start_of_dll13;
      case (14): return start_of_dll14;
      case (15): return start_of_dll15;
      case (16): return start_of_dll16;
      case (17): return start_of_dll17;
      case (18): return start_of_dll18;
      case (19): return start_of_dll19;
      case (20): return start_of_dll20;
      case (21): return start_of_dll21;
      case (22): return start_of_dll22;
      case (23): return start_of_dll23;
      case (24): return start_of_dll24;
      case (25): return start_of_dll25;
      case (26): return start_of_dll26;
      case (27): return start_of_dll27;
      case (28): return start_of_dll28;
      case (29): return start_of_dll29;
      case (30): return start_of_dll30;
      case (31): return start_of_dll31;
      case (32): return start_of_dll32; 
      default: return start_of_dll15;
    }
  }
  /* Assign value of ptr tolist*/
  else
  {
    switch(counter)
    {
      case (1): 
      {
        start_of_dll1=ptr;
        break;
      }  
      case (2): 
      {
        start_of_dll2=ptr;
        break;
      } 
      case (3): 
      {
        start_of_dll3=ptr;
        break;
      } 
      case (4): 
      {
        start_of_dll4=ptr;
        break;
      } 
      case (5): 
      {
        start_of_dll5=ptr;
        break;
      } 
      case (6): 
      {
        start_of_dll6=ptr;
        break;
      } 
      case (7): 
      {
        start_of_dll7=ptr;
        break;
      } 
      case (8):
      {
        start_of_dll8=ptr;
        break;
      } 
      case (9): 
      {
        start_of_dll9=ptr;
        break;
      } 
      case (10): 
      {
        start_of_dll10=ptr;
        break;
      }
      case (11): 
      {
        start_of_dll11=ptr;
        break;
      } 
      case (12): 
      {
        start_of_dll12=ptr;
        break;
      } 
      case (13): 
      {
        start_of_dll13=ptr;
        break;
      } 
      case (14): 
      {
        start_of_dll14=ptr;
        break;
      } 
      case (15): 
      {
        start_of_dll15=ptr;
        break;
      } 
      case (16): 
      {
        start_of_dll16=ptr;
        break;
      } 
      case (17): 
      {
        start_of_dll17=ptr;
        break;
      } 
      case (18): 
      {
        start_of_dll18=ptr;
        break;
      } 
      case (19): 
      {
        start_of_dll19=ptr;
        break;
      } 
      case (20): 
      {
        start_of_dll20=ptr;
        break;
      } 
      case (21): 
      {
        start_of_dll21=ptr;
        break;
      } 
      case (22): 
      {
        start_of_dll22=ptr;
        break;
      } 
      case (23): 
      {
        start_of_dll23=ptr;
        break;
      } 
      case (24): 
      {
        start_of_dll24=ptr;
        break;
      } 
      case (25): 
      {
        start_of_dll25=ptr;
        break;
      } 
      case (26): 
      {
        start_of_dll26=ptr;
        break;
      } 
      case (27):
      {
        start_of_dll27=ptr;
        break;
      } 
      case (28): 
      {
        start_of_dll28=ptr;
        break;
      } 
      case (29): 
      {
        start_of_dll29=ptr;
        break;
      } 
      case (30): 
      {
        start_of_dll30=ptr;
        break;
      } 
      case (31): 
      {
        start_of_dll31=ptr;
        break;
      } 
      case (32): 
      {
        start_of_dll32=ptr;
        break;
      } 
      default: 
      {
        start_of_dll32=ptr;
        break;
      } 
    }
    return NULL;  
  }
  return NULL;
}


/*
 * This function returns the next non empty bucket given the 
 * count (index), and returns NULL if all the buckets are empty
 * 
 * Since buckets are arranged in ascending order, we cascade 
 * through the cases
 *
 * There might be a more efficient method, but this gave the best
 * throughput.
*/
unsigned long *next_list(int number)
{
  if ((number==1))
  {
    if (start_of_dll2 != NULL) return start_of_dll2;
    else if (start_of_dll3 !=NULL) return start_of_dll3;
    else if (start_of_dll4 !=NULL) return start_of_dll4;
    else if (start_of_dll5 !=NULL) return start_of_dll5;
    else if (start_of_dll6 !=NULL) return start_of_dll6;
    else if (start_of_dll7 !=NULL) return start_of_dll7;
    else if (start_of_dll8 !=NULL) return start_of_dll8;
    else if (start_of_dll9 !=NULL) return start_of_dll9;
    else if (start_of_dll10 !=NULL) return start_of_dll10;
    else if (start_of_dll11 !=NULL) return start_of_dll11;
    else if (start_of_dll12 !=NULL) return start_of_dll12;
    else if (start_of_dll13 !=NULL) return start_of_dll13;
    else if (start_of_dll14 !=NULL) return start_of_dll14;
    else if (start_of_dll15 !=NULL) return start_of_dll15;
    else if (start_of_dll16 !=NULL) return start_of_dll16;
    else if (start_of_dll17 !=NULL) return start_of_dll17;
    else if (start_of_dll18 !=NULL) return start_of_dll18;
    else if (start_of_dll19 !=NULL) return start_of_dll19;
    else if (start_of_dll20 !=NULL) return start_of_dll20;
    else if (start_of_dll21 !=NULL) return start_of_dll21;
    else if (start_of_dll22 !=NULL) return start_of_dll22;
    else if (start_of_dll23 !=NULL) return start_of_dll23;
    else if (start_of_dll24 !=NULL) return start_of_dll24;
    else if (start_of_dll25 !=NULL) return start_of_dll25;
    else if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }  
  else if ((number==2))
  {
    if (start_of_dll3 !=NULL) return start_of_dll3;
    else if (start_of_dll4 !=NULL) return start_of_dll4;
    else if (start_of_dll5 !=NULL) return start_of_dll5;
    else if (start_of_dll6 !=NULL) return start_of_dll6;
    else if (start_of_dll7 !=NULL) return start_of_dll7;
    else if (start_of_dll8 !=NULL) return start_of_dll8;
    else if (start_of_dll9 !=NULL) return start_of_dll9;
    else if (start_of_dll10 !=NULL) return start_of_dll10;
    else if (start_of_dll11 !=NULL) return start_of_dll11;
    else if (start_of_dll12 !=NULL) return start_of_dll12;
    else if (start_of_dll13 !=NULL) return start_of_dll13;
    else if (start_of_dll14 !=NULL) return start_of_dll14;
    else if (start_of_dll15 !=NULL) return start_of_dll15;
    else if (start_of_dll16 !=NULL) return start_of_dll16;
    else if (start_of_dll17 !=NULL) return start_of_dll17;
    else if (start_of_dll18 !=NULL) return start_of_dll18;
    else if (start_of_dll19 !=NULL) return start_of_dll19;
    else if (start_of_dll20 !=NULL) return start_of_dll20;
    else if (start_of_dll21 !=NULL) return start_of_dll21;
    else if (start_of_dll22 !=NULL) return start_of_dll22;
    else if (start_of_dll23 !=NULL) return start_of_dll23;
    else if (start_of_dll24 !=NULL) return start_of_dll24;
    else if (start_of_dll25 !=NULL) return start_of_dll25;
    else if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==3))
  {
    if (start_of_dll4 !=NULL) return start_of_dll4;
    else if (start_of_dll5 !=NULL) return start_of_dll5;
    else if (start_of_dll6 !=NULL) return start_of_dll6;
    else if (start_of_dll7 !=NULL) return start_of_dll7;
    else if (start_of_dll8 !=NULL) return start_of_dll8;
    else if (start_of_dll9 !=NULL) return start_of_dll9;
    else if (start_of_dll10 !=NULL) return start_of_dll10;
    else if (start_of_dll11 !=NULL) return start_of_dll11;
    else if (start_of_dll12 !=NULL) return start_of_dll12;
    else if (start_of_dll13 !=NULL) return start_of_dll13;
    else if (start_of_dll14 !=NULL) return start_of_dll14;
    else if (start_of_dll15 !=NULL) return start_of_dll15;
    else if (start_of_dll16 !=NULL) return start_of_dll16;
    else if (start_of_dll17 !=NULL) return start_of_dll17;
    else if (start_of_dll18 !=NULL) return start_of_dll18;
    else if (start_of_dll19 !=NULL) return start_of_dll19;
    else if (start_of_dll20 !=NULL) return start_of_dll20;
    else if (start_of_dll21 !=NULL) return start_of_dll21;
    else if (start_of_dll22 !=NULL) return start_of_dll22;
    else if (start_of_dll23 !=NULL) return start_of_dll23;
    else if (start_of_dll24 !=NULL) return start_of_dll24;
    else if (start_of_dll25 !=NULL) return start_of_dll25;
    else if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==4))
  {
    if (start_of_dll5 !=NULL) return start_of_dll5;
    else if (start_of_dll6 !=NULL) return start_of_dll6;
    else if (start_of_dll7 !=NULL) return start_of_dll7;
    else if (start_of_dll8 !=NULL) return start_of_dll8;
    else if (start_of_dll9 !=NULL) return start_of_dll9;
    else if (start_of_dll10 !=NULL) return start_of_dll10;
    else if (start_of_dll11 !=NULL) return start_of_dll11;
    else if (start_of_dll12 !=NULL) return start_of_dll12;
    else if (start_of_dll13 !=NULL) return start_of_dll13;
    else if (start_of_dll14 !=NULL) return start_of_dll14;
    else if (start_of_dll15 !=NULL) return start_of_dll15;
    else if (start_of_dll16 !=NULL) return start_of_dll16;
    else if (start_of_dll17 !=NULL) return start_of_dll17;
    else if (start_of_dll18 !=NULL) return start_of_dll18;
    else if (start_of_dll19 !=NULL) return start_of_dll19;
    else if (start_of_dll20 !=NULL) return start_of_dll20;
    else if (start_of_dll21 !=NULL) return start_of_dll21;
    else if (start_of_dll22 !=NULL) return start_of_dll22;
    else if (start_of_dll23 !=NULL) return start_of_dll23;
    else if (start_of_dll24 !=NULL) return start_of_dll24;
    else if (start_of_dll25 !=NULL) return start_of_dll25;
    else if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==5))
  {
    if (start_of_dll6 !=NULL) return start_of_dll6;
    else if (start_of_dll7 !=NULL) return start_of_dll7;
    else if (start_of_dll8 !=NULL) return start_of_dll8;
    else if (start_of_dll9 !=NULL) return start_of_dll9;
    else if (start_of_dll10 !=NULL) return start_of_dll10;
    else if (start_of_dll11 !=NULL) return start_of_dll11;
    else if (start_of_dll12 !=NULL) return start_of_dll12;
    else if (start_of_dll13 !=NULL) return start_of_dll13;
    else if (start_of_dll14 !=NULL) return start_of_dll14;
    else if (start_of_dll15 !=NULL) return start_of_dll15;
    else if (start_of_dll16 !=NULL) return start_of_dll16;
    else if (start_of_dll17 !=NULL) return start_of_dll17;
    else if (start_of_dll18 !=NULL) return start_of_dll18;
    else if (start_of_dll19 !=NULL) return start_of_dll19;
    else if (start_of_dll20 !=NULL) return start_of_dll20;
    else if (start_of_dll21 !=NULL) return start_of_dll21;
    else if (start_of_dll22 !=NULL) return start_of_dll22;
    else if (start_of_dll23 !=NULL) return start_of_dll23;
    else if (start_of_dll24 !=NULL) return start_of_dll24;
    else if (start_of_dll25 !=NULL) return start_of_dll25;
    else if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==6))
  {
    if (start_of_dll7 !=NULL) return start_of_dll7;
    else if (start_of_dll8 !=NULL) return start_of_dll8;
    else if (start_of_dll9 !=NULL) return start_of_dll9;
    else if (start_of_dll10 !=NULL) return start_of_dll10;
    else if (start_of_dll11 !=NULL) return start_of_dll11;
    else if (start_of_dll12 !=NULL) return start_of_dll12;
    else if (start_of_dll13 !=NULL) return start_of_dll13;
    else if (start_of_dll14 !=NULL) return start_of_dll14;
    else if (start_of_dll15 !=NULL) return start_of_dll15;
    else if (start_of_dll16 !=NULL) return start_of_dll16;
    else if (start_of_dll17 !=NULL) return start_of_dll17;
    else if (start_of_dll18 !=NULL) return start_of_dll18;
    else if (start_of_dll19 !=NULL) return start_of_dll19;
    else if (start_of_dll20 !=NULL) return start_of_dll20;
    else if (start_of_dll21 !=NULL) return start_of_dll21;
    else if (start_of_dll22 !=NULL) return start_of_dll22;
    else if (start_of_dll23 !=NULL) return start_of_dll23;
    else if (start_of_dll24 !=NULL) return start_of_dll24;
    else if (start_of_dll25 !=NULL) return start_of_dll25;
    else if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==7))
  {
    if (start_of_dll8 !=NULL) return start_of_dll8;
    else if (start_of_dll9 !=NULL) return start_of_dll9;
    else if (start_of_dll10 !=NULL) return start_of_dll10;
    else if (start_of_dll11 !=NULL) return start_of_dll11;
    else if (start_of_dll12 !=NULL) return start_of_dll12;
    else if (start_of_dll13 !=NULL) return start_of_dll13;
    else if (start_of_dll14 !=NULL) return start_of_dll14;
    else if (start_of_dll15 !=NULL) return start_of_dll15;
    else if (start_of_dll16 !=NULL) return start_of_dll16;
    else if (start_of_dll17 !=NULL) return start_of_dll17;
    else if (start_of_dll18 !=NULL) return start_of_dll18;
    else if (start_of_dll19 !=NULL) return start_of_dll19;
    else if (start_of_dll20 !=NULL) return start_of_dll20;
    else if (start_of_dll21 !=NULL) return start_of_dll21;
    else if (start_of_dll22 !=NULL) return start_of_dll22;
    else if (start_of_dll23 !=NULL) return start_of_dll23;
    else if (start_of_dll24 !=NULL) return start_of_dll24;
    else if (start_of_dll25 !=NULL) return start_of_dll25;
    else if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==8))
  {
    if (start_of_dll9 !=NULL) return start_of_dll9;
    else if (start_of_dll10 !=NULL) return start_of_dll10;
    else if (start_of_dll11 !=NULL) return start_of_dll11;
    else if (start_of_dll12 !=NULL) return start_of_dll12;
    else if (start_of_dll13 !=NULL) return start_of_dll13;
    else if (start_of_dll14 !=NULL) return start_of_dll14;
    else if (start_of_dll15 !=NULL) return start_of_dll15;
    else if (start_of_dll16 !=NULL) return start_of_dll16;
    else if (start_of_dll17 !=NULL) return start_of_dll17;
    else if (start_of_dll18 !=NULL) return start_of_dll18;
    else if (start_of_dll19 !=NULL) return start_of_dll19;
    else if (start_of_dll20 !=NULL) return start_of_dll20;
    else if (start_of_dll21 !=NULL) return start_of_dll21;
    else if (start_of_dll22 !=NULL) return start_of_dll22;
    else if (start_of_dll23 !=NULL) return start_of_dll23;
    else if (start_of_dll24 !=NULL) return start_of_dll24;
    else if (start_of_dll25 !=NULL) return start_of_dll25;
    else if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==9))
  {
    if (start_of_dll10 !=NULL) return start_of_dll10;
    else if (start_of_dll11 !=NULL) return start_of_dll11;
    else if (start_of_dll12 !=NULL) return start_of_dll12;
    else if (start_of_dll13 !=NULL) return start_of_dll13;
    else if (start_of_dll14 !=NULL) return start_of_dll14;
    else if (start_of_dll15 !=NULL) return start_of_dll15;
    else if (start_of_dll16 !=NULL) return start_of_dll16;
    else if (start_of_dll17 !=NULL) return start_of_dll17;
    else if (start_of_dll18 !=NULL) return start_of_dll18;
    else if (start_of_dll19 !=NULL) return start_of_dll19;
    else if (start_of_dll20 !=NULL) return start_of_dll20;
    else if (start_of_dll21 !=NULL) return start_of_dll21;
    else if (start_of_dll22 !=NULL) return start_of_dll22;
    else if (start_of_dll23 !=NULL) return start_of_dll23;
    else if (start_of_dll24 !=NULL) return start_of_dll24;
    else if (start_of_dll25 !=NULL) return start_of_dll25;
    else if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==10))
  {
    if (start_of_dll11 !=NULL) return start_of_dll11;
    else if (start_of_dll12 !=NULL) return start_of_dll12;
    else if (start_of_dll13 !=NULL) return start_of_dll13;
    else if (start_of_dll14 !=NULL) return start_of_dll14;
    else if (start_of_dll15 !=NULL) return start_of_dll15;
    else if (start_of_dll16 !=NULL) return start_of_dll16;
    else if (start_of_dll17 !=NULL) return start_of_dll17;
    else if (start_of_dll18 !=NULL) return start_of_dll18;
    else if (start_of_dll19 !=NULL) return start_of_dll19;
    else if (start_of_dll20 !=NULL) return start_of_dll20;
    else if (start_of_dll21 !=NULL) return start_of_dll21;
    else if (start_of_dll22 !=NULL) return start_of_dll22;
    else if (start_of_dll23 !=NULL) return start_of_dll23;
    else if (start_of_dll24 !=NULL) return start_of_dll24;
    else if (start_of_dll25 !=NULL) return start_of_dll25;
    else if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==11))
  {
    if (start_of_dll12 !=NULL) return start_of_dll12;
    else if (start_of_dll13 !=NULL) return start_of_dll13;
    else if (start_of_dll14 !=NULL) return start_of_dll14;
    else if (start_of_dll15 !=NULL) return start_of_dll15;
    else if (start_of_dll16 !=NULL) return start_of_dll16;
    else if (start_of_dll17 !=NULL) return start_of_dll17;
    else if (start_of_dll18 !=NULL) return start_of_dll18;
    else if (start_of_dll19 !=NULL) return start_of_dll19;
    else if (start_of_dll20 !=NULL) return start_of_dll20;
    else if (start_of_dll21 !=NULL) return start_of_dll21;
    else if (start_of_dll22 !=NULL) return start_of_dll22;
    else if (start_of_dll23 !=NULL) return start_of_dll23;
    else if (start_of_dll24 !=NULL) return start_of_dll24;
    else if (start_of_dll25 !=NULL) return start_of_dll25;
    else if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==12))
  {
    if (start_of_dll13 !=NULL) return start_of_dll13;
    else if (start_of_dll14 !=NULL) return start_of_dll14;
    else if (start_of_dll15 !=NULL) return start_of_dll15;
    else if (start_of_dll16 !=NULL) return start_of_dll16;
    else if (start_of_dll17 !=NULL) return start_of_dll17;
    else if (start_of_dll18 !=NULL) return start_of_dll18;
    else if (start_of_dll19 !=NULL) return start_of_dll19;
    else if (start_of_dll20 !=NULL) return start_of_dll20;
    else if (start_of_dll21 !=NULL) return start_of_dll21;
    else if (start_of_dll22 !=NULL) return start_of_dll22;
    else if (start_of_dll23 !=NULL) return start_of_dll23;
    else if (start_of_dll24 !=NULL) return start_of_dll24;
    else if (start_of_dll25 !=NULL) return start_of_dll25;
    else if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==13))
  {
    if (start_of_dll14 !=NULL) return start_of_dll14;
    else if (start_of_dll15 !=NULL) return start_of_dll15;
    else if (start_of_dll16 !=NULL) return start_of_dll16;
    else if (start_of_dll17 !=NULL) return start_of_dll17;
    else if (start_of_dll18 !=NULL) return start_of_dll18;
    else if (start_of_dll19 !=NULL) return start_of_dll19;
    else if (start_of_dll20 !=NULL) return start_of_dll20;
    else if (start_of_dll21 !=NULL) return start_of_dll21;
    else if (start_of_dll22 !=NULL) return start_of_dll22;
    else if (start_of_dll23 !=NULL) return start_of_dll23;
    else if (start_of_dll24 !=NULL) return start_of_dll24;
    else if (start_of_dll25 !=NULL) return start_of_dll25;
    else if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==14))
  {
    if (start_of_dll15 !=NULL) return start_of_dll15;
    else if (start_of_dll16 !=NULL) return start_of_dll16;
    else if (start_of_dll17 !=NULL) return start_of_dll17;
    else if (start_of_dll18 !=NULL) return start_of_dll18;
    else if (start_of_dll19 !=NULL) return start_of_dll19;
    else if (start_of_dll20 !=NULL) return start_of_dll20;
    else if (start_of_dll21 !=NULL) return start_of_dll21;
    else if (start_of_dll22 !=NULL) return start_of_dll22;
    else if (start_of_dll23 !=NULL) return start_of_dll23;
    else if (start_of_dll24 !=NULL) return start_of_dll24;
    else if (start_of_dll25 !=NULL) return start_of_dll25;
    else if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==15))
  {
    if (start_of_dll16 !=NULL) return start_of_dll16;
    else if (start_of_dll17 !=NULL) return start_of_dll17;
    else if (start_of_dll18 !=NULL) return start_of_dll18;
    else if (start_of_dll19 !=NULL) return start_of_dll19;
    else if (start_of_dll20 !=NULL) return start_of_dll20;
    else if (start_of_dll21 !=NULL) return start_of_dll21;
    else if (start_of_dll22 !=NULL) return start_of_dll22;
    else if (start_of_dll23 !=NULL) return start_of_dll23;
    else if (start_of_dll24 !=NULL) return start_of_dll24;
    else if (start_of_dll25 !=NULL) return start_of_dll25;
    else if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==16))
  {
    if (start_of_dll17 !=NULL) return start_of_dll17;
    else if (start_of_dll18 !=NULL) return start_of_dll18;
    else if (start_of_dll19 !=NULL) return start_of_dll19;
    else if (start_of_dll20 !=NULL) return start_of_dll20;
    else if (start_of_dll21 !=NULL) return start_of_dll21;
    else if (start_of_dll22 !=NULL) return start_of_dll22;
    else if (start_of_dll23 !=NULL) return start_of_dll23;
    else if (start_of_dll24 !=NULL) return start_of_dll24;
    else if (start_of_dll25 !=NULL) return start_of_dll25;
    else if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==17))
  {
    if (start_of_dll18 !=NULL) return start_of_dll18;
    else if (start_of_dll19 !=NULL) return start_of_dll19;
    else if (start_of_dll20 !=NULL) return start_of_dll20;
    else if (start_of_dll21 !=NULL) return start_of_dll21;
    else if (start_of_dll22 !=NULL) return start_of_dll22;
    else if (start_of_dll23 !=NULL) return start_of_dll23;
    else if (start_of_dll24 !=NULL) return start_of_dll24;
    else if (start_of_dll25 !=NULL) return start_of_dll25;
    else if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==18))
  {
    if (start_of_dll19 !=NULL) return start_of_dll19;
    else if (start_of_dll20 !=NULL) return start_of_dll20;
    else if (start_of_dll21 !=NULL) return start_of_dll21;
    else if (start_of_dll22 !=NULL) return start_of_dll22;
    else if (start_of_dll23 !=NULL) return start_of_dll23;
    else if (start_of_dll24 !=NULL) return start_of_dll24;
    else if (start_of_dll25 !=NULL) return start_of_dll25;
    else if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==19))
  {
    if (start_of_dll20 !=NULL) return start_of_dll20;
    else if (start_of_dll21 !=NULL) return start_of_dll21;
    else if (start_of_dll22 !=NULL) return start_of_dll22;
    else if (start_of_dll23 !=NULL) return start_of_dll23;
    else if (start_of_dll24 !=NULL) return start_of_dll24;
    else if (start_of_dll25 !=NULL) return start_of_dll25;
    else if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==20))
  {
    if (start_of_dll21 !=NULL) return start_of_dll21;
    else if (start_of_dll22 !=NULL) return start_of_dll22;
    else if (start_of_dll23 !=NULL) return start_of_dll23;
    else if (start_of_dll24 !=NULL) return start_of_dll24;
    else if (start_of_dll25 !=NULL) return start_of_dll25;
    else if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==21))
  {
    if (start_of_dll22 !=NULL) return start_of_dll22;
    else if (start_of_dll23 !=NULL) return start_of_dll23;
    else if (start_of_dll24 !=NULL) return start_of_dll24;
    else if (start_of_dll25 !=NULL) return start_of_dll25;
    else if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==22))
  {
    if (start_of_dll23 !=NULL) return start_of_dll23;
    else if (start_of_dll24 !=NULL) return start_of_dll24;
    else if (start_of_dll25 !=NULL) return start_of_dll25;
    else if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==23))
  {
    if (start_of_dll24 !=NULL) return start_of_dll24;
    else if (start_of_dll25 !=NULL) return start_of_dll25;
    else if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==24))
  {
    if (start_of_dll25 !=NULL) return start_of_dll25;
    else if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==25))
  {
    if (start_of_dll26 !=NULL) return start_of_dll26;
    else if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==26))
  {
    if (start_of_dll27 !=NULL) return start_of_dll27;
    else if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==27))
  {
    if (start_of_dll28 !=NULL) return start_of_dll28;
    else if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==28))
  {
    if (start_of_dll29 !=NULL) return start_of_dll29;
    else if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==29))
  {
    if (start_of_dll30 !=NULL) return start_of_dll30;
    else if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==30))
  {
    if (start_of_dll31 !=NULL) return start_of_dll31;
    else if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==31))
  {
    if (start_of_dll32 !=NULL) return start_of_dll32;
  }
  else if ((number==32))
  {
    return NULL;
  }
  else 
  {
    return NULL;
  } 
  return NULL;
}



