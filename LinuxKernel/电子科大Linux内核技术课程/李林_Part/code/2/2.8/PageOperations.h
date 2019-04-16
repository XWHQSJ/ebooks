//page table definition
struct pgd_t 
{ 
	unsigned long pgd; 
};

struct pud_t
{ 
	unsigned long pud; 
};

struct pmd_t
{ 
	unsigned long pmd; 
};

struct pte_t
{ 
	unsigned long pte; 
};

//page table index
#define pgd_index(address) (((address) >> 39) & 0x1FF)

static inline unsigned long pud_index(unsigned long address)
{
	return (address >> 30) & 0x1FF;
}

static inline unsigned long pmd_index(unsigned long address)
{
	return (address >> 21) & 0x1FF;
}

static inline unsigned long pte_index(unsigned long address)
{
	return (address >> 12) & 0x1FF;
}

//page table entry addr
#define pgd_offset(mm, address) ((mm)->pgd + pgd_index((address)))

static inline unsigned long pgd_page_vaddr(pgd_t pgd)
{
	return (unsigned long)__va(pgd.pgd & 0x3ffffffff000);
}

static inline pud_t *pud_offset(pgd_t *pgd, unsigned long address)
{
	//46 bits physical address
	return (pud_t *)(__va(pgd->pgd & 0x3ffffffff000)) + pud_index(address);
}

static inline pmd_t *pmd_offset(pud_t *pud, unsigned long address)
{
	return (pmd_t *)(__va(pud->pud & 0x3ffffffff000)) + pmd_index(address);
}

static inline pte_t *pte_offset_kernel(pmd_t *pmd, unsigned long address)
{
	return (pte_t *)(__va(pmd->pmd & 0x3ffffffff000)) + pte_index(address);
}