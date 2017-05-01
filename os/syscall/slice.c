SYSCALL_DEFINE4(setmynice,pid_t,pid,int,flag,int,nicevalue,void __user*,prio){
	int error = 0;
	int PRIO ;
	struct task_struct *p;
	for(p = &init_task;(p = next_task(p)) !=&init_task;){
		if(p->pid == pid){
			if(flag == 0){
			  printk("the process's nice = %d\n",task_nice(p));
			}else if(flag == 1){
				printk("the process changed to %d\n", nicevalue);
				set_user_nice(p,nicevalue);
			}else{error=-EFAULT;break;}
			PRIO=task_prio(p);
			printk("the prio of the process is %d\n",PRIO);
			if(copy_to_user(prio,(const void*)&PRIO,sizeof(int))){
				printk("copy priority value failed\n");
				error= -EFAULT;
			}
		}
	}
	return error;
}

