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

void PrintMemBlockInfo(struct memblock_type *pMemBlockType)
{
	int i;

	DEBUG_PRINT(DEVICE_NAME " cnt :  %lx\n", (unsigned long)pMemBlockType->cnt);
	DEBUG_PRINT(DEVICE_NAME " max :  %lx\n", (unsigned long)pMemBlockType->max);
	DEBUG_PRINT(DEVICE_NAME " total_size :  %lx\n", (unsigned long)pMemBlockType->total_size);

	for(i = 0; i < pMemBlockType->cnt; i++)
	{
		DEBUG_PRINT(DEVICE_NAME " regions[%d].base :  %lx\n", i, (unsigned long)pMemBlockType->regions[i].base);
		DEBUG_PRINT(DEVICE_NAME " regions[%d].size :  %lx\n", i, (unsigned long)pMemBlockType->regions[i].size);
		DEBUG_PRINT(DEVICE_NAME " regions[%d].nid :  %x\n", i, pMemBlockType->regions[i].nid);
	}
}

static int DriverInitialize(void)
{
	struct memblock *pMemBlock;

	SetPageReadAndWriteAttribute((unsigned long)DriverInitialize);
	DEBUG_PRINT(DEVICE_NAME " Initialize\n");

	pMemBlock = (struct memblock *)kallsyms_lookup_name("memblock");
	if(pMemBlock != 0)
	{
		DEBUG_PRINT(DEVICE_NAME " memblock.current_limit :  %lx\n", (unsigned long)pMemBlock->current_limit);

		DEBUG_PRINT(DEVICE_NAME " memblock.memory: \n");
		PrintMemBlockInfo(&(pMemBlock->memory));

		DEBUG_PRINT(DEVICE_NAME " memblock.reserved: \n");
		PrintMemBlockInfo(&(pMemBlock->reserved));
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
