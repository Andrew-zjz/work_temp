/** @file      mq_thread1.c 
 *  @brief     练习使用消息消息队列，多线程之间的通信
 *  @note      
 *  @note      
 *  @author   
 *  @date     
 *  @version   v1.0
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/msg.h>
#include <time.h>
#include <sys/ipc.h>
#include <mqueue.h>
#include <errno.h>


#define MSG_FILE "."
#define MAXBUF 255
#define PERM S_IRUSR|S_IWUSR


struct myMsg
{
    long msgType;
    char msgText[MAXBUF+1];
};


struct msqid_ds msgbuf;      // 获取消息属性的结构体


void *thr_fun1(void *arg)
{
    //printf("here is thread 1!\n");
    struct myMsg msg;
    //const int qid=*((int *)arg);
    int qid, tmpqid, err;
    tmpqid = *((int *)arg);
    qid = tmpqid;
    while (1)
    {
        qid = tmpqid;   
        /* msgrcv竟然会在出错返回时改掉 qid 的值，加const修饰都会被改为 1 
         * 太扯淡了，所以必须使用qidtmp保存第一次传进来的 qid值，
         * 以便后面消息的接收
         */
        if (-1 == (err = msgrcv(qid, &msg, sizeof(struct myMsg), 1, IPC_NOWAIT)) )
        {
            //printf("qid = %d  ", qid);
            //perror("thr1: recv msg error!");
            //printf("strerror = %s\n", strerror(errno));
            //sleep(2);
            continue;
            //pthread_exit((void *)-1);
            
            


        }
        printf("thread1:received msg \"%s\"  MsgType = %d\n", msg.msgText, msg.msgType);
        
       /*
        msg.msgType = 2;
        
        sleep(2);
        if ( -1 == (err = msgsnd(qid, &msg, sizeof(struct myMsg), 0)) )
        {
            perror("thr1: send msg error!\n");


        }
        */
        //sleep(2);
        
    }
    
    //fprintf(stdout, "thread1 receive: %s\n", msg.msgText);
    //


    


    //pthread_exit((void *)2);
}


void *thr_fun2(void *arg)
{
    //printf("here is thread 2!\n");
    struct myMsg msg;
    int qid, qidtmp;
    int err;
    qidtmp = *((int *)arg);
    while (1)
    {
        qid = qidtmp;
        if (-1 == (err = msgrcv(qid, &msg, sizeof(struct myMsg), 2, IPC_NOWAIT)) )
        {
            //perror("thr2: recv msg error!");
            //sleep(2);
            continue;
            //pthread_exit((void *)-1);
        }
        
        
        else
        {
            printf("thread2:received msg \"%s\" MsgType = %d\n", msg.msgText, msg.msgType);
        }
        /* 
        msg.msgType = 1;


        sleep(2);
        if ( -1 == (err = msgsnd(qid, &msg, sizeof(struct myMsg), 0)) )
        {
            perror("thr2: send msg error!\n");


        }
        */
        //sleep(2);
        
    }
    


    
    
    //fprintf(stdout, "thread1 receive: %s\n", msg.msgText);
    //msg.msgType = 2;


    //msgsnd(qid, &msg, sizeof(struct myMsg), 0);


    //pthread_exit((void *)2);


}


