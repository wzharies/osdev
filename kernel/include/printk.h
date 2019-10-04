#ifndef _PRINTK_H_
#define _PRINTK_H_
#include "console.h"
// 内核的打印函数
void printk(const char *format, ...);

// 内核的打印函数 带颜色
void printk_color(real_color_t back, real_color_t fore, const char *format, ...);

#endif