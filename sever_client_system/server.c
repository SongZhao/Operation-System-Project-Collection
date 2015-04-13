#include "cs537.h"
#include "request.h"
#include <pthread.h>
#include <stdio.h>
// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// CS537: Parse the new arguments too
void producer(int fd,struct server_t* server_reg,char* buf,int file_sz,int filename_sz){
pthread_mutex_lock(server_reg->mutex);
while (server_reg->buff_count == server_reg->buffer_sz)
	pthread_cond_wait(server_reg->empty, server_reg->mutex);
put(fd,server_reg,buf,file_sz,filename_sz);
pthread_cond_signal(server_reg->fill);
pthread_mutex_unlock(server_reg->mutex);
}

void *consumer(void* arg) {
while(1){
char* buff_ptr=0;
struct server_t* server_reg=(struct server_t*)arg;
pthread_mutex_lock(server_reg->mutex);
while (server_reg->buff_count == 0)
	pthread_cond_wait(server_reg->fill, server_reg->mutex);
int temp = get(server_reg,&buff_ptr);
pthread_cond_signal(server_reg->empty);
pthread_mutex_unlock(server_reg->mutex);
requestHandle(temp,buff_ptr);
Close(temp);
}
}


void getfileinfo(char* buf,int* file_sz,int* filename_sz){
   int is_static;
   struct stat sbuf;
   char method[MAXLINE],uri[MAXLINE],version[MAXLINE];
   char filename[MAXLINE], cgiargs[MAXLINE];
   sscanf(buf, "%s %s %s", method, uri, version);
   is_static = requestParseURI(uri, filename, cgiargs);  
   if (stat(filename, &sbuf) < 0) {
         fprintf(stderr, "Error\n");
   }
   *file_sz=sbuf.st_size;
   *filename_sz=strlen(filename);

}

int main(int argc, char *argv[])
{   
    struct server_t server_reg;
    struct sockaddr_in clientaddr;
    int listenfd, connfd,clientlen;

    
    server_reg.buff_count=0;
    server_reg.buffer_sz=0;
    server_reg.empty =malloc(sizeof(pthread_cond_t));
    server_reg.fill= malloc(sizeof(pthread_cond_t));
    server_reg.mutex = malloc(sizeof(pthread_mutex_t));
        

    getargs2(&server_reg,argc,argv);
    server_reg.buffer = malloc(sizeof(int)*server_reg.buffer_sz); 
    server_reg.valid = malloc(sizeof(int)*server_reg.buffer_sz); 
    server_reg.size_filename = malloc(sizeof(int)*server_reg.buffer_sz);
    server_reg.size_file = malloc(sizeof(int)*server_reg.buffer_sz); 
    pthread_t p[server_reg.thread_num];	
    int i=0;
    for(i=0;i<server_reg.buffer_sz;i++)
	server_reg.valid[i]=0;

    int filebuffer_size=server_reg.buffer_sz+2;
    char fileinfo_buffer[filebuffer_size][MAXLINE];
    server_reg.fileinfo =(char**) malloc(filebuffer_size*sizeof(char*));
    rio_t rio;

    i=0;
    for(i=0;i<server_reg.thread_num;i++){
	pthread_create(&p[i],NULL,consumer,&server_reg);
    }
    
    listenfd = Open_listenfd(server_reg.port);
    i=0;
    int file_sz;
    int filename_sz;
    while (1) {
	clientlen = sizeof(clientaddr);
	connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        Rio_readinitb(&rio, connfd);
        Rio_readlineb(&rio,fileinfo_buffer[i], MAXLINE);
        getfileinfo(fileinfo_buffer[i],&file_sz,&filename_sz);
        producer(connfd,&server_reg,fileinfo_buffer[i],file_sz,filename_sz);
        i=(i+1)%filebuffer_size;
    } 
} 