void *thr_fun3(void *arg)
{
    struct myMsg msg;
    int qid, qidtmp;
    int err;
    qidtmp = *((int *)arg);  
    while (1)
    {
        qid = qidtmp;
        //printf("thread3111111: qid = %d  ", qid);
        if (-1 == (err = msgrcv(qid, &msg, sizeof(struct myMsg), 3, IPC_NOWAIT)) )
        {
            //perror("thr3: recv msg error!");
            //sleep(2);
            continue;
            //pthread_exit((void *)-1);
        }
        else
        {
            qid = qidtmp;
            printf("thread3222222: qid = %d  ");
            err = msgctl(qid, IPC_RMID, 0);    // 删除消息队列
            if ( 0 == err )
            {
                printf("msg queue removed successfully!\n");
                printf("thread3:received msg \"%s\" MsgType = %d\n", msg.msgText, msg.msgType);
                printf("Now going to shutdown the system!\n");
                pthread_exit((void *)0);
            }
            else
            {
                perror("thread3: delete msg queue error!\n");
                exit (-1);
            }
            
        }
        /* 
        msg.msgType = 1;


        sleep(2);
        if ( -1 == (err = msgsnd(qid, &msg, sizeof(struct myMsg), 0)) )
        {
            perror("thr2: send msg error!\n");


        }
        */
        //sleep(2);
        
    
    }
}








int main(int argc, char *argv[])
{
    pthread_t td1, td2, td3;
    int err1, err2, err3, err;
    int qid;
    key_t key;


    char buf[MAXBUF+1];
    int tmpMsgType;
    
    struct myMsg msg;


    fflush(stdin);
    fflush(stdout);


    if ( -1 == (key = ftok(".", 'a')) )   // 创建key
    {
         perror("Create key error!\n");
         exit (-1);
    }
    /* 创建消息队列，若存在直接报错 */
    if ( -1 == (qid = msgget(key, IPC_CREAT | 0777)) )  // | IPC_EXCL 
    {
        perror("message queue already exitsted!\n");
        exit (-1);
    }
    printf("create . open queue qid is %d!\n", qid);
    
    err1 = pthread_create(&td1, NULL, thr_fun1, &qid);
    if ( 0 != err1 )
    {
        fprintf(stderr, "sync thread create error!\n");
        exit(1);
    }


    err2 = pthread_create(&td2, NULL, thr_fun2, &qid);
    if ( 0 != err2 )
    {
        fprintf(stderr, "fdatasync thread create error!\n");
        exit(1);
    }


    err3 = pthread_create(&td3, NULL, thr_fun3, &qid);
    if ( 0 != err3 )
    {
        fprintf(stderr, "sync thread create error!\n");
        exit(1);
    }
    
    while (1)
    {
        printf("Input the string: ");
        scanf("%s", buf);
        printf("buf = %s\n");
        fflush(stdin);
        fflush(stdout);


        strncpy(msg.msgText, buf, MAXBUF);


        printf("Input the msgType:");
        scanf("%d", &tmpMsgType);
        printf("msgType = %d\n", tmpMsgType);
        fflush(stdin);
        fflush(stdout);
        msg.msgType = tmpMsgType;
        
        if ( -1 == (msgsnd(qid, &msg, sizeof(struct myMsg), 0)) )
        {
            perror("msg closed! quit the system!\n");
            break;
            //exit (-1);
        }
        printf("Write msg success!\n");


        /*
        printf("\n---------------------------\n");
        err = msgctl(qid, IPC_STAT, &msgbuf);
        if ( err != -1 )
        {
            printf("current number of bytes on queue: %ld\n", msgbuf.msg_cbytes);
            printf("number of msg in queue is: %ld\n", msgbuf.msg_qnum);
            printf("Max number of bytes on queue is: %ld\n", msgbuf.msg_qbytes);
        }
        else
        {
            printf("msg queue msgctl error!\n");
        }
        printf("---------------------------\n"); 
        */
        sleep(2);
    }
    


    /*
    err1 = pthread_join(td1, NULL);
    if ( 0 != err1 )
    {
        printf("join thread1 failed!\n");
        exit(1);
    }


    err2 = pthread_join(td2, NULL);
    if ( 0 != err2 )
    {
        printf("join thread2 failed!\n");
        exit(1);
    }
    */
    
    err3 = pthread_join(td3, NULL);
    if ( 0 != err3 )
    {
        printf("join thread3 failed!\n");
        exit(1);
    }


    
    
    return 0;
}

