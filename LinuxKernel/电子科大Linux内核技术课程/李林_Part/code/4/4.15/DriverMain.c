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

void PrintVMapAreaList(void)
{
	struct list_head *pos, *pvmap_area_list;
	struct vmap_area *pVMapArea;
	spinlock_t *pSpinLock;
	
	pvmap_area_list = (struct list_head *)kallsyms_lookup_name("vmap_area_list");
	pSpinLock = (spinlock_t *)kallsyms_lookup_name("vmap_area_lock");

	spin_lock(pSpinLock);

	list_for_each(pos, pvmap_area_list)
	{
		pVMapArea = list_entry(pos, struct vmap_area, list);
		DEBUG_PRINT(DEVICE_NAME " vmap_area [0x%lx, 0x%lx), flags = 0x%lx\n", pVMapArea->va_start, pVMapArea->va_end, pVMapArea->flags);
	}

	spin_unlock(pSpinLock);
}

static int DriverInitialize(void)
{
	unsigned long *addr;

	SetPageReadAndWriteAttribute((unsigned long)DriverInitialize);
	DEBUG_PRINT(DEVICE_NAME " Initialize\n");

	addr = (unsigned long *)vmalloc(4*4096);
	DEBUG_PRINT(DEVICE_NAME " addr = 0x%lx\n", (unsigned long)addr);

	PrintVMapAreaList();

	vfree(addr);

	DEBUG_PRINT(DEVICE_NAME " After vfree\n");

	PrintVMapAreaList();

	return InitalizeCharDevice();
}

static void DriverUninitialize(void)
{
	DEBUG_PRINT(DEVICE_NAME " Uninitialize\n");

	UninitialCharDevice();
}

module_init(DriverInitialize);
module_exit(DriverUninitialize);
