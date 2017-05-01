// 进程的PDB结构：
typedef struct{
   int  pid ;      
   int  state ;    //0　进程初始化的状态， 1 就绪队列中，2　正在运行，3运行完毕
   int  cpu_time;  
   int  used_time; 　//记录已经运行的时间,判断进程时候运行完成
   int  arrive_time;	
   int  priority;   
   int  finsh_time; 
   char name[20]; 　//进程名称
   int  start_time; 
}PCB,*PPCB;

// 循环节点结构
typedef struct node
{
	PCB p_cb;
	struct node * next;
}Node,*PNode;


int time_slice=2;　//时间片信息
int unused_cpu=0;　//空转ＣＰＵ时间
PNode * ready_queue;　//就绪队列的头指针
PCB  p_cb[PRO_LEN]; //进程信息记录数组
int process_count=0;　//进程个数
int current_time=0;　//当下的时间计数器的值
//记录进程上下文的切换情况
int  change_time[50];
int  count=0;
char process_change[50][20];


//程序的整体的运行的过程
void main()
{
   ready_queue=(PNode*)malloc(sizeof(PNode));
   init_queue(ready_queue);
   start_state();　　//输入的进程组的信息
   dispath();　　//程序的调度运行过程
   sort_pcb(); //按照进程结束时间进行排序
   display();　//进程运行过程的信息输出
}

void start_state(){
	int N
	//输入的进程组的信息
	for i in N:
		scanf("%s%d%d%d",p_cb[i].name,&p_cb[i].arrive_time,&p_cb[i].cpu_time,&p_cb[i].priority);
}

void dispath()
{
	PPCB p_pcb;

	do{
		
        insert(); //该时刻判断是否有进程来临，有就插入就绪队列
		
		p_pcb=select_by_priority(); //选择最高的优先级进行运行
		
    	run(p_pcb); //运行指定的进程
	}while(!finshed());  　//判断所有的进程全部运行与否
}

void insert(){
	for i in N:
		if p_cb[i].arrive_time==current_time&&p_cb[i].state==0: 
			p_cb[i].state=1;				//调整为运行就绪状态
	    	en_queue(ready_queue,&p_cb[i]); //插入就绪队列
}

PPCB  select_by_priority()
{
	 while(head!=(*ready_queue)->next){
	 		记录进程中优先级最高的pid
	 }
	 删除循环链表中pid对应的进程
	 p_pcb->state=2;　//设置为运行状态
	 return p_pcb;
}

void run(){
	for(i=0;i<time_slice;i++){   //一个时间片中操作
        current_time++;
		insert();				//判断是否有进程新的进程达到
		p_pcb->used_time++;
		if(p_pcb->used_time==p_pcb->cpu_time){　　//如果进程在一个时间片中运行完毕，直接跳出
			p_pcb->finsh_time=current_time;
			break;
		}
	}
	
	time_priority(p_pcb);　//调整就绪队列中的各个进程的优先级

	if(i==time_slice){	//如果进程没有运行完成，重新插入就绪队列
		p_pcb->state=1;
        en_queue(ready_queue,p_pcb);  
	}
	else{
		p_pcb->state=3;
        p_cb[p_pcb->pid-1]=*p_pcb;　//更新结束进程的信息，为了后续的进程信息的输出操作
		free(p_pcb);
	}
}

































