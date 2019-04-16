#ifndef DriverFileOperations_H
#define DriverFileOperations_H

#include "DriverMain.h"

int DriverOpen(struct inode *pslINode, struct file *pslFileStruct);

int DriverClose(struct inode *pslINode, struct file *pslFileStruct);

ssize_t DriverRead(struct file *pslFileStruct, char __user *pBuffer, size_t nCount, loff_t *pOffset);

ssize_t DriverWrite(struct file *pslFileStruct, const char __user *pBuffer, size_t nCount, loff_t *pOffset);

long DriverIOControl(struct file *pslFileStruct, unsigned int uiCmd, unsigned long ulArg);

int DriverMMap(struct file *pslFileStruct, struct vm_area_struct *pslVirtualMemoryArea);

#endif
