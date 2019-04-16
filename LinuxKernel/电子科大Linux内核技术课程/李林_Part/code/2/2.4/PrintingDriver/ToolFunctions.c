#include "ToolFunctions.h"

void MEM_PRINT(unsigned long addr, unsigned long size)
{
    unsigned long i, j;
    unsigned char *p = (unsigned char *)addr;

    for(i = 0; i < size; )
    {
     	printk("%lx:\t", (unsigned long)p);	

	for(j = 0; j < 8; j++)
	{  
	    if(i + j >= size)
		break;

	    if(*(p+j) <= 0xf)
		printk("0%x ", *(p+j));
	    else
		printk("%x ", *(p+j));
	}

	printk("\n");

	i = i + 8;
	p = p + 8;
    }
}
