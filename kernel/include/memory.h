#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "bitmap.h"
#include "printk.h"

#define PG_SIZE 4096
struct virtual_addr{
    struct bitmap vaddr_bitmap;
    uint32_t vaddr_start;
};

/* 内存池结构,生成两个实例用于管理内核内存池和用户内存池 */
struct pool{
    struct bitmap pool_bitmap;
    uint32_t phy_addr_start;
    uint32_t pool_size;
};
enum pool_flags{
    PF_KERNEL = 1,
    PF_USER = 2
};

#define	 PG_P_1	  1	// 页表项或页目录项存在属性位
#define	 PG_P_0	  0	// 页表项或页目录项存在属性位
#define	 PG_RW_R  0	// R/W 属性位值, 读/执行
#define	 PG_RW_W  2	// R/W 属性位值, 读/写/执行
#define	 PG_US_S  0	// U/S 属性位值, 系统级
#define	 PG_US_U  4	// U/S 属性位值, 用户级

#define BITMAP_BASE 0xc009a000

void* get_kernel_pages(uint32_t pg_cnt);
void* malloc_page(enum pool_flags pf, uint32_t pg_cnt);
void mem_init();


#endif