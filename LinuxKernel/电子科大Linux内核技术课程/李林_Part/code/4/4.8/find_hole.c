/**
*	vmalloc  -  allocate virtually contiguous memory
*	@size:		allocation size
*	Allocate enough pages to cover @size from the page level
*	allocator and map them into contiguous kernel virtual space.
*
*	For tight control over page level allocator and protection flags
*	use __vmalloc() instead.
*/
void *vmalloc(unsigned long size)
{
	return __vmalloc_node_flags(size, NUMA_NO_NODE, //-1
		GFP_KERNEL | __GFP_HIGHMEM);
}
EXPORT_SYMBOL(vmalloc);

static inline void *__vmalloc_node_flags(unsigned long size,
										 int node, gfp_t flags)
{
	return __vmalloc_node(size, 1, flags, PAGE_KERNEL,//对应页表项的属性 _PAGE_PRESENT | _PAGE_RW | _PAGE_DIRTY | _PAGE_ACCESSED | _PAGE_GLOBAL | _PAGE_NX
		node, __builtin_return_address(0));//获得当前函数__vmalloc_node_flags的返回地址
}

/**
*	__vmalloc_node  -  allocate virtually contiguous memory
*	@size:		allocation size
*	@align:		desired alignment
*	@gfp_mask:	flags for the page level allocator
*	@prot:		protection mask for the allocated pages
*	@node:		node to use for allocation or NUMA_NO_NODE
*	@caller:	caller's return address
*
*	Allocate enough pages to cover @size from the page level
*	allocator with @gfp_mask flags.  Map them into contiguous
*	kernel virtual space, using a pagetable protection of @prot.
*/
static void *__vmalloc_node(unsigned long size, unsigned long align, gfp_t gfp_mask, pgprot_t prot, int node, const void *caller)
{
	return __vmalloc_node_range(size, align, VMALLOC_START, VMALLOC_END, gfp_mask, prot, node, caller);
}

/**
*	__vmalloc_node_range  -  allocate virtually contiguous memory
*	@size:		allocation size
*	@align:		desired alignment
*	@start:		vm area range start
*	@end:		vm area range end
*	@gfp_mask:	flags for the page level allocator
*	@prot:		protection mask for the allocated pages
*	@node:		node to use for allocation or NUMA_NO_NODE
*	@caller:	caller's return address
*
*	Allocate enough pages to cover @size from the page level
*	allocator with @gfp_mask flags.  Map them into contiguous
*	kernel virtual space, using a pagetable protection of @prot.
*/
void *__vmalloc_node_range(unsigned long size, unsigned long align, unsigned long start, unsigned long end, gfp_t gfp_mask,
						   pgprot_t prot, int node, const void *caller)
{
	struct vm_struct *area;
	void *addr;
	unsigned long real_size = size;

	size = PAGE_ALIGN(size);
	if (!size || (size >> PAGE_SHIFT) > totalram_pages)
		goto fail;

	//分配了vm_struct结构和vmap_area结构
	area = __get_vm_area_node(size, align, VM_ALLOC | VM_UNINITIALIZED, start, end, node, gfp_mask, caller);
	if (!area)
		goto fail;

	//分配vm_struct结构中的pages数组和逐页面分配页框，并完成页表映射
	addr = __vmalloc_area_node(area, gfp_mask, prot, node);
	if (!addr)
		return NULL;

	/*
	* In this function, newly allocated vm_struct has VM_UNINITIALIZED
	* flag. It means that vm_struct is not fully initialized.
	* Now, it is fully initialized, so remove this flag here.
	*/
	clear_vm_uninitialized_flag(area);

	/*
	* A ref_count = 2 is needed because vm_struct allocated in
	* __get_vm_area_node() contains a reference to the virtual address of
	* the vmalloc'ed block.
	*/
	kmemleak_alloc(addr, real_size, 2, gfp_mask);

	return addr;

fail:
	warn_alloc_failed(gfp_mask, 0,
		"vmalloc: allocation failure: %lu bytes\n",
		real_size);
	return NULL;
}

static struct vm_struct *__get_vm_area_node(unsigned long size,
											unsigned long align, unsigned long flags, unsigned long start,
											unsigned long end, int node, gfp_t gfp_mask, const void *caller)
{
	struct vmap_area *va;
	struct vm_struct *area;

	BUG_ON(in_interrupt());
	if (flags & VM_IOREMAP)
		align = 1ul << clamp(fls(size), PAGE_SHIFT, IOREMAP_MAX_ORDER);

	size = PAGE_ALIGN(size);
	if (unlikely(!size))
		return NULL;

	area = kzalloc_node(sizeof(*area), gfp_mask & GFP_RECLAIM_MASK, node);
	if (unlikely(!area))
		return NULL;

	/*
	* We always allocate a guard page.
	*/
	//这里就是多申请的第一个页面，guard page，安全区间
	size += PAGE_SIZE;

	va = alloc_vmap_area(size, align, start, end, node, gfp_mask);
	if (IS_ERR(va)) {
		kfree(area);
		return NULL;
	}

	setup_vmalloc_vm(area, va, flags, caller);

	return area;
}

