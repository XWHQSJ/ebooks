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
	struct pglist_data *pNode;
	struct zone *pZone;
	int i, j;

	SetPageReadAndWriteAttribute((unsigned long)DriverInitialize);
	DEBUG_PRINT(DEVICE_NAME " Initialize\n");

	for(i = 0; i < MAX_NUMNODES; i++)
	{
		pNode = node_data[i];
		if(pNode == 0)
			continue;

		DEBUG_PRINT(DEVICE_NAME " NODE %d: address of node = 0x%lx\n", i, (unsigned long)pNode);

		DEBUG_PRINT(DEVICE_NAME " NODE %d: node_id = %d\n", i, pNode->node_id);
		DEBUG_PRINT(DEVICE_NAME " NODE %d: node_start_pfn = 0x%lx\n", i, pNode->node_start_pfn);
		DEBUG_PRINT(DEVICE_NAME " NODE %d: node_present_pages = 0x%lx\n", i, pNode->node_present_pages);
		DEBUG_PRINT(DEVICE_NAME " NODE %d: node_spanned_pages = 0x%lx\n", i, pNode->node_spanned_pages);
		DEBUG_PRINT(DEVICE_NAME " NODE %d: nr_zones = %d\n", i, pNode->nr_zones);

		for(j = 0; j < MAX_NR_ZONES; j++)
		{
			pZone = &(pNode->node_zones[j]);

			DEBUG_PRINT(DEVICE_NAME " ZONE %d: name = %s\n", j, pZone->name);
			DEBUG_PRINT(DEVICE_NAME " ZONE %d: node = %d\n", j, pZone->node);
			DEBUG_PRINT(DEVICE_NAME " ZONE %d: zone_pgdat = 0x%lx\n", j, (unsigned long)pZone->zone_pgdat);

			DEBUG_PRINT(DEVICE_NAME " ZONE %d: zone_start_pfn = 0x%lx\n", j, pZone->zone_start_pfn);
			DEBUG_PRINT(DEVICE_NAME " ZONE %d: spanned_pages = 0x%lx\n", j, pZone->spanned_pages);
			DEBUG_PRINT(DEVICE_NAME " ZONE %d: present_pages = 0x%lx\n", j, pZone->present_pages);
			DEBUG_PRINT(DEVICE_NAME " ZONE %d: managed_pages = 0x%lx\n", j, pZone->managed_pages);
		}
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
