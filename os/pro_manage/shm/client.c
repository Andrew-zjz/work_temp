#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/shm.h>

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
	if(shm_id<0){
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
		if(share->has_data==1){
			printf("server sent data %s\n",share->text);
			sem_wait(sem);
			strncpy(share->text,"over",BUFFER_SIZE);
			share->has_data=2;
			sem_post(sem);
			break;
			}
		sleep(1);
	}
	if(shmdt(shm)==-1){
		printf("shmdt failed\n");
		exit(-1);
	}
	sem_close(sem);
	exit(0);
}
