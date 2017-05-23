#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include"queue.c"


//优先级非抢占式调度算法　Ｃ语言实现

#define PRIO_STEP 2
#define PRO_LEN	10


int time_slice=2;
int unused_cpu=0;
PNode * ready_queue;
PCB  p_cb[PRO_LEN];
int process_count=0;
int current_time=0;
char process_change[50][20];
int  count=0;
int  change_time[50];



void start_state();
void display();
void insert();
PPCB  select_by_priority();
void run(PPCB p_pcb);
void dispath();
double calculate();
void sort_pcb(); 
void time_priority();
int finshed();
void dump();

void main()
{
   ready_queue=(PNode*)malloc(sizeof(PNode));
   init_queue(ready_queue);
   start_state();
   dispath();
   sort_pcb();
   display();
}



void start_state()
{
    int i,j=1;
	printf("please input the num of processes:\n");
	scanf("%d",&process_count);
	printf("please input the time slice :\n");
	scanf("%d",&time_slice);
	printf("name\tarrived_time\tserved_time\tprio\n");
	for(i=0;i<process_count;i++){
    	printf("please input the info of process %d:\n",i+1);
        scanf("%s%d%d%d",p_cb[i].name,&p_cb[i].arrive_time,&p_cb[i].cpu_time,&p_cb[i].priority);
		p_cb[i].pid=i+1;
		p_cb[i].finsh_time=-1;
		p_cb[i].start_time=-1;
	}
}



void dispath()
{
	PPCB p_pcb;

	do{
		
        insert();
		
		p_pcb=select_by_priority();
		
    	run(p_pcb);
	}while(!finshed());  
}

void sort_pcb()
{
	int i,index,j;
	PCB temp;
	for(i=0;i<process_count-1;i++){
		index=i;
		for(j=i;j<process_count;j++)
    		if(p_cb[index].finsh_time>p_cb[j].finsh_time){
                index=j;
		}
        temp=p_cb[index];
        p_cb[index]=p_cb[i];
		p_cb[i]=temp;
	}
}

void display()
{
	int i,turnaround_time;
	double sum_turnaround_time=0;
	printf("name\tarrived_time\tserved_time\tstart_time\tfinished_time\taverage_roundtime\n");
	for(i=0;i<process_count;i++){
        turnaround_time=p_cb[i].finsh_time-p_cb[i].arrive_time;
        sum_turnaround_time+=turnaround_time;
    	printf("%-11s%-12d%-12d%-11d%-7d%-10d\n",p_cb[i].name,p_cb[i].arrive_time,p_cb[i].cpu_time,p_cb[i].start_time,p_cb[i].finsh_time,turnaround_time);

	}
	printf("average round time%.4f\n",sum_turnaround_time/process_count);
    printf("\nthe usage of cpu%.4f\n",calculate());

	printf("\nthe details of context\n\n");
	for(i=0;i<count;i++){
		printf("at time:%d\t %s get the cpu \n",change_time[i],process_change[i]);
	}
}

void insert()
{
	int i;
	for(i=0;i<process_count;i++){
		if(p_cb[i].arrive_time==current_time&&p_cb[i].state==0){
            p_cb[i].state=1;
		    en_queue(ready_queue,&p_cb[i]);
		}
	}
}


PPCB  select_by_priority()
{

	int id;
	int priority;
	PNode head,p;
	PPCB p_pcb=(PPCB)malloc(sizeof(PCB));
	int temp_id;

	if(!queue_empty(ready_queue)){
    	head=(*ready_queue)->next->next;
    	priority=head->p_cb.priority;
		temp_id=head->p_cb.pid;
        while(head!=(*ready_queue)->next){
	    	if(head->p_cb.priority>priority){ 
		    	priority=head->p_cb.priority;
		    	id=head->p_cb.pid;
				temp_id=id;
			}else if(head->p_cb.priority==priority){
				id=temp_id;
			}
            head=head->next; 
		}
    	head=(*ready_queue)->next->next;
    	p=(*ready_queue)->next;
        while(head!=(*ready_queue)->next){
	    	if(head->p_cb.pid==id){
				*p_pcb=head->p_cb;
		    	p->next=head->next;
				if((*ready_queue)->p_cb.pid==head->p_cb.pid){
				   *ready_queue=p;
				}
				free(head);
		    	break;
			}
    		head=head->next; 
     		p=p->next; 
		}
		p_pcb->state=2;
		if(p_pcb->start_time==-1)
    		p_pcb->start_time=current_time;
    	return p_pcb;
	}else
		return NULL;
}

void run(PPCB p_pcb)
{
	int i;
	if(p_pcb==NULL){
        current_time++; 
		unused_cpu++;
		return;
	}

//show the context changing process	
	strcpy(process_change[count],p_pcb->name);
	change_time[count++]=current_time;

	for(i=0;i<time_slice;i++){
        current_time++;
        if(i!=time_slice)
		insert();
		p_pcb->used_time++;
		if(p_pcb->used_time==p_pcb->cpu_time){
			p_pcb->finsh_time=current_time;
			break;
		}
	}
	time_priority(p_pcb);
	if(i==time_slice){
		p_pcb->state=1;
		insert();
        en_queue(ready_queue,p_pcb);  
	}
	else{
		p_pcb->state=3;
        p_cb[p_pcb->pid-1]=*p_pcb;
		free(p_pcb);
	}
}


int finshed()
{
	int i;
	for(i=0;i<process_count;i++)
		if(p_cb[i].state!=3)
			return 0;
	return 1;
}



double calculate()
{
    return 1.0*(current_time-unused_cpu)/current_time;
}


void time_priority(PPCB p_pcb){
	PNode head;
	head=(*ready_queue)->next->next;
	p_pcb->priority-=PRIO_STEP;
	while(head!=(*ready_queue)->next){
		printf("pid %d priority %d\n",head->p_cb.pid,head->p_cb.priority);
		if(head->p_cb.state==1)
			head->p_cb.priority+=PRIO_STEP;
		head=head->next;
	}
}


/*

测试程序使用的实例：
	Ａ　２　３　０
	Ｂ　　１　２　０
	Ｃ　３　２　２
	Ｄ　３　１　０　


	Ａ　２　３　２
	Ｂ　　１　２　０　＃
	Ｃ　３　２　４
	Ｄ　３　１　２

	Ａ　２　３　４
	Ｂ　　１　２　０　＃
	Ｃ　３　２　２　＃
	Ｄ　３　１　４　

	Ａ　２　３　２　
	Ｂ　　１　２　２　＃
	Ｃ　３　２　２　＃
	Ｄ　３　１　６　　

Ａ　２　３　２　＃
	Ｂ　　１　２　４　＃　　
	Ｃ　３　２　２　＃
	Ｄ　３　１　４　＃

　


*/
