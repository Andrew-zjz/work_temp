#include<stdlib.h>
#include"queue.h"
//initial queue
void init_queue(PNode*  rear)
{
	PNode p;
	p=(PNode)malloc(sizeof(Node));
	p->next=p;
    *rear=p;
}

//delete a node at the head of queue
PPCB de_queue(PNode * rear)
{
	PNode pnode;
	PPCB p_pcb=(PPCB)malloc(sizeof(PCB));
	if(!queue_empty(rear)){
    	pnode=(*rear)->next->next;
    	*p_pcb=pnode->p_cb;
    	(*rear)->next->next=pnode->next;
		if(pnode==*rear)
			*rear=(*rear)->next;
    	free(pnode);
    	return p_pcb;
	}else
		return NULL;
}

//return the rail node's data
PPCB get_queue(PNode * rear)
{
	PPCB p_pcb=(PPCB)malloc(sizeof(PCB));
	if(!queue_empty(rear)){
		*p_pcb=(*rear)->next->next->p_cb;
    	return p_pcb;
	}else
		return NULL;
}

//insert a node at the queue's tail
void en_queue(PNode* rear,PPCB p_pcb)
{
	PNode pnode;
    PPCB pp_pcb=(PPCB)malloc(sizeof(PCB));
	*pp_pcb=*p_pcb;
	pnode=(PNode)malloc(sizeof(Node));
	pnode->next=(*rear)->next;
	pnode->p_cb=*pp_pcb;
	(*rear)->next=pnode;
	*rear=pnode;
}

//judge the queue is empty or not
int queue_empty(PNode*  rear)
{
	if((*rear)->next==*rear)
		return 1;
	else
		return 0;
}
