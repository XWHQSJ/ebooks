ifneq ($(KERNELRELEASE),)
	obj-m := PrintModule.o
	PrintModule-objs := DriverMain.o DriverFileOperations.o
EXTRA_CFLAGS := -DTEST_DEBUG -ggdb -O0
else
	KERNELDIR = /root/kernelsrc/linux-3.13
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	rm *.order *.symvers *.mod.c *.o .*.o.cmd .*.cmd .tmp_versions -rf
endif
