#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "scull.h"

static int scull_major = SCULL_MAJOR;
static int scull_minor = SCULL_MINOR;
static int scull_nr_devs = SCULL_NR_DEVS;
static int scull_qset = SCULL_QSET;
static int scull_quantum = SCULL_QUANTUM;
module_param(scull_major, int, S_IRUGO);
module_param(scull_minor, int, S_IRUGO);
module_param(scull_nr_devs, int, S_IRUGO);
module_param(scull_qset, int, S_IRUGO);
module_param(scull_quantum, int, S_IRUGO);

static struct scull_dev *scull_devices;

static void scull_trim(struct scull_dev *dev)
{
    int i, qset = dev->qset;
    struct scull_qset *dptr, *next;

    for (dptr = dev->data; dptr; dptr = next) {
        if (dptr->data) {
            for (i = 0; i < qset; i++) {
                if (dptr->data[i]) {
                    kfree(dptr->data[i]);
                }
            }
            kfree(dptr->data);
        }
        next = dptr->next;
        kfree(dptr);
    }

    dev->data = NULL;
    dev->size = 0;
    dev->qset = scull_qset;
    dev->quantum = scull_quantum;
}

int scull_open(struct inode *inode, struct file *filp)
{
    struct scull_dev *dev;

    printk(KERN_INFO "scull open is called\n");
    dev = container_of(inode->i_cdev, struct scull_dev, cdev);
    filp->private_data = dev;

    if ((filp->f_flags & O_ACCMODE) == O_WRONLY) {
        scull_trim(dev);
    }
    return 0;
}

int scull_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "scull release is called\n");
    return 0;
}

static struct scull_qset *scull_follow(struct scull_dev *dev, int item)
{
    int i;
    struct scull_qset *dptr;

    if (!dev->data) {
        dev->data = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
        if (!dev->data) {
            return NULL;
        }
        memset(dev->data, 0, sizeof(struct scull_qset));
    }
    dptr = dev->data;

    for (i = 0; i < item; i++) {
        if (!dptr->next) {
            dptr->next = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
            if (!dptr->next) {
                return NULL;
            }
            memset(dptr->next, 0, sizeof(struct scull_qset));
        }
        dptr = dptr->next;
    }
    return dptr;
}

ssize_t scull_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct scull_dev *dev = filp->private_data;
    struct scull_qset *dptr;
    int qset = dev->qset, quantum = dev->quantum;
    int itemsize = qset * quantum;
    int item, rest, s_pos, q_pos;
    ssize_t retval = 0;

    printk(KERN_INFO "scull read is called, request size: %zu\n", count);
    if (down_interruptible(&dev->sem)) {
        return -ERESTARTSYS;
    }

    if (*f_pos > dev->size) {
        goto out;
    }
    if (*f_pos + count > dev->size) {
        count = dev->size - *f_pos;
    }

    item = (long)*f_pos / itemsize;
    rest = (long)*f_pos % itemsize;
    s_pos = rest / quantum;
    q_pos = rest % quantum;

    dptr = scull_follow(dev, item);
    if (!dptr || !dptr->data || !dptr->data[s_pos]) {
        goto out;
    }
    if (q_pos + count > quantum) {
        count = quantum - q_pos;
    }

    if (copy_to_user(buf, dptr->data[s_pos]+q_pos, count)) {
        retval = -EFAULT;
        goto out;
    }
    *f_pos += count;
    retval = count;

out:
    up(&dev->sem);
    return retval;
}

ssize_t scull_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct scull_dev *dev = filp->private_data;
    struct scull_qset *dptr;
    int qset = dev->qset, quantum = dev->quantum;
    int itemsize = qset * quantum;
    int item, rest, s_pos, q_pos;
    ssize_t retval = -ENOMEM;

    printk(KERN_INFO "scull write is called, request size: %zu\n", count);
    if (down_interruptible(&dev->sem)) {
        return -ERESTARTSYS;
    }

    item = (long)*f_pos / itemsize;
    rest = (long)*f_pos % itemsize;
    s_pos = rest / quantum;
    q_pos = rest % quantum;

    if (q_pos + count > quantum) {
        count = quantum - q_pos;
    }

    dptr = scull_follow(dev, item);
    if (!dptr) {
        goto out;
    }
    if (!dptr->data) {
        dptr->data = kmalloc(qset * sizeof(char *), GFP_KERNEL);
        if (!dptr->data) {
            goto out;
        }
        memset(dptr->data, 0, qset * sizeof(char *));
    }
    if (!dptr->data[s_pos]) {
        dptr->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
        if (!dptr->data[s_pos]) {
            goto out;
        }
        memset(dptr->data[s_pos], 0, quantum);
    }
    if (copy_from_user(dptr->data[s_pos] + q_pos, buf, count)) {
        retval = EFAULT;
        goto out;
    }
    dev->size = *f_pos + count > dev->size ? *f_pos + count : dev->size;
    *f_pos += count;
    retval = count;

out:
    up(&dev->sem);
    return retval;
}

struct file_operations scull_fops ={
    .owner = THIS_MODULE,
    .open = scull_open,
    .release = scull_release,
    .read = scull_read,
    .write = scull_write,
};

static void scull_setup_cdev(struct scull_dev *dev, int index)
{
    int err, devno = MKDEV(scull_major, scull_minor + index);

    cdev_init(&dev->cdev, &scull_fops);
    dev->cdev.owner = THIS_MODULE;
    err = cdev_add(&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_NOTICE "Error %d adding scull%d", err, index);
    }
}

static int __init scull_init(void)
{
    int i, ret;
    dev_t dev;

    if (scull_major) {
        dev = MKDEV(scull_major, scull_minor);
        ret = register_chrdev_region(dev, scull_nr_devs, "scull");
    } else {
        ret = alloc_chrdev_region(&dev, scull_minor, scull_nr_devs, "scull");
        scull_major = MAJOR(dev);
    }
    if (ret) {
        printk(KERN_WARNING "scull: can't get major %d\n", scull_major);
        return ret;
    }

    scull_devices = kmalloc(scull_nr_devs * sizeof(struct scull_dev), GFP_KERNEL);
    if (!scull_devices) {
        printk(KERN_NOTICE "Allocate scull_dev error");
        ret = -ENOMEM;
        goto fail;
    }
    memset(scull_devices, 0, scull_nr_devs * sizeof(struct scull_dev));

    for (i = 0; i < scull_nr_devs; i++) {
        scull_setup_cdev(&scull_devices[i], i);
        sema_init(&scull_devices[i].sem, 1);
        scull_devices[i].qset = scull_qset;
        scull_devices[i].quantum = scull_quantum;
    }

    return 0;

fail:
    unregister_chrdev_region(dev, scull_nr_devs);
    return ret;
}

static void __exit scull_exit(void)
{
    int i;
    dev_t dev = MKDEV(scull_major, scull_minor);

    for (i = 0; i < scull_nr_devs; i++) {
        cdev_del(&scull_devices[i].cdev);
        scull_trim(&scull_devices[i]);
    }
    kfree(scull_devices);

    unregister_chrdev_region(dev, scull_nr_devs);
}

module_init(scull_init);
module_exit(scull_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Jiang Yong <jiangyoun.hn@gmail.com>");
MODULE_DESCRIPTION("the scull module within chapter 3");
