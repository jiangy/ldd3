ifneq ($(KERNELRELEASE),)
	obj-m := faulty.o
	faulty-objs := main.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif
