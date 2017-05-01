#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

#define  BUFFER_SIZE		512

struct message
{
	long msg_type;
	char msg_text[BUFFER_SIZE];
};

//pthread_mutex_t mutex;


sem_t sem; //the sem should be a static value instead of a pointer 
//which would cause segmafault 

void *server_process(void *arg){
	int qid;
	key_t key;
	struct message msg;
	key = ftok(".", 'c');
	if (key == -1)
	{
		perror("ftok");
		exit(1);
	}
	
	qid = msgget(key, IPC_CREAT|0666);
	if (qid == -1)
	{
		perror("msgget");
		exit(1);
	}
	printf("server Open queue %d\n", qid);
	int msgFlag;
	do
	{
		memset(msg.msg_text, 0, BUFFER_SIZE);
		sem_wait(&sem);
//		pthread_mutex_lock(&mutex);
		msgFlag=msgrcv(qid, (void*)&msg,sizeof(struct message), 0, IPC_NOWAIT);
//		pthread_mutex_unlock(&mutex);
		sem_post(&sem);
		if (msgFlag < 0) 
		{
			if(errno!=ENOMSG) {perror("msgrcv failed\n");exit(1);}
		}
		else {
		printf("The message from process %ld : %s\n", msg.msg_type, msg.msg_text);		
		}
		sleep(1);
		
	} while(strncmp(msg.msg_text, "quit", 4));

	if ((msgctl(qid, IPC_RMID, NULL)) < 0)
	{
		perror("msgctl");
		exit(1);
	}
}

void *client_process(void *arg){
	int qid;
	key_t key;
	struct message msg;
	key = ftok(".", 'c');
	if ( key == -1)
	{
		perror("ftok");
		exit(1);
	}
	
	qid = msgget(key, IPC_CREAT|0666);
	if (qid == -1)
	{
		perror("msgget");
		exit(1);
	}
	
    printf("client Open queue %d\n",qid);

    while(1)
    {
        printf("Enter some message to the queue(enter 'quit' to exit):\n");
        if ((fgets(msg.msg_text, BUFFER_SIZE, stdin)) == NULL)
        {
            puts("no message");
            exit(1);
        }

        msg.msg_type = getpid();

	sem_wait(&sem);
//	pthread_mutex_lock(&mutex);
    if ((msgsnd(qid, &msg, sizeof(struct message), 0)) < 0)
    {
        perror("message posted");
        exit(1);
    }
	sem_post(&sem);
//		pthread_mutex_unlock(&mutex);
		sleep(1);
        if (strncmp(msg.msg_text, "quit", 4) == 0)
        {
            exit(0);
        }
    }
}

int main()
{
	
	pthread_t server,client;
/*	if(pthread_mutex_init(&mutex,NULL)<0){
		perror("mutex init failed\n");
		exit(-1);
	}
*/
	if(sem_init(&sem,0,1)<0){
		perror("sem_open failed\n");
		exit(-1);
	}
	if(pthread_create(&server,NULL,server_process,NULL)<0){
		perror("create server thread failed\n");
		exit(-1);
	}
	if(pthread_create(&client,NULL,client_process,NULL)<0){
		perror("create client thread failed\n");
		exit(-1);
	}
	
	if(pthread_join(server,NULL)<0){
		perror("join server thread failed\n");
		exit(-1);
	}

	if(pthread_join(client,NULL)<0){
		perror("join client thread failed\n");
		exit(-1);
	}
//	pthread_mutex_destroy(&mutex);
	exit(0);
}
