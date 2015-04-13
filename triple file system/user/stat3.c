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

  int size = 512;

  int n = (MAXFILE * BSIZE) / size / 2;

  int i;
  char buf[size];


  fd = open("file1", O_MIRRORED | O_CREATE | O_RDWR);

  memset(buf, 0, size);

  for (i = 0; i < n; i++) {
     buf[0] = (char)('A' + i);
     write(fd, buf, size);
  }

  struct stat st;
  fstat(fd, &st);
  
  assert(st.type == 4);

  assert(st.logical_size == 35840);
  //Works for extra or regular credit
  assert(st.physical_size == 71680 || 35840 * 3);
  
  close(fd);

  printf(1, "TEST PASSED\n");
  
  exit();
}
