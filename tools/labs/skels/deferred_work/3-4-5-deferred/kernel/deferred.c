/*
 * SO2 - Lab 6 - Deferred Work
 *
 * Exercises #3, #4, #5: deferred work
 *
 * Code skeleton.
 */

#include "linux/workqueue.h"
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/sched/task.h>
#include "../include/deferred.h"

#define MY_MAJOR		42
#define MY_MINOR		0
#define MODULE_NAME		"deferred"

#define TIMER_TYPE_NONE		-1
#define TIMER_TYPE_SET		0
#define TIMER_TYPE_ALLOC	1
#define TIMER_TYPE_MON		2

MODULE_DESCRIPTION("Deferred work character device");
MODULE_AUTHOR("SO2");
MODULE_LICENSE("GPL");

struct mon_proc {
	struct task_struct *task;
	struct list_head list;
};

static struct my_device_data {
	struct cdev cdev;
	/* TODO 1: add timer */
  struct timer_list timer;
	/* TODO 2: add flag */
  int timer_type;
	/* TODO 3: add work */
  struct work_struct work;
	/* TODO 4: add list for monitored processes */
	/* TODO 4: add spinlock to protect list */
} dev;

static void alloc_io(void)
{
	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(5 * HZ);
	pr_info("Yawn! I've been sleeping for 5 seconds.\n");
  pr_info("PID: %d, %s\n", current->pid, current->comm);
}

static struct mon_proc *get_proc(pid_t pid)
{
	struct task_struct *task;
	struct mon_proc *p;

	rcu_read_lock();
	task = pid_task(find_vpid(pid), PIDTYPE_PID);
	rcu_read_unlock();
	if (!task)
		return ERR_PTR(-ESRCH);

	p = kmalloc(sizeof(*p), GFP_ATOMIC);
	if (!p)
		return ERR_PTR(-ENOMEM);

	get_task_struct(task);
	p->task = task;

	return p;
}


/* TODO 3: define work handler */
void work_handler(struct work_struct *work)
{
  alloc_io();
}

static void timer_handler(struct timer_list *tl)
{
	/* TODO 1: implement timer handler */
  pr_info("PID: %d, %s\n", current->pid, current->comm);
  
	/* TODO 2: check flags: TIMER_TYPE_SET or TIMER_TYPE_ALLOC */
  switch (dev.timer_type)
  {
    case TIMER_TYPE_ALLOC:
      {
		    /* TODO 3: schedule work */
        schedule_work(&dev.work);
        break;
      }

    default:
      break;
  }
		/* TODO 4: iterate the list and check the proccess state */
			/* TODO 4: if task is dead print info ... */
			/* TODO 4: ... decrement task usage counter ... */
			/* TODO 4: ... remove it from the list ... */
			/* TODO 4: ... free the struct mon_proc */
}

static int deferred_open(struct inode *inode, struct file *file)
{
	struct my_device_data *my_data =
		container_of(inode->i_cdev, struct my_device_data, cdev);
	file->private_data = my_data;
	pr_info("[deferred_open] Device opened\n");
	return 0;
}

static int deferred_release(struct inode *inode, struct file *file)
{
	pr_info("[deferred_release] Device released\n");
	return 0;
}

static long deferred_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct my_device_data *my_data = (struct my_device_data*) file->private_data;

	pr_info("[deferred_ioctl] Command: %s\n", ioctl_command_to_string(cmd));

	switch (cmd) {
		case MY_IOCTL_TIMER_SET:
			/* TODO 2: set flag */
      dev.timer_type = TIMER_TYPE_SET;
			/* TODO 1: schedule timer */
      mod_timer(&dev.timer, jiffies + arg * HZ);
			break;
		case MY_IOCTL_TIMER_CANCEL:
			/* TODO 1: cancel timer */
      del_timer(&dev.timer);
			break;
		case MY_IOCTL_TIMER_ALLOC:
			/* TODO 2: set flag and schedule timer */
      dev.timer_type = TIMER_TYPE_ALLOC;
      mod_timer(&dev.timer, jiffies + arg * HZ);
			break;
		case MY_IOCTL_TIMER_MON:
		{
			/* TODO 4: use get_proc() and add task to list */
			/* TODO 4: protect access to list */

			/* TODO 4: set flag and schedule timer */
			break;
		}
		default:
			return -ENOTTY;
	}
	return 0;
}

struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.open = deferred_open,
	.release = deferred_release,
	.unlocked_ioctl = deferred_ioctl,
};

static int deferred_init(void)
{
	int err;

	pr_info("[deferred_init] Init module\n");
	err = register_chrdev_region(MKDEV(MY_MAJOR, MY_MINOR), 1, MODULE_NAME);
	if (err) {
		pr_info("[deffered_init] register_chrdev_region: %d\n", err);
		return err;
	}

	/* TODO 2: Initialize flag. */
  dev.timer_type = TIMER_TYPE_NONE;

	/* TODO 3: Initialize work. */
  INIT_WORK(&dev.work, work_handler);

	/* TODO 4: Initialize lock and list. */

	cdev_init(&dev.cdev, &my_fops);
	cdev_add(&dev.cdev, MKDEV(MY_MAJOR, MY_MINOR), 1);

	/* TODO 1: Initialize timer. */
  timer_setup(&dev.timer,timer_handler, 0);

	return 0;
}

static void deferred_exit(void)
{
	struct mon_proc *p, *n;

	pr_info("[deferred_exit] Exit module\n" );

	cdev_del(&dev.cdev);
	unregister_chrdev_region(MKDEV(MY_MAJOR, MY_MINOR), 1);

	/* TODO 1: Cleanup: make sure the timer is not running after exiting. */
  del_timer(&dev.timer);
	/* TODO 3: Cleanup: make sure the work handler is not scheduled. */
  cancel_work_sync(&dev.work);

	/* TODO 4: Cleanup the monitered process list */
		/* TODO 4: ... decrement task usage counter ... */
		/* TODO 4: ... remove it from the list ... */
		/* TODO 4: ... free the struct mon_proc */
}

module_init(deferred_init);
module_exit(deferred_exit);
