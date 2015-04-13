/******************************************************************************
 * FILENAME: mem.c
 * AUTHOR:   cherin@cs.wisc.edu <Cherin Joseph>
 * DATE:     20 Nov 2013
 * PROVIDES: Contains a set of library functions for memory allocation
 * * MODIFIED BY:  Jason Tiedt, 1
 * *****************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "mem.h"

/* this structure serves as the header for each block */
typedef struct block_hd{
  /* The blocks are maintained as a linked list */
  /* The blocks are ordered in the increasing order of addresses */
  struct block_hd *next;

  /* size of the block is always a multiple of 4 */
  /* ie, last two bits are always zero - can be used to store other information*/
  /* LSB = 0 => free block */
  /* LSB = 1 => allocated/busy block */

  /* For free block, block size = size_status */
  /* For an allocated block, block size = size_status - 1 */

  /* The size of the block stored here is not the real size of the block */
  /* the size stored here = (size of block) - (size of header) */
  int size_status;

}block_header;

/* Global variable - This will always point to the first block */
/* ie, the block with the lowest address */
block_header* list_head = NULL;
long totalsize;

/* Function used to Initialize the memory allocator */
/* Not intended to be called more than once by a program */
/* Argument - sizeOfRegion: Specifies the size of the chunk which needs to be allocated */
/* Returns 0 on success and -1 on failure */
int Mem_Init(int sizeOfRegion)
{
  int pagesize;
  int padsize;
  int fd;
  int alloc_size;
  //totalsize = sizeOfRegion;
 // void* space_ptr;
  static int allocated_once = 0;
//  int m_error = 0;
  if(0 != allocated_once)
  {
    fprintf(stderr,"Error:mem.c: Mem_Init has allocated space during a previous call\n");
    return -1;
  }
  if(sizeOfRegion <= 0)
  {
    fprintf(stderr,"Error:mem.c: Requested block size is not positive\n");
    return -1;
  }

  /* Get the pagesize */
  pagesize = getpagesize();

  /* Calculate padsize as the padding required to round up sizeOfRegio to a multiple of pagesize */
  padsize = sizeOfRegion % pagesize;
  padsize = (pagesize - padsize) % pagesize;

  alloc_size = sizeOfRegion + padsize;

  /* Using mmap to allocate memory */
  fd = open("/dev/zero", O_RDWR);
  if(-1 == fd)
  {
    fprintf(stderr,"Error:mem.c: Cannot open /dev/zero\n");
    return -1;
  }
  block_header* space_ptr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  list_head = space_ptr;
  if (MAP_FAILED == space_ptr)
  {
    fprintf(stderr,"Error:mem.c: mmap cannot allocate space\n");
    allocated_once = 0;
    return -1;
  }
  
  allocated_once = 1;
  
  /* To begin with, there is only one big, free block */
 // list_head = (block_header*)space_ptr;
  space_ptr->next = NULL;
  /* Remember that the 'size' stored in block size excludes the space for the header */
  space_ptr->size_status = alloc_size - sizeof(block_header);
  printf("starting mem_addr is %p\n", list_head);
  printf("size of the mem is %d\n", space_ptr->size_status);
  close(fd);
  return 0;
}


/* Function for allocating 'size' bytes.*/
/* Returns address of allocated block on success */
/* Returns NULL on failure */
/* Here is what this function should accomplish */
/* - Check for sanity of size - Return NULL when appropriate */
/* - Round up size to a multiple of 4 */
/* - Traverse the list of blocks and allocate the first free block which can accommodate the requested size */
/* -- Also, when allocating a block - split it into two blocks when possible */
/* Tips: Be careful with pointer arithmetic */
void* Mem_Alloc(int size)
{
  /* Your code should go in here */
    //size = size + sizeof(block_header);
    if(size%8 != 0)
	size = size + 8 - size%8;
    //traverse list again, adding up size?
//    int m_error = 0;
//    int done = 0;
    //Start with list_head
    block_header* itr = list_head;
  //  block_header* temp_head = NULL;
    //block_header* new_node = NULL;
  /*  long current_size=0;

    while(itr != NULL){
       current_size += itr->size_status+sizeof(block_header);
       itr=itr->next;
    }
    if(current_size+size+sizeof(block_header) > totalsize){
	return NULL;
    }
*/    
    itr = list_head;

    block_header* temp_head = NULL;
    block_header* new_node = NULL;
    int m_error = 0;
    int done = 0;



    while(itr != NULL && (done != 1)){ 			//might need a done signal to loop over.   
        if((itr->size_status < size) || (itr->size_status & 1)){
            itr=itr->next;
            
        }
        else{							//we actually dont need to check if the black is alloced or not, since alloced block's LSB is 1 so it wont be 
            if((itr->size_status) == size){				//equal to the size we r going to alloc
            //allocate
                itr->size_status = itr->size_status+1;
               	done = 1;
		printf("\nperfect fit\n");
                return itr;
            }
            else if(itr->size_status - size > sizeof(block_header)){
                //allocate, partition
                //int itrSave = itr->size_status;
                temp_head = itr->next;
		new_node = (void*)itr + size + sizeof(block_header);
		new_node->size_status = itr->size_status - size - sizeof(block_header);
		itr->next = new_node;
		new_node->next = temp_head;
		//itr->next->next = temp_head;
              	itr->size_status = size + sizeof(block_header)+1;
		done = 1;
	/*	printf("\n fit\n");
		printf("\n %p\n", new_node);
		printf("\n sizeof(block_header) %ld \n", sizeof(block_header));
		printf("\n itr+sizeof(block_header: %p\n", (void*)itr+sizeof(block_header));
*/
		return((void*) itr+sizeof(block_header));
            }
		else{itr=itr->next;}
        }
    }
    m_error = 1; //E_NO_SPACE;
    return NULL;
}

