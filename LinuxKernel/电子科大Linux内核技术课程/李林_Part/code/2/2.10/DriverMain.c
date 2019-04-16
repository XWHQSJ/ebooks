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

void ConstructPageTable(unsigned long ulPhysAddr)
{
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;

	unsigned long ulAddr = (unsigned long)(__va(ulPhysAddr));

	pgd = pgd_offset(current->mm, ulAddr);
	DEBUG_PRINT(DEVICE_NAME " pgde: %lx\n", (unsigned long)pgd->pgd);

	pud = pud_offset(pgd, ulAddr);
	DEBUG_PRINT(DEVICE_NAME " pude: %lx\n", (unsigned long)pud->pud);

	pmd = pmd_offset(pud, ulAddr);
	DEBUG_PRINT(DEVICE_NAME " pmde: %lx\n", (unsigned long)pmd->pmd);

	//U/S, R/W, P, PS, D, A, G
	pmd->pmd = ulPhysAddr | 0x1e7;

	DEBUG_PRINT(DEVICE_NAME " pmde: %lx\n", (unsigned long)pmd->pmd);
}

static int DriverInitialize(void)
{
	unsigned long physaddr = 0x3c000000;
	unsigned char *addr = (unsigned char *)(__va(physaddr));

	DEBUG_PRINT(DEVICE_NAME " Initialize\n");
	
	ConstructPageTable(physaddr);

	*addr = 0x78;

	DEBUG_PRINT(DEVICE_NAME " 0x3c000000 : %x\n", *addr);

	return InitalizeCharDevice();
}

static void DriverUninitialize(void)
{
	DEBUG_PRINT(DEVICE_NAME " Uninitialize\n");

	UninitialCharDevice();
}

module_init(DriverInitialize);
module_exit(DriverUninitialize);
