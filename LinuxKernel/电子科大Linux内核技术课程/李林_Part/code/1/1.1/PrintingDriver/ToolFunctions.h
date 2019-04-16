#ifndef ToolFunctions_H
#define ToolFunctions_H

#ifdef TEST_DEBUG
#define DEBUG_PRINT_LINE(...) do{printk(KERN_ALERT "ErrLine : %s::%d\nErrorInfo : ", __FILE__, __LINE__); printk(KERN_ALERT __VA_ARGS__);}while(0)
#define DEBUG_PRINT(...) printk(KERN_ALERT __VA_ARGS__)
#else
#define DEBUG_PRINT_LINE(...)
#define DEBUG_PRINT(...)
#endif

#endif