#ifndef ToolFunctions_H
#define ToolFunctions_H

#include <linux/kernel.h>

#ifdef TEST_DEBUG
#define DEBUG_PRINT_LINE(...) do{printk(KERN_ALERT "ErrLine : %s::%d\nErrorInfo : ", __FILE__, __LINE__); printk(KERN_ALERT __VA_ARGS__);}while(0)
#define DEBUG_PRINT(...) printk(KERN_ALERT __VA_ARGS__)
#else
#define DEBUG_PRINT_LINE(...)
#define DEBUG_PRINT(...)
#endif

#ifdef TEST_DEBUG
void MEM_PRINT(unsigned long addr, unsigned long size);
void SetPageReadAndWriteAttribute(unsigned long ulAddress);
#else
#define MEM_PRINT(x, y)
#define SetPageReadAndWriteAttribute(x)
#endif

#endif
