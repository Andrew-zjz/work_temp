#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/list.h>

static int pid=1;
module_param(pid,int,S_IRUGO); //just have the read params

static int info_init(void)
{
	struct task_struct *task1,*task2;
	task1=pid_task(find_get_pid(pid),PIDTYPE_PID);
	if(task1){
		printk("name\t\tpid\tstate\n");
		printk("the info about the process %d:\n",pid);
		printk("%15s%10ld%10ld\n",task1->comm,task1->pid,task1->state);
		task2=task1->parent;
		printk("the info about the process's parent:\n");
		printk("%15s%10ld%10ld\n",task2->comm,task2->pid,task2->state);
		printk("the info about the process's children:\n");
		list_for_each_entry(task2,&task1->children,sibling){
			printk("%15s%10ld%10ld\n",task2->comm,task2->pid,task2->state);
		}
		printk("the info about the process's siblings:\n");
		list_for_each_entry(task2,&task1->real_parent->children,sibling){
			printk("%15s%10ld%10ld\n",task2->comm,task2->pid,task2->state);
		}
	}else{
		printk("process %d doesn't exist\n");
	}
	return 0;
}

static void info_exit(void)
{
	printk("process info module exit\n");
}

module_init(info_init);
module_exit(info_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("list the info about the process");
MODULE_AUTHOR("julyerr");
