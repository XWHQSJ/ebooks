#define virt_addr_valid(kaddr) __virt_addr_valid((unsigned long) (kaddr))
bool __virt_addr_valid(unsigned long x)
{
	//当x<__START_KERNEL_map时，y>x；
	//当x>=__START_KERNEL_map时，x>y
	unsigned long y = x - __START_KERNEL_map;

	if (unlikely(x > y)) 
	{
		//此时新计算出来的x即物理地址
		x = y + phys_base;

		//线性地址超过了内核映像映射的范围
		if (y >= KERNEL_IMAGE_SIZE)
			return false;
	} 
	else
	{
		//__START_KERNEL_map - PAGE_OFFSET = 0x77FF80000000
		//x<0xffff8800_00000000时，新计算得出的x>y，直接返回false
		//x>=0xffff8800_00000000时,新计算得出的x<y
		//新计算得出的x实际上就是输入参数x对应的物理地址
		x = y + (__START_KERNEL_map - PAGE_OFFSET);

		//判断物理地址x是否超出当前cpu最大支持的物理地址范围
		if ((x > y) || !phys_addr_valid(x))
			return false;
	}   

	//这里是根据section的情况进行判断
	return pfn_valid(x >> PAGE_SHIFT);
}
#define KERNEL_IMAGE_SIZE (512 * 1024 * 1024)

//判断物理地址addr是否超出当前cpu最大支持的物理地址范围
static inline int phys_addr_valid(resource_size_t addr)
{
	//x86_phys_bits保留当前cpu的物理地址位数，虚拟机上测得为40
	return !(addr >> boot_cpu_data.x86_phys_bits);
}

static inline int pfn_valid(unsigned long pfn)
{
	//pfn对应的section号是否超过最大section号
	if (pfn_to_section_nr(pfn) >= NR_MEM_SECTIONS)
		return 0;

	return valid_section(__nr_to_section(pfn_to_section_nr(pfn)));
}

//即判断section，以及SECTION_HAS_MEM_MAP位（bit1）是否设置判断是否有效
static inline int valid_section(struct mem_section *section)
{
	return (section && (section->section_mem_map & SECTION_HAS_MEM_MAP));
}