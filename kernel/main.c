#include "idt.h"
#include "printk.h"
#include "console.h"
#include "timer.h"
#include "debug.h"
#include "memory.h"

int main(){
    //while(1);
    console_clear();
    printk("start\n");
    idt_init();
    timer_init();
    mem_init();
    get_kernel_pages(2000);
    //ASSERT(1==2);
    while(1);
}