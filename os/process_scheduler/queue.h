
#ifndef NODE_TYPE
#define NODE_TYPE
#include "pcb.h"

typedef struct node
{
	PCB p_cb;
	struct node * next;
}Node,*PNode;


void init_queue(PNode* rear);
PPCB de_queue(PNode * rear);
PPCB get_queue(PNode * rear);
void en_queue(PNode* rear,PPCB p_pcb);
int queue_empty(PNode* rear);

#endif
