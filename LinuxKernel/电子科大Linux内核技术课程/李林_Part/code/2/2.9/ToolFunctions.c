#include <linux/sched.h>
#include <linux/mm.h>
#include "ToolFunctions.h"

int SetPageReadAndWriteAttribute(unsigned long addr)
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

	return 0;
}