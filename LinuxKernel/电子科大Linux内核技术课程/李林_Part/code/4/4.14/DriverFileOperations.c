#include "DriverFileOperations.h"
#include "ToolFunctions.h"
#include "VMallocSpaceMangement.h"

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

void PrintPgd(void)
{
	pgd_t *pgd;
	int i;

	pgd = current->mm->pgd;

	DEBUG_PRINT(DEVICE_NAME " pgd = 0x%lx\n", (unsigned long)pgd);

	for(i = 0; i < 512; i++, pgd++)
	{
		if(pgd->pgd == 0)
			continue;

		DEBUG_PRINT(DEVICE_NAME " index 0x%x = 0x%lx\n", i, pgd->pgd);
	}
}

void PrintInit_level4_pgt(void)
{
	pgd_t *pgd;
	int i;

	pgd = (pgd_t *)kallsyms_lookup_name("init_level4_pgt");

	DEBUG_PRINT(DEVICE_NAME " init_level4_pgt = 0x%lx\n", (unsigned long)pgd);

	for(i = 0; i < 512; i++, pgd++)
	{
		if(pgd->pgd == 0)
			continue;

		DEBUG_PRINT(DEVICE_NAME " index 0x%x = 0x%lx\n", i, pgd->pgd);
	}
}

ssize_t DriverWrite(struct file *pslFileStruct, const char __user *pBuffer, size_t nCount, loff_t *pOffset)
{
	unsigned long *ptr;
	char idtr[10] = {0};

	DEBUG_PRINT(DEVICE_NAME ": write invoked, do nothing\n");

	DEBUG_PRINT(DEVICE_NAME " current process:\n");
	PrintPgd();

	ptr = (unsigned long *)VMallocAtAddr(0xffffc98000000000UL, 4*4096);
	DEBUG_PRINT(DEVICE_NAME " VMallocAtAddr = 0x%lx\n", (unsigned long)ptr);

	DEBUG_PRINT(DEVICE_NAME " After VMallocAtAddr\n");

	DEBUG_PRINT(DEVICE_NAME " current process:\n");
	PrintPgd();

	DEBUG_PRINT(DEVICE_NAME " Init_level4_pgt:\n");
	PrintInit_level4_pgt();

	asm volatile ("sidt %0\n\t" : "=m"(idtr) : :);

	*ptr = 0x12345678;

	DEBUG_PRINT(DEVICE_NAME " *ptr = 0x%lx\n", *ptr);

	DEBUG_PRINT(DEVICE_NAME " After visiting\n");

	DEBUG_PRINT(DEVICE_NAME " current process:\n");
	PrintPgd();

	vfree(ptr);
	
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
