#include "idt.h"
#include "printk.h"
#include "console.h"
int main(){
    //while(1);
    console_clear();
    printk("start\n");
    idt_init();
    
    while(1);
}