# Comment/uncomment the following line to disable/enable debugging
DEBUG = y

ifeq ($(DEBUG),y)
	DEBFLAGS = -O -g -DSCULL_DEBUG
else
	DEBFLAGS = -O2
endif
ccflags-y += $(DEBFLAGS)

ifneq ($(KERNELRELEASE),)
	obj-m := scull.o
	scull-objs := main.o

else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

endif

clean:
	rm -fr *.o *.ko .*.cmd *.mod.c modules.order Module.symvers .tmp_versions
