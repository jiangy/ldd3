#ifndef _SCULL_H
#define _SCULL_H

#include <linux/cdev.h>
#include <linux/semaphore.h>

#define SCULL_MAJOR 0
#define SCULL_MINOR 0
#define SCULL_NR_DEVS 4

#define SCULL_QSET 1000
#define SCULL_QUANTUM 4000

struct scull_qset {
    void **data;
    struct scull_qset *next;
};

struct scull_dev {
    struct scull_qset *data;
    int qset;
    int quantum;
    unsigned long size;
    struct semaphore sem;
    struct cdev cdev;
};

#undef PDEBUG
#ifdef SCULL_DEBUG
#   ifdef __KERNEL__
#       define PDEBUG(fmt, args...) printk(KERN_DEBUG "scull: " fmt, ## args)
#   else
#       define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#   endif
#else
#   define PDEBUG(fmt, args...)
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...)

#endif
