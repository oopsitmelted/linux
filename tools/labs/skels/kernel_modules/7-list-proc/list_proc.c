#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
/* TODO: add missing headers */

MODULE_DESCRIPTION("List current processes");
MODULE_AUTHOR("Kernel Hacker");
MODULE_LICENSE("GPL");

static int my_proc_init(void)
{
	struct task_struct *p = get_current();

	/* TODO: print current process pid and its name */
	pr_info("My PID: %d, Name: %s\n", (p->pid), (p->comm));

	/* TODO: print the pid and name of all processes */
	pr_info("All Processes:\n");

	for_each_process(p)
	{
		pr_info("PID: %d, Name: %s\n", (p->pid), (p->comm));
	}

	return 0;
}

static void my_proc_exit(void)
{

	struct task_struct *p = get_current();

	/* TODO: print current process pid and its name */
	pr_info("PID: %d, Name: %s\n", (p->pid), (p->comm));
}

module_init(my_proc_init);
module_exit(my_proc_exit);
