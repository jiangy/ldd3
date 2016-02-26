#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

static char *whom = "world";
static int howmany =1;
module_param(howmany, int, S_IRUGO);
module_param(whom, charp, S_IRUGO);

static int __init hellop_init(void)
{
    int i;
    for(i = 0; i < howmany; i++) {
        printk(KERN_ALERT "Hello, %s\n", whom);
    }
    return 0;
}

static void __exit hellop_exit(void)
{
    int i;
    for(i = 0; i < howmany; i++) {
        printk(KERN_ALERT "Goodbye, %s\n", whom);
    }
}

module_init(hellop_init);
module_exit(hellop_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Jiang Yong <jiangyoun.hn@gmail.com>");
MODULE_DESCRIPTION("the hello world module with module parameters");
