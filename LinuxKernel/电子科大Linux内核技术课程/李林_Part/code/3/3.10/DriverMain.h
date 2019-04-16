#ifndef DriverMain_H
#define DriverMain_H

#include <linux/init.h>
#include <linux/module.h>
#include <asm/mtrr.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/kallsyms.h>
#include <linux/memblock.h>

#define DEVICE_NAME "msg_printer"

struct SLDriverParameters
{
	struct class *pslDriverClass;
	dev_t uiDeviceNumber;
	struct cdev slCharDevice;
};

extern struct SLDriverParameters gslDriverParameters;

#endif
