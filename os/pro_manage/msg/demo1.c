#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <string.h>
#include <time.h>

#define BUFFSIZE 1024

struct msgbuf{
	long msg_type;
	int msg_date;
	char msg_text[BUFFSIZE];
};

int main(){
	key_t key;
	if((key=ftok(".",'a'))==-1){
		perror("generating key failed\n");
		exit(1);
	}

	int qid=msgget(key,IPC_CREAT|0666);
	if(-1==qid){
		perror("create msg queue failed\n");
		exit(1);
	}
	struct msgbuf msg;
	msg.msg_type=100;
	int ret;
	while(1){
		printf("please input the message:\n");
		scanf("%s",msg.msg_text);
		msg.msg_date=system("date|cut -d' ' -f 5");
		if((ret=msgsnd(qid,&msg,sizeof(msg.msg_text),msg.msg_type))<0){
			perror("inserting message failed\n");
			exit(1);
		}
		msg.msg_type=200;
		if((ret=msgrcv(qid,&msg,sizeof(msg),msg.msg_type,0))<0){
			perror("getting the message failed\n");
			exit(1);
		}
		printf("the content of the message:\n%s\n",msg.msg_text);
	}
	msgctl(qid,IPC_RMID,NULL);
	return 0;
}
