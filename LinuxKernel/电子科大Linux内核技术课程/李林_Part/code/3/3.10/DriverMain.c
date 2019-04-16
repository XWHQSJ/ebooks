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

int GetElementNumberOfList(struct list_head *head)
{
	int count;
	struct list_head *pos;

	count = 0;
	list_for_each(pos, head)
	{
		count++;
	}

	return count;
}

static int DriverInitialize(void)
{
	struct pglist_data *pNode;
	struct zone *pZone;
	int i, j, k;

	SetPageReadAndWriteAttribute((unsigned long)DriverInitialize);
	DEBUG_PRINT(DEVICE_NAME " Initialize\n");

	for(i = 0; i < MAX_NUMNODES; i++)
	{
		pNode = node_data[i];
		if(pNode == 0)
			continue;

		for(j = 0; j < MAX_NR_ZONES; j++)
		{
			pZone = &(pNode->node_zones[j]);
			if(pZone->managed_pages == 0)
				continue;

			for(k = 0; k < MAX_ORDER; k++)
			{
				DEBUG_PRINT(DEVICE_NAME " ZONE %d free_area[%d].nr_free = 0x%lx\n", j, k, pZone->free_area[k].nr_free);

				DEBUG_PRINT(DEVICE_NAME " ZONE %d free_area[%d].free_list[Unmovable] = 0x%d\n", j, k, GetElementNumberOfList(&(pZone->free_area[k].free_list[MIGRATE_UNMOVABLE])));
				DEBUG_PRINT(DEVICE_NAME " ZONE %d free_area[%d].free_list[Reclaimable] = 0x%d\n", j, k, GetElementNumberOfList(&(pZone->free_area[k].free_list[MIGRATE_RECLAIMABLE])));
				DEBUG_PRINT(DEVICE_NAME " ZONE %d free_area[%d].free_list[Movable] = 0x%d\n", j, k, GetElementNumberOfList(&(pZone->free_area[k].free_list[MIGRATE_MOVABLE])));
				DEBUG_PRINT(DEVICE_NAME " ZONE %d free_area[%d].free_list[Reserve] = 0x%d\n", j, k, GetElementNumberOfList(&(pZone->free_area[k].free_list[MIGRATE_RESERVE])));
				DEBUG_PRINT(DEVICE_NAME " ZONE %d free_area[%d].free_list[CMA] = 0x%d\n", j, k, GetElementNumberOfList(&(pZone->free_area[k].free_list[MIGRATE_CMA])));
				DEBUG_PRINT(DEVICE_NAME " ZONE %d free_area[%d].free_list[Isolate] = 0x%d\n", j, k, GetElementNumberOfList(&(pZone->free_area[k].free_list[MIGRATE_ISOLATE])));
			}
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
