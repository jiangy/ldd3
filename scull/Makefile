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
