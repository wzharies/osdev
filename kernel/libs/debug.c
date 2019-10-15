#include "debug.h"
#include "printk.h"
#include "idt.h"

void panic_spin(char* filename, int line, const char* func, const char* condition){
    set_intr_status(INTR_OFF);
    printk("\n\n!!!!error!!!!\n");
    printk("filename : %s\n",filename);
    printk("line : %d\n",line);
    printk("function name : %s\n",func);
    printk("condition : %s\n",condition);
    while(1);
}
