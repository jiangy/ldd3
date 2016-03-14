#include <linux/init.h>
#include <linux/module.h>

#include <linux/cdev.h>
#include <linux/completion.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/types.h>

dev_t complete_dev;
struct cdev *complete_cdev;
DECLARE_COMPLETION(comp);

ssize_t complete_read(struct file *filp, char __user *buf, size_t count, loff_t *f_ops)
{
    printk(KERN_DEBUG "process %i (%s) going to sleep\n",
            current->pid, current->comm);
    wait_for_completion(&comp);
    printk(KERN_DEBUG "awoken %i (%s)\n", current->pid, current->comm);
    return 0;
}

ssize_t complete_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_ops)
{
    printk(KERN_DEBUG "process %i (%s) awakening the readers...\n",
            current->pid, current->comm);
    complete(&comp);
    return count;
}

struct file_operations complete_fops = {
    .owner = THIS_MODULE,
    .read = complete_read,
    .write = complete_write
};


static int __init complete_init(void)
{
    int ret;
    ret = alloc_chrdev_region(&complete_dev, 0, 1, "complete");
    if (ret < 0) {
        printk(KERN_WARNING "complete: can't alloc chrdev\n");
        return ret;
    }

    complete_cdev = cdev_alloc();
    if (!complete_cdev) {
        ret = -ENOMEM;
        goto fail;
    }
    complete_cdev->ops = &complete_fops;
    ret = cdev_add(complete_cdev, complete_dev, 1);
    if (ret < 0) {
        printk(KERN_NOTICE "complete: Error adding cdev\n");
        goto fail_cdev;
    }

    return 0;

fail_cdev:
    kfree(complete_cdev);
fail:
    unregister_chrdev_region(complete_dev, 1);
    return ret;
}

static void __exit complete_exit(void)
{
    cdev_del(complete_cdev);
    kfree(complete_cdev);
    unregister_chrdev_region(complete_dev, 1);
}

module_init(complete_init);
module_exit(complete_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Jiang Yong<jiangyong.hn@gmail.com>");
MODULE_DESCRIPTION("the completion char device module within chapter 5");
