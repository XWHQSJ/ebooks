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
	unsigned long max_pfn, highmem_pages;

	DEBUG_PRINT(DEVICE_NAME " Initialize\n");

	DEBUG_PRINT(DEVICE_NAME " PAGE_OFFSET = 0x%lx\n", (unsigned long)PAGE_OFFSET);
	DEBUG_PRINT(DEVICE_NAME " high_memory = 0x%lx\n", (unsigned long)high_memory);
	DEBUG_PRINT(DEVICE_NAME " VMALLOC_START = 0x%lx\n", (unsigned long)VMALLOC_START);
	DEBUG_PRINT(DEVICE_NAME " VMALLOC_END = 0x%lx\n", (unsigned long)VMALLOC_END);
	DEBUG_PRINT(DEVICE_NAME " PKMAP_BASE = 0x%lx\n", (unsigned long)PKMAP_BASE);
	DEBUG_PRINT(DEVICE_NAME " FIXADDR_START = 0x%lx\n", (unsigned long)FIXADDR_START);
	DEBUG_PRINT(DEVICE_NAME " FIXADDR_TOP = 0x%lx\n", (unsigned long)FIXADDR_TOP);

	highmem_pages = *((unsigned long *)kallsyms_lookup_name("highmem_pages"));
	max_pfn = *((unsigned long *)kallsyms_lookup_name("max_pfn"));

	DEBUG_PRINT(DEVICE_NAME " max_pfn = 0x%lx\n", max_pfn);
	DEBUG_PRINT(DEVICE_NAME " highmem_pages = 0x%lx\n", highmem_pages);
	DEBUG_PRINT(DEVICE_NAME " high_memory_pfn = max_pfn - highmem_pages = 0x%lx\n", max_pfn - highmem_pages);

	return InitalizeCharDevice();
}

static void DriverUninitialize(void)
{
	DEBUG_PRINT(DEVICE_NAME " Uninitialize\n");

	UninitialCharDevice();
}

module_init(DriverInitialize);
module_exit(DriverUninitialize);
