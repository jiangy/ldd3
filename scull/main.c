#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>

#include "scull.h"

static int scull_major = SCULL_MAJOR;
static int scull_minor = SCULL_MINOR;
static int scull_nr_devs = SCULL_NR_DEVS;
module_param(scull_major, int, S_IRUGO);
module_param(scull_minor, int, S_IRUGO);
module_param(scull_nr_devs, int, S_IRUGO);

static int __init scull_init(void)
{
    int ret;
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

    return 0;
}

static void __exit scull_exit(void)
{
    dev_t dev = MKDEV(scull_major, scull_minor);
    unregister_chrdev_region(dev, scull_nr_devs);
}

module_init(scull_init);
module_exit(scull_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Jiang Yong <jiangyoun.hn@gmail.com>");
MODULE_DESCRIPTION("the scull module within chapter 3");
