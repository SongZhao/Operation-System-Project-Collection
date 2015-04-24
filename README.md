# Operation-System-Project-Collection
<pre> C practice and xv6 intro : For C practice I did a progaram that can read a file and parse the data to 
caluclate how many people have the same birthday data. and the xv6 intro is adding a new system call to xv6 OS.

Memory Model: Designed memory system and optimized it for three different kind of workload. 
              (1)Any size of memory consume
              (2)Only 8mb memory will allocated by a process
              (3)Different processes will allocate possiblly 2mb,8mb,16mb memory.

xv6 kernel modify: Implemented a lottery scheduler and  changing xv6 to support a few features virtually every modern
OS does like Null-pointer Dereference and stack rearrangement.

Server-client: Implemented a multi-threaded server and three different scheduling policies(FIFO, SFNF,SFF)

Simple Shell: Implemented a command line interpreter.

Triple File System: Modified the existing xv6 file system to have the protation from data corruption. Originally the 
xv6 file system is a raid 0 file system. First modified it to a mirror file system and then base on that created a file 
system that each file has two copies. This project is for extra points.</pre>
