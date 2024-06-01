/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h> // file_operations
#include "aesdchar.h"
#include "aesd-circular-buffer.h"
int aesd_major =   0; // use dynamic major
int aesd_minor =   0;
int aesd_quantum = AESD_QUANTUM;
int aesd_qset =    AESD_QSET;

MODULE_AUTHOR("Your Name Here"); /** TODO: fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");

static struct aesd_circular_buffer *aesd_buffer;
struct aesd_dev *aesd_device;

int aesd_trim(struct aesd_dev *dev)
{
	struct aesd_qset *next, *dptr;
	int qset = dev->qset;   /* "dev" is not-null */
	int i;

	for (dptr = dev->data; dptr; dptr = next) { /* all the list items */
		if (dptr->data) {
			for (i = 0; i < qset; i++)
				kfree(dptr->data[i]);
			kfree(dptr->data);
			dptr->data = NULL;
		}
		next = dptr->next;
		kfree(dptr);
	}
	dev->size = 0;
	dev->quantum = aesd_quantum;
	dev->qset = aesd_qset;
	dev->data = NULL;
	return 0;
}

int aesd_open(struct inode *inode, struct file *filp)
{
    struct aesd_dev *dev; /* device information */
    PDEBUG("open aesd_char dev");

    dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
    filp->private_data = dev;

    	if ( (filp->f_flags & O_ACCMODE) == O_WRONLY) {
            if (mutex_lock_interruptible(&dev->lock))
            {
                return -ERESTARTSYS;
            }
            aesd_trim(dev); /* ignore errors */
            mutex_unlock(&dev->lock);
        
        }
    return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");
    /**
     * TODO: handle release
     */
    return 0;
}

struct aesd_qset *aesd_follow(struct aesd_dev *dev, int n)
{
	struct aesd_qset *qs = dev->data;

        /* Allocate first qset explicitly if need be */
	if (! qs) {
		qs = dev->data = kmalloc(sizeof(struct aesd_qset), GFP_KERNEL);
		if (qs == NULL)
			return NULL;  /* Never mind */
		memset(qs, 0, sizeof(struct aesd_qset));
	}

	/* Then follow the list */
	while (n--) {
		if (!qs->next) {
			qs->next = kmalloc(sizeof(struct aesd_qset), GFP_KERNEL);
			if (qs->next == NULL)
				return NULL;  /* Never mind */
			memset(qs->next, 0, sizeof(struct aesd_qset));
		}
		qs = qs->next;
		continue;
	}
	return qs;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct aesd_dev *dev = filp->private_data;
	//struct aesd_qset *dptr;	/* the first listitem */
	//int quantum = dev->quantum, qset = dev->qset;
	//int itemsize = quantum * qset; /* how many bytes in the listitem */
	//int item, s_pos, q_pos, rest;
	ssize_t retval = 0;
	ssize_t offset_rtn=0;

    PDEBUG("read %zu bytes with offset %lld",count,*f_pos);

	if (mutex_lock_interruptible(&dev->lock))
		return -ERESTARTSYS;


	if (*f_pos >= dev->size)
		goto out;
	if (*f_pos + count > dev->size)
		count = dev->size - *f_pos;
#if 0

	/* find listitem, qset index, and offset in the quantum */
	item = (long)*f_pos / itemsize;
	rest = (long)*f_pos % itemsize;
	s_pos = rest / quantum; q_pos = rest % quantum;

	/* follow the list up to the right position (defined elsewhere) */
	dptr = aesd_follow(dev, item);

	if (dptr == NULL || !dptr->data || ! dptr->data[s_pos])
		goto out; /* don't fill holes */

	/* read only up to the end of this quantum */
	if (count > quantum - q_pos)
	{
		count = quantum - q_pos;
	}

	/* Circular buffer read function */
	struct aesd_buffer_entry *rtnentry = aesd_circular_buffer_find_entry_offset_for_fpos(aesd_buffer, s_pos, &offset_rtn);
	dptr->data[s_pos] = (void *) rtnentry->buffptr;


