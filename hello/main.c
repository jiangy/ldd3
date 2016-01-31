#include <linux/init.h>
#include <linux/module.h>

static int __init hello_init(void)
{
    printk(KERN_ALERT "Hello, world\n");
    return 0;
}

static void __exit hello_exit(void)
{
    printk(KERN_ALERT "Goodbye, cruel wodld\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Jiang Yong <jiangyoun.hn@gmail.com>");
MODULE_DESCRIPTION("the basic hello world module");
