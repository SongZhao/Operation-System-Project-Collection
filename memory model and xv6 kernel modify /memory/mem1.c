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
#include<pthread.h>

pthread_mutex_t lock;
typedef unsigned char *bitmap_t;
void set_bitmap(bitmap_t b, int i)
{
	*b |=0x01<<(i&7);
//	printf("i&7 is %d, 1<<0 is %d, *b|0x01 is %x\n", i&7, 1<<0, *b );
//	printf("set %p's %d bit to 1", b, i);
}
void unset_bitmap(bitmap_t b, int i)
{
	*b &= ~(0x01<<(i&7));
}
int get_bitmap(bitmap_t b, int i)
{
//	printf("when getbitmap, this char is %x, we want to get the %dth bit\n", *b, i);      
	
	return *b&(0x01<<(i&7))? 1:0;
	
}

bitmap_t head_bit = NULL;
int x = 0;
long totalsize;
int mem_size;
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
  
  if(pthread_mutex_init(&lock, NULL)!=0)
{
	return -1;
}


 bitmap_t space_ptr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  head_bit = space_ptr;
mem_size = alloc_size;
 // *space_ptr = 0xff;
 // printf("size of this char is %lu\n", sizeof(*space_ptr));
 // printf("this char is %x\n", *space_ptr);
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
	pthread_mutex_lock(&lock);	
    bitmap_t itr = head_bit;		//get the initial pointer
    bitmap_t ptr = itr + mem_size/128;		//memory allocation will start from 33*8 byte away from the initial addr
   // int x = 0;						//     the bitmap took the first 31 byte 
    int i = 0;
	while(i< mem_size/128)			//basic unit of the bitmap is char, loop over each char
	{
		//printf("now itr comes to %dth iteration, itr is %p\n ", i, itr);
		int j = 0;		
		if(i !=( mem_size/128 - 1))	
		{ 
			//printf("itr char is %x\n", *itr);
			if(*itr != 0xff) //if char is not all zero
				while(j<8){ 			//then we check everybit in that char
					//printf("i is %d, j is %d\n", i, j);
					if(get_bitmap(itr,j) == 0){		//if find a zero bit
						set_bitmap(itr,j);
								
					
						//printf("did set 1 success? %d\n", get_bitmap(itr,j));
						ptr= (void*)ptr + j*16 + i*128;
						//printf("Alloc -------------------------------------> returned %p,  i is %d, j is %d\n", ptr,i,j);
						x++;
						return ptr;
						}		//and return corresponding pointer addr
					else{
						//printf("get_bitmap is now %d,*itr is %x, j is %d \n", get_bitmap(itr,j), *itr, j);
						j++;}}
		       else{ i++;
				itr = (void*)itr+1;
				//printf("passed cuz of 0xff");
				 continue;}
		}	
		else				//we only used half of the char for the 32th char
		{
			while(j<4){		//  so we only exam the first 4 bits	
					//printf("i isss %d, j is %d", i, j);					
					if(get_bitmap(itr,j)==0){
						set_bitmap(itr,j);
						ptr=(void*)ptr+j*16+i*128;
						x++;
						//printf("allocated number is %d\n", x);
						return ptr;}
					else
						j++;

				}	
		 }
		i++;
		//printf("increaded i is %d\n", i);
		itr = (void*)itr + 1;
	}
	pthread_mutex_unlock(&lock);   
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
	pthread_mutex_lock(&lock);
  
    if(ptr == NULL){
	printf("NULL ptr");
        return -1;
    }
    printf("------------------------------->wanna free ptr is %p\n", ptr);
    bitmap_t itr = head_bit;
    printf("--------------> the header is %p\n", itr);
    int i =( (void*)ptr-(void*)itr-32)/128;
    int x = (void*)ptr-(void*)itr;
    printf("decode x(the difference between itr and ptr) is %d\n", x);
    int j =(((void*)ptr-(void*)itr-32)%128+8)/16;
    printf("--------------------------------> decode i is %d, j is %d\n", i, j);
    itr = (void*)itr + i;
    printf("--------------------------------> correspending itr is %p\n", itr);
    //unset_bitmap(itr,j);
   	if(get_bitmap(itr,j) == 0){
		 printf("error invailid ptr\n");
     		 return -1;}
    unset_bitmap(itr,j);
    printf("success mother fker");
   pthread_mutex_unlock(&lock);
    return 0;
	
 }

//this function print out the maximum free memory block.
int Mem_Availible(){ 
return 0;
}
// Function to be used f:w
// or debug */
// Prints out a list of all the blocks along with the following information for each block */
// No.      : Serial number of the block */
// Status   : free/busy */
// Begin    : Address of the first useful byte in the block */
// End      : Address of the last byte in the block */
// Size     : Size of the block (excluding the header) */
// t_Size   : Size of the block (including the header) */
// t_Begin  : Address of the first byte in the block (this is where the header starts) */

void Mem_Dump()
{
  int counter = 0;
  bitmap_t itr = head_bit;
  while(counter < 63)
	{
	printf("%x\n", *itr);
	counter++;
	}
printf("number is %d\n", x);
	



  return;
}
