void vfree(const void *addr)
{
	BUG_ON(in_nmi());

	kmemleak_free(addr);

	if (!addr)
		return;
	if (unlikely(in_interrupt())) {
		struct vfree_deferred *p = &__get_cpu_var(vfree_deferred);
		if (llist_add((struct llist_node *)addr, &p->list))
			schedule_work(&p->wq);
	} else
		__vunmap(addr, 1);
}
EXPORT_SYMBOL(vfree);

static void __vunmap(const void *addr, int deallocate_pages)
{
	struct vm_struct *area;

	if (!addr)
		return;

	if (WARN(!PAGE_ALIGNED(addr), "Trying to vfree() bad address (%p)\n",
		addr))
		return;

	area = remove_vm_area(addr);
	if (unlikely(!area)) {
		WARN(1, KERN_ERR "Trying to vfree() nonexistent vm area (%p)\n",
			addr);
		return;
	}

	debug_check_no_locks_freed(addr, area->size);
	debug_check_no_obj_freed(addr, area->size);

	if (deallocate_pages) {
		int i;

		for (i = 0; i < area->nr_pages; i++) {
			struct page *page = area->pages[i];

			BUG_ON(!page);
			__free_page(page);
		}

		if (area->flags & VM_VPAGES)
			vfree(area->pages);
		else
			kfree(area->pages);
	}

	kfree(area);
	return;
}

struct vm_struct *remove_vm_area(const void *addr)
{
	struct vmap_area *va;

	va = find_vmap_area((unsigned long)addr);
	if (va && va->flags & VM_VM_AREA) 
	{
		struct vm_struct *vm = va->vm;

		spin_lock(&vmap_area_lock);
		va->vm = NULL;
		va->flags &= ~VM_VM_AREA;
		spin_unlock(&vmap_area_lock);

		vmap_debug_free_range(va->va_start, va->va_end);
		free_unmap_vmap_area(va);
		vm->size -= PAGE_SIZE;

		return vm;
	}
	return NULL;
}

static struct vmap_area *find_vmap_area(unsigned long addr)
{
	struct vmap_area *va;

	spin_lock(&vmap_area_lock);
	va = __find_vmap_area(addr);
	spin_unlock(&vmap_area_lock);

	return va;
}

static struct vmap_area *__find_vmap_area(unsigned long addr)
{
	struct rb_node *n = vmap_area_root.rb_node;

	while (n) {
		struct vmap_area *va;

		va = rb_entry(n, struct vmap_area, rb_node);
		if (addr < va->va_start)
			n = n->rb_left;
		else if (addr >= va->va_end)
			n = n->rb_right;
		else
			return va;
	}

	return NULL;
}

static void free_unmap_vmap_area(struct vmap_area *va)
{
	flush_cache_vunmap(va->va_start, va->va_end);
	free_unmap_vmap_area_noflush(va);
}

/*
 * Free and unmap a vmap area, caller ensuring flush_cache_vunmap had been
 * called for the correct range previously.
 */
static void free_unmap_vmap_area_noflush(struct vmap_area *va)
{
	unmap_vmap_area(va);
	free_vmap_area_noflush(va);
}

/*
 * Clear the pagetable entries of a given vmap_area
 */
static void unmap_vmap_area(struct vmap_area *va)
{
	vunmap_page_range(va->va_start, va->va_end);
}

static void vunmap_page_range(unsigned long addr, unsigned long end)
{
	pgd_t *pgd;
	unsigned long next;

	BUG_ON(addr >= end);
	pgd = pgd_offset_k(addr);
	do {
		next = pgd_addr_end(addr, end);
		if (pgd_none_or_clear_bad(pgd))
			continue;
		vunmap_pud_range(pgd, addr, next);
	} while (pgd++, addr = next, addr != end);
}

static void vunmap_pud_range(pgd_t *pgd, unsigned long addr, unsigned long end)
{
	pud_t *pud;
	unsigned long next;

	pud = pud_offset(pgd, addr);
	do {
		next = pud_addr_end(addr, end);
		if (pud_none_or_clear_bad(pud))
			continue;
		vunmap_pmd_range(pud, addr, next);
	} while (pud++, addr = next, addr != end);
}

static void vunmap_pmd_range(pud_t *pud, unsigned long addr, unsigned long end)
{
	pmd_t *pmd;
	unsigned long next;

	pmd = pmd_offset(pud, addr);
	do {
		next = pmd_addr_end(addr, end);
		if (pmd_none_or_clear_bad(pmd))
			continue;
		vunmap_pte_range(pmd, addr, next);
	} while (pmd++, addr = next, addr != end);
}

static void vunmap_pte_range(pmd_t *pmd, unsigned long addr, unsigned long end)
{
	pte_t *pte;

	pte = pte_offset_kernel(pmd, addr);
	do {
		pte_t ptent = ptep_get_and_clear(&init_mm, addr, pte);
		WARN_ON(!pte_none(ptent) && !pte_present(ptent));
	} while (pte++, addr += PAGE_SIZE, addr != end);
}

//清除页表会执行到如下函数：
static inline pte_t ptep_get_and_clear(struct mm_struct *mm, unsigned long addr,
									   pte_t *ptep)
{
	//将pte项清0
	pte_t pte = native_ptep_get_and_clear(ptep);
	pte_update(mm, addr, ptep);
	return pte;
}

static void free_vmap_area_noflush(struct vmap_area *va)
{
	va->flags |= VM_LAZY_FREE;
	atomic_add((va->va_end - va->va_start) >> PAGE_SHIFT, &vmap_lazy_nr);
	if (unlikely(atomic_read(&vmap_lazy_nr) > lazy_max_pages()))
		try_purge_vmap_area_lazy();
}