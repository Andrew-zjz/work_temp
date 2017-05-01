#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <string.h>
#include <time.h>

#define BUFFSIZE 1024

struct msgbuf {
	int msg_type ;
	int msg_date;
	char msg_text[BUFFSIZE];
};

int main(){
	key_t key;
	if((key=ftok(".",'a'))<0){
		perror("creating key failed\n");
		exit(1);
	}
	int qid=msgget(key,IPC_CREAT|0666);
	if(-1==qid){
		perror("creating message queue failed\n");
		exit(1);
	}
	struct msgbuf msg;
	msg.msg_type=100;
	int ret;
	while(1){
		if((ret=msgrcv(qid,&msg,sizeof(msg),msg.msg_type,0))<0){
			perror("receiving message failed\n");
			exit(1);
		}
		printf("the content of the message:%s\n",msg.msg_text);
		msg.msg_type=200;
		printf("please input the message\n");
		scanf("%s",msg.msg_text);
		msg.msg_date=system("date|cut -d' ' -f 5");
		if((ret=msgsnd(qid,&msg,sizeof(msg.msg_text),msg.msg_type))<0){
			perror("inserting message failed\n");
			exit(1);
		}
	}
	msgctl(qid,IPC_RMID,NULL);
	return 0;
}
