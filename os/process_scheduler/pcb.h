#ifndef PCB_TYPE
#define PCB_TYPE

typedef struct{
   int  pid ;      
   int  state ;    
   int  cpu_time;  
   int  used_time; 
   int  arrive_time;
   int  priority;   
   int  finsh_time; 
   char name[20]; 
   int  start_time; 
}PCB,*PPCB;

#endif
