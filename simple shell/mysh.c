#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>


typedef int bool;
#define TRUE 1
#define FALSE 0 
#define MAX_ARGV 1024
//char *argv[MAX_ARGV];

char cwd[MAX_ARGV];
int main(int argc, char *argv[MAX_ARGV])
{
int status;
char input[MAX_ARGV], *pre_command;
const char *token = " \t\n";



while(1)
{
printf("mysh>");


fgets(input, sizeof(input), stdin);
if(strcmp(input, "\n") != 0)
{
	pre_command = input;
	bool or = FALSE;
	bool ar = FALSE;
	bool piped = FALSE;
	char *or_sign = ">";
	char *ar_sign = ">>";
	char *piped_sign = "|";
	char *cmd1[MAX_ARGV], *cmd2[MAX_ARGV];
	bool sep_cmd = FALSE;
	if(strstr(pre_command, ar_sign)!= NULL)
	{
		ar = TRUE;
	}
		
	if(strstr(pre_command, or_sign)!= NULL)
	{
		or = TRUE;	
	}
	
	if(strstr(pre_command, piped_sign)!=NULL)
	{
		piped = TRUE;;
	}

	if(piped){
			for(argc = 0; argc < MAX_ARGV; argc++)
			{
			argv[argc] = strtok(pre_command, token);			
			if(argv[argc] == NULL)
				{
				break;
				}
			pre_command = NULL;
			}
			int i =0;
			int a =0;
			for(i = 0; i<MAX_ARGV; i++)
			{		
				if(!(strcmp(argv[i], "|") == 0))
				{
					cmd1[i] = argv[i];
					a++;

				}
				if((strcmp(argv[i], "|") == 0))
					break;
					
			}
			a = a + 1;
			cmd2[0] = argv[a];
				
			
			
		}
	else if(!piped)
	{
	for(argc = 0; argc < MAX_ARGV; argc++)
		{
			argv[argc] = strtok(pre_command, token);			
			if(argv[argc] == NULL)
				{
				break;
				}
			pre_command = NULL;
		}
	}

	
	//exit command
if(strcmp(argv[0], "exit") == 0)
{
	if(argv[1]!=NULL){
		fprintf(stderr, "Error!\n");
	}
	else
		return 0;
	
}


//cd command
else if(strcmp(argv[0], "cd") == 0)
{
	
	if(argv[1] == NULL)
	{
		//printf("changed %s", argv[1]);
		chdir(getenv("HOME"));
	}
	else
	{
		 if(chdir(argv[1]) == -1)
		 {
			fprintf(stderr, "Error!\n");
		 }	
	}
}
//display current dir
else if(strcmp(argv[0], "pwd") == 0)
{
	
	if(getcwd(cwd, sizeof(cwd)) != NULL)
		printf("%s \n", cwd);
	else
	   perror("getcwd");
}

//handle systemcall
else
	{
	
	
	pid_t pid = fork();
	
	if(pid == -1)
		{
		perror("fork");
		fprintf(stderr, "Error!\n");
		exit(0);
		}
	else if(pid == 0)
		{

		if(!or && !ar && !piped)
		{	
		if(execvp(argv[0], argv) == -1)
					{
					fprintf(stderr, "Error!\n");
					exit(0);
					}
		}
		else if(or && !ar)
		{		
			int i = 0;
			for(i = 0; i<MAX_ARGV; i++)
			{		
				if(strcmp(argv[i], ">") == 0)
				{		
				argv[i] = argv[i+2];
				argv[i+2] = argv[i+1];
				argv[i+1] = NULL;	
				int fd = open(argv[i+2], O_CREAT|O_WRONLY|O_TRUNC, 0666);
				dup2(fd, 1);
				if(execvp(argv[0], argv) == -1)
					{
					fprintf(stderr, "Error!\n");
					exit(0);					
					}
				}	
			}
		}
		else if(ar)
		{	
			int i = 0;
			for(i = 0; i<MAX_ARGV; i++)
			{		
				if(strcmp(argv[i], ">>") == 0)
				{		
				argv[i] = argv[i+2];
				argv[i+2] = argv[i+1];
				argv[i+1] = NULL;	
				int fd = open(argv[i+2], O_CREAT|O_WRONLY|O_APPEND, 0666);
				dup2(fd, 1);
				if(execvp(argv[0], argv) == -1)
					{
					fprintf(stderr, "Error!\n");
					exit(0);					
					}
				}	
			}
		}
		else if(piped)
		{	
				int fpipe[2];
				pipe(fpipe);
				pid_t pid2 = fork();
				if(pid2 == -1)
					{
						perror("fork");
						fprintf(stderr, "Error!\n");
						exit(0);
					}
				else if(pid2 == 0)
					{	
										
						close(fpipe[0]);
						dup2(fpipe[1], 1);
						if(execvp(cmd1[0], cmd1) == -1)
							{
								fprintf(stderr, "Error!\n");
								exit(0);					
							}
				
			
					}
				else
					{	
						wait(NULL);
						close(fpipe[1]);
						dup2(fpipe[0], 0);
				
						if(execvp(cmd2[0], cmd2) == -1)
						{
							fprintf(stderr, "Error!\n");
							exit(0);					
						}
					}
		}
		}
	else{
		/*
		if(piped)
		{
				close(fpipe[1]);
				dup2(fpipe[0], 0);
				
				if(execvp(cmd2[0], cmd2) == -1)
					{
					fprintf(stderr, "Error!\n");
					exit(0);					
					}
				
			
		}
		*/
		wait(NULL);

	   }
	}

}
}

}


