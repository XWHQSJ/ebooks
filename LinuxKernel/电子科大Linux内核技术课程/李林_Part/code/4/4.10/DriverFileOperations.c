#include "DriverFileOperations.h"
#include "ToolFunctions.h"

int DriverOpen(struct inode *pslINode, struct file *pslFileStruct)
{
	DEBUG_PRINT(DEVICE_NAME ": open invoked, do nothing\n");
	return 0;
}

int DriverClose(struct inode *pslINode, struct file *pslFileStruct)
{
	DEBUG_PRINT(DEVICE_NAME ": close invoked, do nothing\n");
	return 0;
}

ssize_t DriverRead(struct file *pslFileStruct, char __user *pBuffer, size_t nCount, loff_t *pOffset)
{
	DEBUG_PRINT(DEVICE_NAME ": read invoked, do nothing\n");
	return 0;
}

ssize_t DriverWrite(struct file *pslFileStruct, const char __user *pBuffer, size_t nCount, loff_t *pOffset)
{
	pgd_t *pgd;
	int i;

	DEBUG_PRINT(DEVICE_NAME ": write invoked, do nothing\n");

	pgd = current->mm->pgd;

	DEBUG_PRINT(DEVICE_NAME " pgd = 0x%lx\n", (unsigned long)pgd);

	for(i = 0; i < 512; i++, pgd++)
	{
		if(pgd->pgd == 0)
			continue;

		DEBUG_PRINT(DEVICE_NAME " index 0x%x = 0x%lx\n", i, pgd->pgd);
	}

	return 0;
}

long DriverIOControl(struct file *pslFileStruct, unsigned int uiCmd, unsigned long ulArg)
{
	DEBUG_PRINT(DEVICE_NAME ": ioctl invoked, do nothing\n");
	return 0;
}

int DriverMMap(struct file *pslFileStruct, struct vm_area_struct *pslVirtualMemoryArea)
{
	DEBUG_PRINT(DEVICE_NAME ": mmap invoked, do nothing\n");
	return 0;
}
