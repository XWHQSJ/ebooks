#include <asm/page.h>
#include <asm/special_insns.h>
#include "ToolFunctions.h"

int SetPageReadAndWriteAttribute(unsigned long ulAddress)
{
	unsigned long ulCR3;
	unsigned long ulPML4TPhysAddr;
	unsigned long ulPDPTPhysAddr;
	unsigned long ulPDTPhysAddr;
	unsigned long ulPTPhysAddr;

	unsigned long ulPML4TIndex;
	unsigned long ulPDPTIndex;
	unsigned long ulPDTIndex;
	unsigned long ulPTIndex;

	unsigned long ulPML4TE;
	unsigned long ulPDPTE;
	unsigned long ulPDTE;
	unsigned long ulPTE;
	unsigned long ulPTEAddr;

	ulPML4TIndex = (ulAddress & 0xff8000000000UL) >> 39;
	ulPDPTIndex = (ulAddress & 0x7fc0000000UL) >> 30;
	ulPDTIndex = (ulAddress & 0x3fe00000UL) >> 21;
	ulPTIndex = (ulAddress & 0x1ff000UL) >> 12;

	asm volatile("mov %%cr3, %0\n\t" : "=r" (ulCR3), "=m" (__force_order));

	ulPML4TPhysAddr = ulCR3 & 0xfffffffffffff000UL;
	ulPML4TE = *((unsigned long *)(__va(ulPML4TPhysAddr + ulPML4TIndex * 8)));

	ulPDPTPhysAddr = ulPML4TE & 0x7ffffffffffff000UL;
	ulPDPTE = *((unsigned long *)(__va(ulPDPTPhysAddr + ulPDPTIndex * 8)));

	ulPDTPhysAddr = ulPDPTE & 0x7ffffffffffff000UL;
	ulPDTE = *((unsigned long *)(__va(ulPDTPhysAddr + ulPDTIndex * 8))); 

	ulPTPhysAddr = ulPDTE & 0x7ffffffffffff000UL;
	ulPTEAddr = (unsigned long)(__va(ulPTPhysAddr + ulPTIndex * 8));
	ulPTE = *((unsigned long *)(ulPTEAddr)); 

	ulPTE = ulPTE | 0x2UL;
	*((unsigned long *)(ulPTEAddr)) = ulPTE;

	return 0;
}