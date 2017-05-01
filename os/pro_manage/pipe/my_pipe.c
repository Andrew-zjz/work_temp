#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/types.h>

/*
the strlen of "child 1 get the lock and writing the pipe\n" is 42
the default size of the pipe buf is 64K
64*1024/10=6653
three child processes 
	not over:
		LOOP_TIMES 100
	over:
		LOOP_TIMES 2500
*/

int LOOP_TIMES;

char str_array[][11]={
		"aaaaaaaaaa","bbbbbbbbbb","cccccccccc"
	};

char outpipe[];

void child_process(int fd,int num){
		int i=0,bytes=0;
		sem_t *sem=NULL;
		sem=sem_open("name_sem",O_CREAT|O_RDWR,0666,1);
		if(sem==SEM_FAILED){
			perror("sem open failed\n");
			exit(-1);
		}
		for(i=0;i<LOOP_TIMES;i++){
			sem_wait(sem);
    		write(fd,str_array[num],sizeof(str_array[num]));   
			bytes+=sizeof(str_array[num]);
			printf("\t\t\t\tchild %d write %d to the pipe\n",num,bytes);
			sem_post(sem);
			sleep(0.1);
		}
		sem_close(sem);
		printf("child %d exit \n",num);
		exit(0);
}

void parent_process(fd){
	int n=0,i=0;
	 while(-1!=wait(0));         
	 while(n=read(fd,outpipe,11)){
	 	printf("%s,%d\n",outpipe,++i);
		}
	sem_unlink("name_sem");
    exit(0);
}


int main(int argc,char *argv[]){ 
	int fd[2];
	int pid;
	
	if(argc<2){
		perror("Usage:%s NWRBLKING|WRBLKING\n");
		exit(-1);
	}

	if(strcmp(argv[1],"NWRBLKING")==0) LOOP_TIMES=100;
	else if(strcmp(argv[1],"WRBLKING")==0) LOOP_TIMES=2500;
	else{perror("parameter wrong\n");exit(-1);}
	if(pipe(fd)<0){
		perror("failed to create pipe\n");
	}

	if((pid=fork())==0){
		child_process(fd[1],0);
   	}
	else if((pid=fork())==0){
		child_process(fd[1],1);
		}
	else if((pid=fork())==0){
		child_process(fd[1],2);
		}
     else{  
		parent_process(fd[0]);
		}
	return 0;
}
