/*
 * SO2 lab3 - task 3
 */

#include "linux/gfp.h"
#include "linux/printk.h"
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>

MODULE_DESCRIPTION("Memory processing");
MODULE_AUTHOR("SO2");
MODULE_LICENSE("GPL");

struct task_info {
	pid_t pid;
	unsigned long timestamp;
};

static struct task_info *ti1, *ti2, *ti3, *ti4;

static struct task_info *task_info_alloc(int pid)
{
	struct task_info *ti;

	/* TODO 1: allocated and initialize a task_info struct */
  ti = kmalloc(sizeof(struct task_info), GFP_KERNEL);

  if (ti != NULL)
    ti->pid = pid;

	return ti;
}

static int memory_init(void)
{
	/* TODO 2: call task_info_alloc for current pid */
  ti1 = task_info_alloc(current->pid);
  if (ti1 == NULL)
  {
    pr_info("Failed to allocate ti1\n");
    return -1;
  }

	/* TODO 2: call task_info_alloc for parent PID */
  ti2 = task_info_alloc(current->parent->pid);
  if (ti2 == NULL)
  {
    pr_info("Failed to allocate ti2\n");
    return -1;
  }

	/* TODO 2: call task_info alloc for next process PID */
  ti3 = task_info_alloc(next_task(current)->pid);
  if (ti3 == NULL)
  {
    pr_info("Failed to allocate ti3\n");
    return -1;
  }

	/* TODO 2: call task_info_alloc for next process of the next process */
  ti4 = task_info_alloc(container_of(current->tasks.next->next, struct task_struct, tasks)->pid);
  if (ti4 == NULL)
  {
    pr_info("Failed to allocate ti4\n");
    return -1;
  }

	return 0;
}

static void memory_exit(void)
{

	/* TODO 3: print ti* field values */
  printk("ti1: %d", ti1->pid);
  printk("ti2: %d", ti2->pid);
  printk("ti3: %d", ti3->pid);
  printk("ti4: %d", ti4->pid);

	/* TODO 4: free ti* structures */
  kfree(ti1);
  kfree(ti2);
  kfree(ti3);
  kfree(ti4);
}

module_init(memory_init);
module_exit(memory_exit);
