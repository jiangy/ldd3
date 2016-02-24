#ifndef _SCULL_H
#define _SCULL_H

#include <linux/cdev.h>

#define SCULL_MAJOR 0
#define SCULL_MINOR 0
#define SCULL_NR_DEVS 4

struct scull_dev {
    struct cdev cdev;
};

#endif
