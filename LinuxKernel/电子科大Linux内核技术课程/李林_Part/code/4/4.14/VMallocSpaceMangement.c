#include "DriverMain.h"
#include "VMallocSpaceMangement.h"

struct list_head *g_pvmap_area_list;
struct rb_root *g_pRBTree;
spinlock_t *g_pSpinLock;
struct rb_node **g_ppFreeVMapCache;
unsigned long *g_pCachedHoleSize;

void InsertVMapArea(struct vmap_area *va);
struct vmap_area *GetVMapArea(unsigned long addr, unsigned long size, gfp_t gfp_mask);
void SetupVmallocVM(struct vm_struct *vm, struct vmap_area *va, unsigned long flags, const void *caller);
void *VMallocAreaNode(struct vm_struct *area, gfp_t gfp_mask, pgprot_t prot);
struct vm_struct *GetVMAreaNode(unsigned long addr, unsigned long size, unsigned long flags, gfp_t gfp_mask, const void *caller);
void *VMallocNodeRange(unsigned long addr, unsigned long size, gfp_t gfp_mask, pgprot_t prot, const void *caller);

void VMallocAtAddrInitialize(void)
{
	g_pvmap_area_list = (struct list_head *)kallsyms_lookup_name("vmap_area_list");
	g_pRBTree = (struct rb_root *)kallsyms_lookup_name("vmap_area_root");
	g_pSpinLock = (spinlock_t *)kallsyms_lookup_name("vmap_area_lock");
	g_ppFreeVMapCache = (struct rb_node **)kallsyms_lookup_name("free_vmap_cache");
	g_pCachedHoleSize = (unsigned long *)kallsyms_lookup_name("cached_hole_size");
}

void *VMallocAtAddr(unsigned long addr, unsigned long size)
{
	return VMallocNodeRange(addr, size, GFP_KERNEL | __GFP_HIGHMEM, PAGE_KERNEL, __builtin_return_address(0));
}

void *VMallocNodeRange(unsigned long addr, unsigned long size, gfp_t gfp_mask, pgprot_t prot, const void *caller)
{
	struct vm_struct *area;
	void *addr1;
	unsigned long real_size = size;

	//分配了vm_struct结构和vmap_area结构
	area =GetVMAreaNode(addr, size, VM_ALLOC | VM_UNINITIALIZED, gfp_mask, caller); 

	//分配vm_struct结构中的pages数组和逐页面分配页框，并完成页表映射
	addr1 = VMallocAreaNode(area, gfp_mask, prot);
	if (!addr1)
		return NULL;

	smp_wmb();
	area->flags &= ~VM_UNINITIALIZED;

	kmemleak_alloc(addr1, real_size, 2, gfp_mask);

	return addr1;
}

struct vm_struct *GetVMAreaNode(unsigned long addr, unsigned long size, unsigned long flags, gfp_t gfp_mask, const void *caller)
{
	struct vmap_area *va;
	struct vm_struct *area;

	area = kzalloc_node(sizeof(*area), gfp_mask & GFP_RECLAIM_MASK, NUMA_NO_NODE);
	if (unlikely(!area))
		return NULL;

	size += PAGE_SIZE;

	va = GetVMapArea(addr, size, gfp_mask);
	if (IS_ERR(va))
	{
		kfree(area);
		return NULL;
	}

	SetupVmallocVM(area, va, flags, caller);

	return area;
}

struct vmap_area *GetVMapArea(unsigned long addr, unsigned long size, gfp_t gfp_mask)
{
	struct vmap_area *va;

	va = kmalloc_node(sizeof(struct vmap_area), gfp_mask & GFP_RECLAIM_MASK, NUMA_NO_NODE);
	if (unlikely(!va))
		return ERR_PTR(-ENOMEM);

	kmemleak_scan_area(&va->rb_node, SIZE_MAX, gfp_mask & GFP_RECLAIM_MASK);

	spin_lock(g_pSpinLock);

	*g_pCachedHoleSize = 0;
	*g_ppFreeVMapCache = NULL;

	va->va_start = addr;
	va->va_end = addr + size;
	va->flags = 0;
	InsertVMapArea(va);

	spin_unlock(g_pSpinLock);

	return va;
}

void InsertVMapArea(struct vmap_area *va)
{
	struct rb_node **p = &(g_pRBTree->rb_node);
	struct rb_node *parent = NULL;
	struct rb_node *tmp;

	while (*p) {
		struct vmap_area *tmp_va;

		parent = *p;
		tmp_va = rb_entry(parent, struct vmap_area, rb_node);
		if (va->va_start < tmp_va->va_end)
			p = &(*p)->rb_left;
		else if (va->va_end > tmp_va->va_start)
			p = &(*p)->rb_right;
	}

	rb_link_node(&va->rb_node, parent, p);
	rb_insert_color(&va->rb_node, g_pRBTree);

	tmp = rb_prev(&va->rb_node);
	if (tmp) {
		struct vmap_area *prev;
		prev = rb_entry(tmp, struct vmap_area, rb_node);
		list_add_rcu(&va->list, &prev->list);
	} else
		list_add_rcu(&va->list, g_pvmap_area_list);
}

void SetupVmallocVM(struct vm_struct *vm, struct vmap_area *va, unsigned long flags, const void *caller)
{
	spin_lock(g_pSpinLock);

	vm->flags = flags;
	vm->addr = (void *)va->va_start;
	vm->size = va->va_end - va->va_start;
	vm->caller = caller;
	va->vm = vm;
	va->flags |= 0x4;

	spin_unlock(g_pSpinLock);
}

void *VMallocAreaNode(struct vm_struct *area, gfp_t gfp_mask, pgprot_t prot)
{
	struct page **pages;
	unsigned int nr_pages, array_size, i;
	gfp_t nested_gfp = (gfp_mask & GFP_RECLAIM_MASK) | __GFP_ZERO;

	nr_pages = (area->size - PAGE_SIZE) >> PAGE_SHIFT;
	array_size = (nr_pages * sizeof(struct page *));

	area->nr_pages = nr_pages;
	pages = kmalloc_node(array_size, nested_gfp, NUMA_NO_NODE);
	area->pages = pages;

	for (i = 0; i < area->nr_pages; i++) 
	{
		struct page *page;
		gfp_t tmp_mask = gfp_mask | __GFP_NOWARN;

		page = alloc_page(tmp_mask);
		area->pages[i] = page;
	}

	map_vm_area(area, prot, &pages);

	return area->addr;
}