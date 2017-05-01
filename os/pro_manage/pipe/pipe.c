#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int pid1,pid2;
main( )
{ 
int fd[2];
char outpipe[100],inpipe[100];
pipe(fd);                       /*创建一个管道*/
while ((pid1=fork( ))==-1);
/*lockf()函数允许将文件区域用作信号量（监视锁），或用于控制对锁定进程的访问（强制模式记录锁定）。
试图访问已锁定资源的其他进程将返回错误或进入休眠状态，直到资源解除锁定为止。
当关闭文件时，将释放进程的所有锁定，即使进程仍然有打开的文件。当进程终止时，将释放进程保留的所有锁定。*/
if(pid1==0)
  {
lockf(fd[1],1,0);
    sprintf(outpipe,"child 1 process is sending message!"); 
    /*把串放入数组outpipe中*/
    write(fd[1],outpipe,strlen(outpipe));     /*向管道写长为50字节的串*/
    lockf(fd[1],0,0);
    exit(0);
   }
else
  {
while((pid2=fork( ))==-1);
    if(pid2==0)
{ 
lockf(fd[1],1,0);           /*互斥*/
        sprintf(outpipe,"child 2 process is sending message!");
        write(fd[1],outpipe,strlen(outpipe));
        lockf(fd[1],0,0);
        exit(0);
     }
     else
     {  
		while(-1!=wait(0));              /*同步*/
         //read(fd[0],inpipe,50);   /*从管道中读长为50字节的串*/
         //printf("%s\n",inpipe);
         //wait(0);
         read(fd[0],inpipe,100);
         printf("%s\n",inpipe);
        exit(0);
    }
  }
}
