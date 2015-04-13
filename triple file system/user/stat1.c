#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"

// Number of copies for mirrored files.
#define NUM_MIRRORED_COPIES 2

int ppid;
#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

int
main(int argc, char *argv[])
{
  ppid = getpid();
  int fd;
  int n = 1;

  int size = 512;
  int i;
  char buf[size];


  fd = open("file1", O_CREATE | O_RDWR);

  memset(buf, 0, size);

   //printf(1, "writing file\n");
  for (i = 0; i < n; i++) {
     buf[0] = (char)('A' + i);
     write(fd, buf, size);
  }

  struct stat st;
  fstat(fd, &st);
  
  printf(1, "type is %d\n", st.type);
  printf(1, "p_size is %d\n", st.physical_size);
  printf(1, "l_size is %d\n", st.logical_size);
  assert(st.type == 2);

  assert(st.logical_size == 512);
  assert(st.physical_size == 512);
  
  close(fd);

  printf(1, "TEST PASSED\n");
  
  exit();
}
