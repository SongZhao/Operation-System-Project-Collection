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
void set_bitmap16(bitmap_t b)
{
	*b &=0x00;
	*b |=0x81;
//	printf("i&7 is %d, 1<<0 is %d, *b|0x01 is %x\n", i&7, 1<<0, *b );
//	printf("set %p's %d bit to 1", b, i);
}
void set_bitmap80(bitmap_t b)
{
	*b &=0x00;
	*b |=0x82;
}

void set_bitmap256(bitmap_t b)
{
	*b &=0x00;
	*b |=0x84;
}
void unset_bitmap(bitmap_t b)
{
	*b &= 0x7f;
}
/*
int get_bitmap(bitmap_t b, int i)
{
//	printf("when getbitmap, this char is %x, we want to get the %dth bit\n", *b, i);      
	
	return *b&(0x01<<(i&7))? 1:0;
	
}*/

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
  if(pthread_mutex_init(&lock,NULL) != 0)
	return -1;

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
   // bitmap_t ptr = itr + 32;		//memory allocation will start from 33*8 byte away from the initial addr
   // int x = 0;						//     the bitmap took the first 31 byte 
    int heap_end = mem_size/16;
	if(size%16 !=0){
	if(size < 16)
		size +=( 16-size%16);
	else if(size < 80)
		size +=(80 - size%80);
	else 
		size +=(256 - size%256);}
	//printf("heap_end is %d\n", heap_end);
	int i = 0;
	int n = 0;
	bitmap_t ptr = itr + heap_end;
	while(i<heap_end-1 )			//basic unit of the bitmap is char, loop over each char
	{
		//printf("now itr comes to %dth iteration, itr is %p\n ", i, itr);
				
		if(i !=( heap_end - 2 ))	
		{ 
			//printf("itr char is %x\n", *itr);
			if(*itr == 0x81){
				n += 16;
				i++;
				itr = (void*)itr+1;
				continue;
				}

			else if(*itr == 0x82){
				n += 80;
				i++;
				itr = (void*)itr+1;
				continue;
				}
			else if(*itr == 0x84){
				n += 256;
				i++;
				itr = (void*)itr+1;
				continue;
				}

			else { //if char is not all zero
				if(size == 16)
				{	
					if(n >= (mem_size - heap_end)){
						printf("x is %d when NULL reached\n", x);
						return NULL;}
					x +=16;
					
					printf("%d byte of 16 block alloced\n", x);
					printf("---------->allocated size16 ptr %p\n", (void*)ptr+n);
					
					set_bitmap16(itr);
					printf("---------->this 16 itr content is %x\n", *itr);
					return ptr =  (void*)ptr + n;
				} 
				if(size == 80)
				{
					if(n>=(mem_size - heap_end)){
							return NULL;
									}
					printf("---------->allocated size80 ptr %p\n", (void*)ptr+n);
					set_bitmap80(itr);
					printf("----------->this 80  itr content  is %x\n", *itr);
					return ptr = (void*)ptr + n;
				}
			/*	if((size == 80)&&(*itr == 0x01)){
					n+= 16;
					i++;
					itr = (void*)itr+1;
					continue;
					}*/
				if(size == 256)
				{		
					printf("---------->allocated size256 ptr %p\n", (void*)ptr+n);
				
					if (n >= (mem_size - heap_end))
						return NULL;
					//x++;
					//printf("%dth 256 block alloced\n", x);
					set_bitmap256(itr);
					printf("------------>this 256 itr content is %x\n", *itr);
					//printf("itr is %p, the value is %x\n", itr, *itr);
					return ptr = (void*)ptr + n;
				}
			}
		}
	
		else				//we only used half of the char for the 32th char
		{
			if(*itr>>7 == 0x00){
				if(size == 16)
				{
					set_bitmap16(itr);
					return ptr = (void*)ptr + n;
				}
				if(size == 80)
				{
					set_bitmap80(itr);
					return ptr = (void*)ptr + n;
				}
				if(size == 256){
					printf("Null returned?\n");
					return NULL;}
			}
			else
				return NULL;
			
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
    int diff = (void*)ptr - (void*)itr - mem_size/16;
    printf("the freaking diff is %d\n", diff);
    int done = 0;
    while(!done)
    {
	//printf("*ITR is %x, itr is %p\n", *itr, itr);
	//printf("16? %x, 80?  %x, 256? %x\n", *itr&(0x01), *itr&(0x01<<1), *itr&(0x01<<2));
	if(diff%16 == 8)
		return -1;
	else if(diff == 0){
		printf("did I get here?\n");
		done = 1;
		break;
		}
	else if((*itr&(0x01)) != 0){
		printf("diff is %d\n", diff);
		diff -=16;
		//printf("did I just -16?\n");
		itr++;
		printf("itr is %p and %x and diff is %d\n", itr, *itr, diff);
		continue;
		}

	else if((*itr&(0x01<<1)) != 0){
		if(diff<80){
			diff -=16;
			itr++;
			continue;
			}
		else{
		printf("diff is %d\n", diff);
		diff -=80;
		printf("did I do this? now diff is %d\n", diff);
		itr++;
		printf("itr is %p and %x\n", itr, *itr);
		continue;
		}}
	else if((*itr&(0x01<<2)) != 0){
		diff -=256;
		//printf("did I just -256?\n");
		itr++;
		//printf("itr is %p and %x\n", itr, *itr);
		continue;
		}

	
    }
	printf("------->chosed itr is %p, value is %x\n", itr, *itr);
    if((*itr>>7) == 0x00)
	{	printf("nah not correct, *itr>>7 = %x\n", *itr>>7);
		return -1;}
    unset_bitmap(itr);
  pthread_mutex_unlock(&lock);
    return 0;
	
 }

//this function print out the maximum free memory block.
int Mem_Availible(){ 
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