/* Function for freeing up a previously allocated block */
/* Argument - ptr: Address of the block to be freed up */
/* Returns 0 on success */
/* Returns -1 on failure */
/* Here is what this function should accomplish */
/* - Return -1 if ptr is NULL */
/* - Return -1 if ptr is not pointing to the first byte of a busy block */
/* - Mark the block as free */
/* - Coalesce if one or both of the immediate neighbours are free */
int Mem_Free(void *ptr)
{

  /* Your code should go in here */
    if(ptr == NULL){
        return -1;
    }
    int done = 0; //done signal
    block_header* itr = list_head;
    //block_header* ptrLoc = (block_header*)ptr;
    block_header* newItr = list_head->next;
    block_header* prevItr = list_head;
    while(itr != NULL && (!done)){
        // compare the location of the header to the location to be free'd
        //char* itrbasic = (char*) itr;
        //char* ptrbasic = (char*) ptrLoc;
/*printf("\nlooking\n");
printf("\n%p\n",itr);
printf("\n%p\n",ptr);
*/
        if((void*)itr+sizeof(block_header) == ptr   /*ptrLoc+(char*)sizeof(block_header)*/){
            //check if it is not allocated yet
//            printf("\nfound\n");

	    if(itr->size_status%8 == 0) {return -1;}
            //free block
            itr->size_status = itr->size_status-1;
           // itr->size_status = itr->size_status-1;
	    
            done = 1;
            //traverse again for coalescing opportunities

            //block_header* newItr = list_head->next;
            //block_header* prevItr = list_head;
            while(newItr != NULL){
                if(newItr->size_status%8 == 0 && prevItr->size_status%8 == 0){
                    prevItr->size_status = prevItr->size_status + itr->size_status + sizeof(block_header) + sizeof(block_header);
                    prevItr->next = newItr->next;
                }
                newItr=newItr->next;
                prevItr=prevItr->next;
            }
            newItr = list_head->next;
            prevItr = list_head;
            while(newItr != NULL){
                if(newItr->size_status%8 == 0 && prevItr->size_status%8 == 0){
                    prevItr->size_status = prevItr->size_status + itr->size_status + sizeof(block_header) + sizeof(block_header);
                    prevItr->next = newItr->next;
                }
                newItr=newItr->next;
                prevItr=prevItr->next;
            }

            return 0;
        }
        itr = itr->next;
    }
    return -1;
}

//this function print out the maximum free memory block.
int Mem_Available()
{
	int size;
	int Max_FreeSizeBlock = 0;	
	block_header* current = list_head;
	while(current != NULL)
	{
	size = current->size_status;
	if(!(size & 1))
	{
		if(Max_FreeSizeBlock < size){
			Max_FreeSizeBlock = size;}
	}
	printf("Max free size block available is %d\n", Max_FreeSizeBlock);	
}
return 0;
}
/* Function to be used for debug */
/* Prints out a list of all the blocks along with the following information for each block */
/* No.      : Serial number of the block */
/* Status   : free/busy */
/* Begin    : Address of the first useful byte in the block */
/* End      : Address of the last byte in the block */
/* Size     : Size of the block (excluding the header) */
/* t_Size   : Size of the block (including the header) */
/* t_Begin  : Address of the first byte in the block (this is where the header starts) */
void Mem_Dump()
{
  int counter;
  block_header* current = NULL;
  char* t_Begin = NULL;
  char* Begin = NULL;
  int Size;
  int t_Size;
  char* End = NULL;
  int free_size;
  int busy_size;
  int total_size;
  char status[5];

  free_size = 0;
  busy_size = 0;
  total_size = 0;
  current = list_head;
  counter = 1;
  fprintf(stdout,"************************************Block list***********************************\n");
  fprintf(stdout,"No.\tStatus\tBegin\t\tEnd\t\tSize\tt_Size\tt_Begin\n");
  fprintf(stdout,"---------------------------------------------------------------------------------\n");
  while(current != NULL)
  {
    t_Begin = (char*)current;
    Begin = t_Begin + (int)sizeof(block_header);
    Size = current->size_status;
    strcpy(status,"Free");
    if(Size & 1) /*LSB = 1 => busy block*/
    {
      strcpy(status,"Busy");
      Size = Size - 1; /*Minus one for ignoring status in busy block*/
      t_Size = Size + (int)sizeof(block_header);
      busy_size = busy_size + t_Size;
    }
    else
    {
      t_Size = Size + (int)sizeof(block_header);
      free_size = free_size + t_Size;
    }
    End = Begin + Size;
    fprintf(stdout,"%d\t%s\t0x%08lx\t0x%08lx\t%d\t%d\t0x%08lx\n",counter,status,(unsigned long int)Begin,(unsigned long int)End,Size,t_Size,(unsigned long int)t_Begin);
    total_size = total_size + t_Size;
    current = current->next;
    counter = counter + 1;
  }
  fprintf(stdout,"---------------------------------------------------------------------------------\n");
  fprintf(stdout,"*********************************************************************************\n");

  fprintf(stdout,"Total busy size = %d\n",busy_size);
  fprintf(stdout,"Total free size = %d\n",free_size);
  fprintf(stdout,"Total size = %d\n",busy_size+free_size);
  fprintf(stdout,"*********************************************************************************\n");
  fflush(stdout);
  return;
}
