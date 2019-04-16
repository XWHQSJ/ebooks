#include "DriverMain.h"
#include "DriverFileOperations.h"
#include "ToolFunctions.h"

MODULE_LICENSE("Dual BSD/GPL");

struct SLDriverParameters gslDriverParameters = {0};

struct file_operations gslNvmDriverFileOperations = 
{
	.owner = THIS_MODULE,
	.open  = DriverOpen,
	.release = DriverClose,
	.read  = DriverRead,
	.write = DriverWrite,
	.unlocked_ioctl = DriverIOControl,
	.mmap = DriverMMap,
};

int InitalizeCharDevice(void)
{
	int result;
	struct device *pdevice;

	result = alloc_chrdev_region(&(gslDriverParameters.uiDeviceNumber), 0, 1, DEVICE_NAME);
	if(result < 0)
	{
		printk(KERN_ALERT DEVICE_NAME " alloc_chrdev_region error\n");
		return result;
	}

	gslDriverParameters.pslDriverClass = class_create(THIS_MODULE, DEVICE_NAME);
	if(IS_ERR(gslDriverParameters.pslDriverClass)) 
	{
		printk(KERN_ALERT DEVICE_NAME " class_create error\n");

		result = PTR_ERR(gslDriverParameters.pslDriverClass);
		goto CLASS_CREATE_ERROR;
	}

	cdev_init(&(gslDriverParameters.slCharDevice), &gslNvmDriverFileOperations);
	gslDriverParameters.slCharDevice.owner = THIS_MODULE;

	result = cdev_add(&(gslDriverParameters.slCharDevice), gslDriverParameters.uiDeviceNumber, 1);
	if(result < 0) 
	{
		printk(KERN_ALERT DEVICE_NAME " cdev_add error\n");
		goto CDEV_ADD_ERROR;
	}

	pdevice = device_create(gslDriverParameters.pslDriverClass, NULL, gslDriverParameters.uiDeviceNumber, NULL, DEVICE_NAME);
	if(IS_ERR(pdevice)) 
	{
		printk(KERN_ALERT DEVICE_NAME " device_create error\n");

		result = PTR_ERR(pdevice);
		goto DEVICE_CREATE_ERROR;
	}

	return 0;

DEVICE_CREATE_ERROR:
	cdev_del(&(gslDriverParameters.slCharDevice));

CDEV_ADD_ERROR:
	class_destroy(gslDriverParameters.pslDriverClass);

CLASS_CREATE_ERROR:
	unregister_chrdev_region(gslDriverParameters.uiDeviceNumber, 1);

	return result;
}

void UninitialCharDevice(void)
{
	device_destroy(gslDriverParameters.pslDriverClass, gslDriverParameters.uiDeviceNumber);

	cdev_del(&(gslDriverParameters.slCharDevice));

	class_destroy(gslDriverParameters.pslDriverClass);

	unregister_chrdev_region(gslDriverParameters.uiDeviceNumber, 1);
}

static int DriverInitialize(void)
{
	struct boot_params *pBootParams;
	int i;

	SetPageReadAndWriteAttribute((unsigned long)DriverInitialize);
	DEBUG_PRINT(DEVICE_NAME " Initialize\n");

	pBootParams = (struct boot_params *)kallsyms_lookup_name("boot_params");
	if(pBootParams != 0)
	{
		DEBUG_PRINT(DEVICE_NAME " e820 entry count : %lx\n", (unsigned long)(pBootParams->e820_entries));
		
		for(i = 0; i < pBootParams->e820_entries; i++)
		{
			DEBUG_PRINT(DEVICE_NAME " e820[%d].base = 0x%lx\n", i, (unsigned long)(pBootParams->e820_map[i].addr));
			DEBUG_PRINT(DEVICE_NAME " e820[%d].size = 0x%lx\n", i, (unsigned long)(pBootParams->e820_map[i].size));
			DEBUG_PRINT(DEVICE_NAME " e820[%d].type = 0x%lx\n", i, (unsigned long)(pBootParams->e820_map[i].type));
		}

		DEBUG_PRINT(DEVICE_NAME " Mem size(KB) betweeen 1MB and 4GB : %lx\n", (unsigned long)(pBootParams->alt_mem_k));
		DEBUG_PRINT(DEVICE_NAME " Mem size(KB) above 1MB : %lx\n", (unsigned long)(pBootParams->screen_info.ext_mem_k));
	}
	
	return InitalizeCharDevice();
}

static void DriverUninitialize(void)
{
	DEBUG_PRINT(DEVICE_NAME " Uninitialize\n");

	UninitialCharDevice();
}

module_init(DriverInitialize);
module_exit(DriverUninitialize);
