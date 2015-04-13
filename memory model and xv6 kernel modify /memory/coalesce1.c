/* check for coalesce free space */
#include <assert.h>
#include <stdlib.h>
#include "mem.h"

int main() {
   assert(Mem_Init(4096) == 0);
   void * ptr[5];

   ptr[0] = Mem_Alloc(80);
   assert(ptr[0] != NULL);

   ptr[1] = Mem_Alloc(80);
   assert(ptr[1] != NULL);

   ptr[2] = Mem_Alloc(80);
   assert(ptr[2] != NULL);

   ptr[3] = Mem_Alloc(80);
   assert(ptr[3] != NULL);

   while (Mem_Alloc(16) != NULL)
     ;

   Mem_Free(ptr[2]);
   Mem_Free(ptr[3]);
   Mem_Free(ptr[1]);
  Mem_free(ptr[0]);
   ptr[2] = Mem_Alloc(256);
   assert(ptr[2] != NULL);

   exit(0);
}
