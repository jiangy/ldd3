#include <linux/init.h>
#include <linux/module.h>

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/uaccess.h>

dev_t faulty_dev;
struct cdev *faulty_cdev;

ssize_t faulty_read(struct file *filp, char __user *buf, size_t count, loff_t *pos)
{
    int ret;
    char stack_buf[4];

    memset(stack_buf, 0xff, 20);
    if (count > 4) {
        count = 4;
    }
    ret = copy_to_user(buf, stack_buf, count);
    if (!ret) {
        return count;
    }
    return ret;
}
ssize_t faulty_write(struct file *filp, const char __user *buf, size_t count, loff_t *pos)
{
    *(int *)0 = 0;
    return 0;
}

struct file_operations faulty_ops = {
    .owner = THIS_MODULE,
    .read = faulty_read,
    .write = faulty_write
};

static int __init faulty_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&faulty_dev, 0, 1, "faulty");
    if (ret < 0) {
        printk(KERN_WARNING "faulty: can't alloc device number\n");
        return ret;
    }
    faulty_cdev = cdev_alloc();
    if (!faulty_cdev) {
        ret = -ENOMEM;
        goto fail;
    }
    faulty_cdev->ops = &faulty_ops;
    cdev_add(faulty_cdev, faulty_dev, 1);

    return 0;

fail:
    unregister_chrdev_region(faulty_dev, 1);
    return ret;
}
static void __exit faulty_exit(void)
{
    cdev_del(faulty_cdev);
    kfree(faulty_cdev);
    unregister_chrdev_region(faulty_dev, 1);
}

module_init(faulty_init);
module_exit(faulty_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Jiang Yong <jiangyong.hn@gmail.com>");
MODULE_DESCRIPTION("the faulty module within in chapter 4");
