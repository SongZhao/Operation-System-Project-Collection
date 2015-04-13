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

typedef char *bitmap_t;
void set_bitmap(bitmap_t b, int i)
{
	b[i/8] |= 1<<(i&7);
}
void unset_bitmap(bitmap_t b, int i)
{
	b[i/8] &= ~(1<<(i&7));
}
int get_bitmap(bitmap_t b, int i)
{
	return b[i/8]&(1<<(1&7))? 1:0;
}

bitmap_t head_bit = NULL;

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
  


 bitmap_t space_ptr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  head_bit = space_ptr;
  


  if (MAP_FAILED == space_ptr)
  {
    fprintf(stderr,"Error:mem.c: mmap cannot allocate space\n");
    allocated_once = 0;
    return -1;
  }
  
  allocated_once = 1;
  

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

    bitmap_t itr = head_bit;
    bitmap_t ptr = itr + 32;
    
    int i = 0;
	while(i<63)
	{
		int j = 0;
		if(i != 62)
		{
			if(*itr != 0xff)
				while(j<7){
					if(get_bitmap(itr,j) == 0){
						set_bitmap(itr,j);
						return ptr= (void*)ptr + j*16 + i*128;}
					else
						j++;}
		       else{ i++; continue;}
		}	
		else
		{
			while(j<3){
					if(get_bitmap(itr,j)==0){
						set_bitmap(itr,j);
						return ptr=(void*)ptr+j*16+i*128;}
				}	
		 }
		i++;
		itr = (void*)itr + 1;
	}
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
    bitmap_t itr = head_bit;
    int i =( (void*)ptr-(void*)itr)/128;
    int j =(((void*)ptr-(void*)itr)%128)/16;
    itr = (void*)itr + i;
    unset_bitmap(itr,j);
    if(get_bitmap(itr,j) == 0){
      return -1;}
    return 0;
	
 }
/*
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
// Function to be used for debug */
// Prints out a list of all the blocks along with the following information for each block */
// No.      : Serial number of the block */
// Status   : free/busy */
// Begin    : Address of the first useful byte in the block */
// End      : Address of the last byte in the block */
// Size     : Size of the block (excluding the header) */
// t_Size   : Size of the block (including the header) */
// t_Begin  : Address of the first byte in the block (this is where the header starts) */
/*
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
    if(Size & 1) 
    {
      strcpy(status,"Busy");
      Size = Size - 1; 
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
}*/