	if (copy_to_user(buf, dptr->data[s_pos] + q_pos, count)) {
		retval = -EFAULT;
		goto out;

	}

#endif

	/* Circular buffer read function */
	struct aesd_buffer_entry *rtnentry = aesd_circular_buffer_find_entry_offset_for_fpos(aesd_buffer, (size_t)f_pos, &offset_rtn);
	//dptr->data[s_pos] = (void *) rtnentry->buffptr;


	if (copy_to_user(buf, rtnentry->buffptr, count)) {
		retval = -EFAULT;
		goto out;

	}

	*f_pos += count;
	retval = count;

  out:
	mutex_unlock(&dev->lock);
    return retval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
	struct aesd_dev *dev = filp->private_data;
	//struct aesd_qset *dptr;
	//int quantum = dev->quantum, qset = dev->qset;
	//int itemsize = quantum * qset;
	//int item, s_pos, q_pos, rest;
	ssize_t retval = -ENOMEM; /* value used in "goto out" statements */
	struct aesd_buffer_entry *entry;

	if (mutex_lock_interruptible(&dev->lock))
		return -ERESTARTSYS;
#if 0

	/* find listitem, qset index and offset in the quantum */
	item = (long)*f_pos / itemsize;
	rest = (long)*f_pos % itemsize;
	s_pos = rest / quantum; q_pos = rest % quantum;

	/* follow the list up to the right position */
	dptr = aesd_follow(dev, item);
	if (dptr == NULL)
		goto out;
	if (!dptr->data) {
		dptr->data = kmalloc(qset * sizeof(char *), GFP_KERNEL);
		if (!dptr->data)
			goto out;
		memset(dptr->data, 0, qset * sizeof(char *));
	}
	if (!dptr->data[s_pos]) {
		dptr->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
		if (!dptr->data[s_pos])
			goto out;
	}
	/* write only up to the end of this quantum */
	if (count > quantum - q_pos)
		count = quantum - q_pos;


	if (copy_from_user(dptr->data[s_pos]+q_pos, buf, count)) {
		retval = -EFAULT;
		goto out;
	}

	aesd_circular_buffer_add_entry(aesd_buffer,dptr->data[s_pos]);

#endif

	if (copy_from_user((void *)entry->buffptr, buf, count)) {
		retval = -EFAULT;
		goto out;
	}

	aesd_circular_buffer_add_entry(aesd_buffer,entry);


	*f_pos += count;
	retval = count;

        /* update the size */
	if (dev->size < *f_pos)
		dev->size = *f_pos;

  out:
	mutex_unlock(&dev->lock);
	return retval;
}
struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
    int err, devno = MKDEV(aesd_major, aesd_minor);

    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding aesd cdev", err);
    }
    return err;
}

void aesd_cleanup_module(void)
{
    dev_t devno = MKDEV(aesd_major, aesd_minor);

    if (aesd_device)
    {
        aesd_trim(aesd_device);
        cdev_del(&aesd_device->cdev);
        kfree(aesd_device);
    }

    /**
     * TODO: cleanup AESD specific poritions here as necessary
     */

    unregister_chrdev_region(devno, 1);
}

int aesd_init_module(void)
{
    dev_t dev = 0;
    int result;
    result = alloc_chrdev_region(&dev, aesd_minor, 1,
            "aesdchar");
    aesd_major = MAJOR(dev);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }

    aesd_device = kmalloc(sizeof(struct aesd_dev), GFP_KERNEL);
	if (!aesd_device) {
		result = -ENOMEM;
		aesd_cleanup_module();
        return result;
	}
    memset(aesd_device,0,sizeof(struct aesd_dev));
    
    mutex_init(&aesd_device->lock);
    result = aesd_setup_cdev(aesd_device);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    return result;
 
}

module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
