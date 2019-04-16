#ifndef VMallocSpaceMangement_H
#define VMallocSpaceMangement_H

void *VMallocAtAddr(unsigned long addr, unsigned long size);
void VMallocAtAddrInitialize(void);

#endif