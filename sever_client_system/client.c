/*
 * client.c: A very, very primitive HTTP client.
 * 
 * To run, try: 
 *      client www.cs.wisc.edu 80 /
 *
 * Sends one HTTP request to the specified HTTP server.
 * Prints out the HTTP response.
 *
 * CS537: For testing your server, you will want to modify this client.  
 * For example:
 * 
 * You may want to make this multi-threaded so that you can 
 * send many requests simultaneously to the server.
 *
 * You may also want to be able to request different URIs; 
 * you may want to get more URIs from the command line 
 * or read the list from a file. 
 *
 * When we test your server, we will be using modifications to this client.
 *
 */

#include "cs537.h"
#include <pthread.h>
#include <assert.h>
/*
 * Send an HTTP request for the specified file 
 */
void clientSend(int fd, char *filename)
{
  char buf[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf, "GET %s HTTP/1.1\n", filename);
  sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
  Rio_writen(fd, buf, strlen(buf));
}
  
/*
 * Read the HTTP response and print it out
 */
void clientPrint(int fd)
{
  rio_t rio;
  char buf[MAXBUF];  
  int length = 0;
  int n;
  
  Rio_readinitb(&rio, fd);

  /* Read and display the HTTP Header */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (strcmp(buf, "\r\n") && (n > 0)) {
    printf("Header: %s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);

    /* If you want to look for certain HTTP tags... */
    if (sscanf(buf, "Content-Length: %d ", &length) == 1) {
      printf("Length = %d\n", length);
    }
  }

  /* Read and display the HTTP Body */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0) {
    printf("%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }
}

struct struct_arg
{
	char* host;
	char* filename;
	int port;
	
};
	 
void *mythread(void* multi_arg) {
 struct struct_arg* temp=multi_arg;
 printf("%s\n",temp->filename);
 int clientfd = Open_clientfd(temp->host,temp->port);

  clientSend(clientfd, temp->filename);
  clientPrint(clientfd);

  Close(clientfd);
return NULL;
}

int main(int argc, char *argv[])
{
  struct struct_arg multi_arg[argc-3];
  int port;
  int clientfd;
   if (argc < 4){
    fprintf(stderr, "Usage: <host> <port> <filename1> ...\n");
    exit(1);
  }
  pthread_t p[argc-3];
  port = atoi(argv[2]);
  int i=0,rc=0;
  for(i=0;i<argc-3;i++){
	multi_arg[i].port=port;
   	multi_arg[i].host=argv[1];
	multi_arg[i].filename=argv[3+i];
	rc=pthread_create(&p[i],NULL,mythread,(void*)&multi_arg[i]);assert(rc==0);
  }
  for(i=0;i<argc-3;i++){
	rc = pthread_join(p[i], NULL); assert(rc == 0); 
  }
  exit(0);
}
