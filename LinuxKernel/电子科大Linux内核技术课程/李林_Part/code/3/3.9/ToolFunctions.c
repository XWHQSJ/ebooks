#include <linux/sched.h>
#include <linux/mm.h>
#include "ToolFunctions.h"

#ifdef TEST_DEBUG

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

void SetPageReadAndWriteAttribute(unsigned long addr)
{
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	pgd = pgd_offset(current->mm, addr);

	pud = pud_offset(pgd, addr);

	pmd = pmd_offset(pud, addr);

	pte = pte_offset_kernel(pmd, addr);

	pte->pte = pte->pte | 0x2UL;
}

#endif
