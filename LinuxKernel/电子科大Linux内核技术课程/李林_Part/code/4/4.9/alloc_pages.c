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
	return __vmalloc_node(size, 1, flags, PAGE_KERNEL,//对应页表项的属性
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

static void *__vmalloc_area_node(struct vm_struct *area, gfp_t gfp_mask,
								 pgprot_t prot, int node)
{
	const int order = 0;
	struct page **pages;
	unsigned int nr_pages, array_size, i;
	gfp_t nested_gfp = (gfp_mask & GFP_RECLAIM_MASK) | __GFP_ZERO;

	nr_pages = get_vm_area_size(area) >> PAGE_SHIFT;
	array_size = (nr_pages * sizeof(struct page *));

	area->nr_pages = nr_pages;
	/* Please note that the recursion is strictly bounded. */
	//分配vm_struct中pages数组所占的空间
	if (array_size > PAGE_SIZE) {
		pages = __vmalloc_node(array_size, 1, nested_gfp|__GFP_HIGHMEM,
			PAGE_KERNEL, node, area->caller);
		area->flags |= VM_VPAGES;
	} else {
		pages = kmalloc_node(array_size, nested_gfp, node);
	}
	area->pages = pages;
	if (!area->pages) {
		remove_vm_area(area->addr);
		kfree(area);
		return NULL;
	}

	for (i = 0; i < area->nr_pages; i++) {
		struct page *page;
		gfp_t tmp_mask = gfp_mask | __GFP_NOWARN;

		if (node == NUMA_NO_NODE)
			page = alloc_page(tmp_mask);
		else
			page = alloc_pages_node(node, tmp_mask, order);

		if (unlikely(!page)) {
			/* Successfully allocated i pages, free them in __vunmap() */
			area->nr_pages = i;
			goto fail;
		}
		area->pages[i] = page;
	}

	if (map_vm_area(area, prot, &pages))
		goto fail;
	return area->addr;

fail:
	warn_alloc_failed(gfp_mask, order,
		"vmalloc: allocation failure, allocated %ld of %ld bytes\n",
		(area->nr_pages*PAGE_SIZE), area->size);
	vfree(area->addr);
	return NULL;
}

static inline size_t get_vm_area_size(const struct vm_struct *area)
{
	/* return actual size without guard page */
	return area->size - PAGE_SIZE;
}
