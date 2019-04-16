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
	struct mempolicy *pMemPolicy;

	DEBUG_PRINT(DEVICE_NAME ": write invoked, do nothing\n");

	DEBUG_PRINT(DEVICE_NAME " current->mempolicy: %lx\n", (unsigned long)current->mempolicy);

	if(current->mempolicy != 0)
	{
		pMemPolicy = current->mempolicy;

		DEBUG_PRINT(DEVICE_NAME " current->mempolicy.mode = %d\n", pMemPolicy->mode);
		DEBUG_PRINT(DEVICE_NAME " current->mempolicy.flags = %x\n", pMemPolicy->flags);
		DEBUG_PRINT(DEVICE_NAME " current->mempolicy.v.preferred_node = %x\n", pMemPolicy->v.preferred_node);
	}

	pMemPolicy = (struct mempolicy *)kallsyms_lookup_name("preferred_node_policy");
	if(pMemPolicy != 0)
	{
		DEBUG_PRINT(DEVICE_NAME " preferred_node_policy->mempolicy.mode = %d\n", pMemPolicy->mode);
		DEBUG_PRINT(DEVICE_NAME " preferred_node_policy->mempolicy.flags = %x\n", pMemPolicy->flags);
		DEBUG_PRINT(DEVICE_NAME " preferred_node_policy->mempolicy.v.preferred_node = %x\n", pMemPolicy->v.preferred_node);
	}

	pMemPolicy = (struct mempolicy *)kallsyms_lookup_name("default_policy");
	if(pMemPolicy != 0)
	{
		DEBUG_PRINT(DEVICE_NAME " default_policy.mempolicy.mode = %d\n", pMemPolicy->mode);
		DEBUG_PRINT(DEVICE_NAME " default_policy.mempolicy.flags = %x\n", pMemPolicy->flags);
		DEBUG_PRINT(DEVICE_NAME " default_policy.mempolicy.v.preferred_node = %x\n", pMemPolicy->v.preferred_node);
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