/*
* Allocate a region of KVA of the specified size and alignment, within the
* vstart and vend.
*/
static struct vmap_area *alloc_vmap_area(unsigned long size,
										 unsigned long align,
										 unsigned long vstart, unsigned long vend,
										 int node, gfp_t gfp_mask)
{
	struct vmap_area *va;
	struct rb_node *n;
	unsigned long addr;
	int purged = 0;
	struct vmap_area *first;

	BUG_ON(!size);
	BUG_ON(size & ~PAGE_MASK);
	BUG_ON(!is_power_of_2(align));

	va = kmalloc_node(sizeof(struct vmap_area), gfp_mask & GFP_RECLAIM_MASK, node);
	if (unlikely(!va))
		return ERR_PTR(-ENOMEM);

	/*
	* Only scan the relevant parts containing pointers to other objects
	* to avoid false negatives.
	*/
	kmemleak_scan_area(&va->rb_node, SIZE_MAX, gfp_mask & GFP_RECLAIM_MASK);

retry:
	spin_lock(&vmap_area_lock);
	/*
	* Invalidate cache if we have more permissive parameters.
	* cached_hole_size notes the largest hole noticed _below_
	* the vmap_area cached in free_vmap_cache: if size fits
	* into that hole, we want to scan from vstart to reuse
	* the hole instead of allocating above free_vmap_cache.
	* Note that __free_vmap_area may update free_vmap_cache
	* without updating cached_hole_size or cached_align.
	*/
	if (!free_vmap_cache ||
		size < cached_hole_size ||
		vstart < cached_vstart ||
		align < cached_align) {
nocache:
			cached_hole_size = 0;
			free_vmap_cache = NULL;
	}
	/* record if we encounter less permissive parameters */
	cached_vstart = vstart;
	cached_align = align;

	/* find starting point for our search */
	if (free_vmap_cache) {
		first = rb_entry(free_vmap_cache, struct vmap_area, rb_node);
		addr = ALIGN(first->va_end, align);
		if (addr < vstart)
			goto nocache;
		if (addr + size < addr)
			goto overflow;

	} else {
		addr = ALIGN(vstart, align);
		if (addr + size < addr)
			goto overflow;

		n = vmap_area_root.rb_node;
		first = NULL;

		while (n) {
			struct vmap_area *tmp;
			tmp = rb_entry(n, struct vmap_area, rb_node);
			if (tmp->va_end >= addr) {
				first = tmp;
				if (tmp->va_start <= addr)
					break;
				n = n->rb_left;
			} else
				n = n->rb_right;
		}

		if (!first)
			goto found;
	}

	/* from the starting point, walk areas until a suitable hole is found */
	while (addr + size > first->va_start && addr + size <= vend) {
		if (addr + cached_hole_size < first->va_start)
			cached_hole_size = first->va_start - addr;
		addr = ALIGN(first->va_end, align);
		if (addr + size < addr)
			goto overflow;

		if (list_is_last(&first->list, &vmap_area_list))
			goto found;

		first = list_entry(first->list.next, struct vmap_area, list);
	}

found:
	if (addr + size > vend)
		goto overflow;

	va->va_start = addr;
	va->va_end = addr + size;
	va->flags = 0;
	__insert_vmap_area(va);
	free_vmap_cache = &va->rb_node;
	spin_unlock(&vmap_area_lock);

	BUG_ON(va->va_start & (align-1));
	BUG_ON(va->va_start < vstart);
	BUG_ON(va->va_end > vend);

	return va;

overflow:
	spin_unlock(&vmap_area_lock);
	if (!purged) {
		purge_vmap_area_lazy();
		purged = 1;
		goto retry;
	}
	if (printk_ratelimit())
		printk(KERN_WARNING
		"vmap allocation for size %lu failed: "
		"use vmalloc=<size> to increase size.\n", size);
	kfree(va);
	return ERR_PTR(-EBUSY);
}

static void setup_vmalloc_vm(struct vm_struct *vm, struct vmap_area *va,
							 unsigned long flags, const void *caller)
{
	spin_lock(&vmap_area_lock);
	vm->flags = flags;
	vm->addr = (void *)va->va_start;
	vm->size = va->va_end - va->va_start;
	vm->caller = caller;
	va->vm = vm;
	va->flags |= VM_VM_AREA;
	spin_unlock(&vmap_area_lock);
}