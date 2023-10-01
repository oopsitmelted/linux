/*
 * Character device drivers lab
 *
 * All tasks
 */

//#include "asm-generic/atomic-instrumented.h"
#include "asm-generic/errno-base.h"
//#include "asm/string_32.h"
#include "asm-generic/errno.h"
#include "asm-generic/fcntl.h"
#include "linux/printk.h"
#include "linux/types.h"
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/string.h>

#include "../include/so2_cdev.h"

MODULE_DESCRIPTION("SO2 character device");
MODULE_AUTHOR("SO2");
MODULE_LICENSE("GPL");

#define EXTRA
#define LOG_LEVEL	KERN_INFO

#define MY_MAJOR		42
#define MY_MINOR		0
#define NUM_MINORS		1
#define MODULE_NAME		"so2_cdev"
#define MESSAGE			"hello\n"
#define IOCTL_MESSAGE		"Hello ioctl"

#ifndef BUFSIZ
#define BUFSIZ		4096
#endif


struct so2_device_data {
	/* TODO 2: add cdev member */
  struct cdev cdev;
	/* TODO 4: add buffer with BUFSIZ elements */
  char buff[BUFSIZ];

  #ifdef EXTRA
	  /* TODO 7: extra members for home */
    wait_queue_head_t wq;
    int flag;
  #endif

	/* TODO 3: add atomic_t access variable to keep track if file is opened */
  atomic_t opened;
};

struct so2_device_data devs[NUM_MINORS];

static int so2_cdev_open(struct inode *inode, struct file *file)
{
	struct so2_device_data *data;

	/* TODO 2: print message when the device file is open. */
  pr_info("Device opened\n");

	/* TODO 3: inode->i_cdev contains our cdev struct, use container_of to obtain a pointer to so2_device_data */
  data = container_of(inode->i_cdev, struct so2_device_data, cdev);

	file->private_data = data;

#ifndef EXTRA
	/* TODO 3: return immediately if access is != 0, use atomic_cmpxchg */
  if(atomic_cmpxchg(&data->opened, 0, 1) != 0)
    return -EBUSY;
#endif

//	set_current_state(TASK_INTERRUPTIBLE);
//	schedule_timeout(10 * HZ);

	return 0;
}

static int
so2_cdev_release(struct inode *inode, struct file *file)
{
#ifndef EXTRA
	struct so2_device_data *data =
		(struct so2_device_data *) file->private_data;

	/* TODO 3: reset access variable to 0, use atomic_set */
  atomic_set(&data->opened, 0);
#endif

	/* TODO 2: print message when the device file is closed. */
  pr_info("Device released\n");
	return 0;
}

static ssize_t
so2_cdev_read(struct file *file,
		char __user *user_buffer,
		size_t size, loff_t *offset)
{
	struct so2_device_data *data =
		(struct so2_device_data *) file->private_data;
	size_t to_read = min((unsigned int)strlen(data->buff) - (unsigned int)*offset, size);

  if (to_read <= 0)
    return 0;

#ifdef EXTRA
	/* TODO 7: extra tasks for home */
  if (file->f_flags & O_NONBLOCK)
  {
    return -EWOULDBLOCK;
  }
  else {
    wait_event_interruptible(data->wq, data->flag != 0);
    to_read = 0;
  }
#endif

	/* TODO 4: Copy data->buffer to user_buffer, use copy_to_user */
  if(copy_to_user(user_buffer, data->buff + *offset, to_read)) 
     return -EFAULT;

  *offset += to_read;
	return to_read;
}

static ssize_t
so2_cdev_write(struct file *file,
		const char __user *user_buffer,
		size_t size, loff_t *offset)
{
	struct so2_device_data *data =
		(struct so2_device_data *) file->private_data;

  ssize_t len = min((unsigned int)strlen(data->buff)- (unsigned int)*offset, size);

  if (len <=0)
    return 0;

	/* TODO 5: copy user_buffer to data->buffer, use copy_from_user */
  if(copy_from_user(data->buff + *offset, user_buffer, len))
    return -EFAULT;

	/* TODO 7: extra tasks for home */
#ifdef EXTRA
  data->flag = 1;
  wake_up_interruptible(&data->wq);
#endif

  *offset += len;
	return size;
}

static long
so2_cdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct so2_device_data *data =
		(struct so2_device_data *) file->private_data;
	int ret = 0;
	int remains;

	switch (cmd) {
	/* TODO 6: if cmd = MY_IOCTL_PRINT, display IOCTL_MESSAGE */
    case MY_IOCTL_PRINT:
      {
        pr_info(IOCTL_MESSAGE);
        break;
      }

    case MY_IOCTL_SET_BUFFER:
      {
        if( copy_from_user(data->buff, (char *)arg, BUFFER_SIZE) )
          return -EFAULT;
        break;
      }

    case MY_IOCTL_GET_BUFFER:
      {
        if( copy_to_user((char *)arg, data->buff, BUFFER_SIZE) )
          return -EFAULT;
        break;
      }

    case MY_IOCTL_UP:
      {
        data->flag = 1;
        wake_up_interruptible(&data->wq);
        break;
      }

    case MY_IOCTL_DOWN:
      {
        wait_event_interruptible(data->wq, data->flag != 0);
        break;
      }


	/* TODO 7: extra tasks, for home */
	default:
		ret = -EINVAL;
	}

	return ret;
}

static const struct file_operations so2_fops = {
	.owner = THIS_MODULE,
/* TODO 2: add open and release functions */
  .open = so2_cdev_open,
  .release = so2_cdev_release,
/* TODO 4: add read function */
  .read = so2_cdev_read,
/* TODO 5: add write function */
  .write = so2_cdev_write,
/* TODO 6: add ioctl function */
  .unlocked_ioctl = so2_cdev_ioctl,
};

static int so2_cdev_init(void)
{
	int err;
	int i;

	/* TODO 1: register char device region for MY_MAJOR and NUM_MINORS starting at MY_MINOR */
  err = register_chrdev_region(MKDEV(MY_MAJOR, MY_MINOR), NUM_MINORS, "my_device_driver");
  if (err != 0)
  {
    pr_info("Error: couldn't create chardev region\n");
    return err;
  }

  pr_info("Registered chardev region\n");

	for (i = 0; i < NUM_MINORS; i++) {
#ifdef EXTRA
		/* TODO 7: extra tasks, for home */
    devs[i].flag = 0;
    init_waitqueue_head(&devs[i].wq);
#else
		/* TODO 3: set access variable to 0, use atomic_set */
    atomic_set(&devs[i].opened, 0);
#endif
		/*TODO 4: initialize buffer with MESSAGE string */
    strncpy(devs[i].buff, MESSAGE, sizeof(devs[i].buff));
		/* TODO 7: extra tasks for home */
		/* TODO 2: init and add cdev to kernel core */
    cdev_init(&devs[i].cdev, &so2_fops);
    cdev_add(&devs[i].cdev, MKDEV(MY_MAJOR, i), 1);
	}

	return 0;
}

static void so2_cdev_exit(void)
{
	int i;

	for (i = 0; i < NUM_MINORS; i++) {
		/* TODO 2: delete cdev from kernel core */
    cdev_del(&devs[i].cdev);
	}

	/* TODO 1: unregister char device region, for MY_MAJOR and NUM_MINORS starting at MY_MINOR */
  unregister_chrdev_region(MKDEV(MY_MAJOR, MY_MINOR), NUM_MINORS);
  pr_info("Unregistered chardev region\n");
}

module_init(so2_cdev_init);
module_exit(so2_cdev_exit);
