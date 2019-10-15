#include "types.h"
#include "printk.h"
#include "bitmap.h"
#include "string.h"
#include "memory.h"
#include "debug.h"

#define PDE_IDX(addr) ((addr>>22)&0x3ff)
#define PTE_IDX(addr) ((addr>>12)&0x3ff)
#define K_HEAP_START 0xc0100000


struct pool kernel_pool, user_pool;
struct virtual_addr kernel_vaddr;


/* 得到虚拟地址vaddr对应的pte指针*/
uint32_t* pte_ptr(uint32_t vaddr) {
   /* 先访问到页表自己 + \
    * 再用页目录项pde(页目录内页表的索引)做为pte的索引访问到页表 + \
    * 再用pte的索引做为页内偏移*/
   uint32_t* pte = (uint32_t*)(0xffc00000+((vaddr & 0xffc00000) >> 10) + PTE_IDX(vaddr) * 4);
   return pte;
}

/* 得到虚拟地址vaddr对应的pde的指针 */
uint32_t* pde_ptr(uint32_t vaddr) {
   /* 0xfffff是用来访问到页表本身所在的地址 */
   uint32_t* pde = (uint32_t*)((0xfffff000) + PDE_IDX(vaddr) * 4);
   return pde;
}

//分配一个物理页
static void* palloc(struct pool* m_pool) {
    uint32_t bitmap_n = bitmap_scan(&m_pool->pool_bitmap,1);
    if(bitmap_n==-1){
        return NULL;
    }
    bitmap_set(&m_pool->pool_bitmap,bitmap_n,1);
    return (void *)(m_pool->phy_addr_start + bitmap_n*PG_SIZE);
}

//将虚拟地址与物理地址映射
static void page_table_add(void* _vaddr, void* _page_phyaddr){
    uint32_t phyaddr = (uint32_t)_page_phyaddr;
    uint32_t *pte_addr = pte_ptr((uint32_t)_vaddr);
    uint32_t *pde_addr = pde_ptr((uint32_t)_vaddr);
    printk("pde_addr %p\n",pde_addr);
    printk("pde_addr %x\n",*pde_addr);
    printk("pte_addr %p\n",pte_addr);
    printk("pte_addr %x\n",*pte_addr);

    if((*pde_addr)&0x1){
        ASSERT(((*pde_addr)&0x1));
        if(!(*pte_addr)&0x1){
            *pte_addr = (phyaddr | PG_P_1|PG_RW_W|PG_US_U);
        }else{
            PANIC("pte not exist");
        }
    }else{
        uint32_t p_addr = (uint32_t)palloc(&kernel_pool);
        memset((void *)(p_addr&0xfffff000),0,PG_SIZE);
        *pde_addr = (p_addr | PG_P_1|PG_RW_W|PG_US_U);
        *pte_addr = (phyaddr | PG_P_1|PG_RW_W|PG_US_U);  //
    }
    asm volatile ("invlpg (%0)" : : "a" (_vaddr));
}


//分配pg_cnt个虚拟页
static void* vaddr_get(enum pool_flags pf, uint32_t pg_cnt){
    if(pf==PF_KERNEL){
        uint32_t bitmap_n = bitmap_scan(&kernel_vaddr.vaddr_bitmap,pg_cnt);
        printk("%d\n",bitmap_n);
        if(bitmap_n==-1){
            return NULL;
        }
        for(uint32_t i = 0;i<pg_cnt;i++){
            bitmap_set(&kernel_vaddr.vaddr_bitmap,bitmap_n+i,1);
        }
        return (void *)(kernel_vaddr.vaddr_start + bitmap_n * PG_SIZE);
    }else{
        return NULL;
    }
}


//分配n个页面
void* malloc_page(enum pool_flags pf, uint32_t pg_cnt){
/***********   malloc_page的原理是三个动作的合成:   ***********
      1通过vaddr_get在虚拟内存池中申请虚拟地址
      2通过palloc在物理内存池中申请物理页
      3通过page_table_add将以上两步得到的虚拟地址和物理地址在页表中完成映射
***************************************************************/
    void * vaddr = vaddr_get(pf,pg_cnt);
    printk("malloc v_addr at :%p\n",vaddr);
    if(vaddr==NULL){
        return NULL;
    }
    for(uint32_t i = 0;i < pg_cnt;i++){
        void * paddr = palloc(pf==PF_KERNEL ? &kernel_pool : &user_pool);
        printk("malloc p_addr at :%p\n",paddr);
        if(paddr==NULL){
            return NULL;
        }
        page_table_add(vaddr+i*PG_SIZE , paddr);
    }
    return vaddr;
}

//内核分配一个页
void* get_kernel_pages(uint32_t pg_cnt){
    void * start = malloc_page(PF_KERNEL,pg_cnt);
    if(start != NULL){
        memset(start,0,pg_cnt*PG_SIZE);
    }
    return start;
}


void mem_pool_init(uint32_t total_mem){

    uint32_t page_table_size = PG_SIZE*256;
    uint32_t used_size = page_table_size + 0x100000;
    uint32_t usable_size = total_mem - used_size;
    uint32_t usable_pages = usable_size/PG_SIZE;
    uint32_t user_pages = usable_pages/2;
    uint32_t kernel_pages = usable_pages-user_pages; 

    kernel_pool.phy_addr_start = used_size;
    kernel_pool.pool_size = kernel_pages * PG_SIZE;
    kernel_pool.pool_bitmap.btmp_bytes_len = kernel_pages/8;

    user_pool.phy_addr_start = used_size + kernel_pool.pool_size;
    user_pool.pool_size = user_pages * PG_SIZE;
    user_pool.pool_bitmap.btmp_bytes_len = user_pages/8;

    kernel_pool.pool_bitmap.bits = (void *)BITMAP_BASE;

    user_pool.pool_bitmap.bits = (void*)(BITMAP_BASE+ kernel_pool.pool_bitmap.btmp_bytes_len);

    printk("kernel_pool start : %p\n",kernel_pool.phy_addr_start);
    printk("kernel_pool size  : %x\n",kernel_pool.pool_size);
    printk("kernel_pool bitmap: %p\n",kernel_pool.pool_bitmap.bits);
    printk("  user_pool start : %p\n",user_pool.phy_addr_start);
    printk("  user_pool size  : %x\n",user_pool.pool_size);
    printk("  user_pool bitmap: %p\n",user_pool.pool_bitmap.bits);

    printk("kernel_bitmap_size: %x\n",kernel_pool.pool_bitmap.btmp_bytes_len);
    printk("  user_bitmap_size: %x\n",user_pool.pool_bitmap.btmp_bytes_len);


    bitmap_init(&kernel_pool.pool_bitmap);
    bitmap_init(&user_pool.pool_bitmap);

    kernel_vaddr.vaddr_bitmap.bits = (uint32_t)(void *)(BITMAP_BASE + kernel_pool.pool_bitmap.btmp_bytes_len+user_pool.pool_bitmap.btmp_bytes_len);
    kernel_vaddr.vaddr_bitmap.btmp_bytes_len = kernel_pool.pool_bitmap.btmp_bytes_len;
    kernel_vaddr.vaddr_start = K_HEAP_START;
    bitmap_init(&kernel_vaddr.vaddr_bitmap);
    printk("kernel_vaddr_start: %p\n",kernel_vaddr.vaddr_start);
    printk("kernel_vaddr_bits : %p\n",kernel_vaddr.vaddr_bitmap.bits);
    
}

void mem_init(){
    printk("mem_init start!\n");
    uint32_t total_mem = 32*1024*1024;
    mem_pool_init(total_mem);
    printk("mem_init end!\n");
}