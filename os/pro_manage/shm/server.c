#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <fcntl.h>

#define  BUFFER_SIZE		512

struct shm_struct
{
	int has_data;
	char text[BUFFER_SIZE];
};

int main()
{
	struct shm_struct *share;
	int shm_id;
	void * shm=NULL;
	shm_id=shmget((key_t)55,sizeof(struct shm_struct),0666|IPC_CREAT);
	if((shm_id<0)){
		perror("shm_get failed\n");
		exit(-1);
	}
	shm=shmat(shm_id,0,0);
	if(shm==(void*)-1){
		perror("shmat failed\n");
		exit(-1);
	}
	
	printf("\nShared Memory at %x\n",(int)shm);
	share=(struct shm_struct*)shm;
	char buf[BUFFER_SIZE+1];
	sem_t *sem=NULL;
	if((sem=sem_open("name_sem",O_CREAT|O_RDWR,0666,1))<0){
		perror("sem _open failed\n");
		exit(-1);
	}
	while(1){
		if(share->has_data==0){
			printf("please input some text\n");
			sem_wait(sem);
			fgets(buf,BUFFER_SIZE,stdin);
			strncpy(share->text,buf,BUFFER_SIZE);
			share->has_data=1;
			sem_post(sem);
		}else if(share->has_data==2){
			printf("client sent data %s\n",share->text);
			sem_wait(sem);
			share->has_data=0;
			sem_post(sem);
			break;
		}
		sleep(1);
	}
	if(shmdt(shm)==-1){
		perror("shmdt failed\n");
		exit(-1);
	}
	if(shmctl(shm_id,IPC_RMID,0)==-1){	
		printf("shmctl (IPC_RMID) failed\n");
		exit(-1);
	}
	sem_close(sem);
	sem_unlink("name_sem");
	exit(0);
}
