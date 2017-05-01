#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int threads_init(void)
{
	struct task_struct *p;
	printk("name\t\tpid\tstate\tprio\n");
	for_each_process(p)
	{
		if(p->mm==NULL){
			printk("%-15s%10ld%10ld%10ld\n",p->comm,p->pid,p->state,p->normal_prio);
		}
	}
	return 0;
}

static void threads_exit(void)
{
	printk("kernel_threads list end\n");
}

module_init(threads_init);
module_exit(threads_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("list kernel threads");
MODULE_AUTHOR("julyerr");
