#include "idt.h"
#include "global.h"
#include "printk.h"
#include "console.h"
#include "types.h"
#include "io.h"

#define PIC_M_CTRL 0x20	       // 这里用的可编程中断控制器是8259A,主片的控制端口是0x20
#define PIC_M_DATA 0x21	       // 主片的数据端口是0x21
#define PIC_S_CTRL 0xa0	       // 从片的控制端口是0xa0
#define PIC_S_DATA 0xa1	       // 从片的数据端口是0xa1

static idt_sec idt_secs[MAX_IDT];
static char * intr_name[MAX_IDT];

extern intr_handler idt_entries[MAX_IDT];//idt的进入函数
intr_handler idt_handlers[MAX_IDT];//idt实际调用函数
//extern uint32_t system_call();


void idt_pic_init(){
    printk("idt_pic_init_start!\n");

    //这段代码来自书本
    /* 初始化主片 */
   outb (PIC_M_CTRL, 0x11);   // ICW1: 边沿触发,级联8259, 需要ICW4.
   outb (PIC_M_DATA, 0x20);   // ICW2: 起始中断向量号为0x20,也就是IR[0-7] 为 0x20 ~ 0x27.
   outb (PIC_M_DATA, 0x04);   // ICW3: IR2接从片. 
   outb (PIC_M_DATA, 0x01);   // ICW4: 8086模式, 正常EOI

   /* 初始化从片 */
   outb (PIC_S_CTRL, 0x11);    // ICW1: 边沿触发,级联8259, 需要ICW4.
   outb (PIC_S_DATA, 0x28);    // ICW2: 起始中断向量号为0x28,也就是IR[8-15] 为 0x28 ~ 0x2F.
   outb (PIC_S_DATA, 0x02);    // ICW3: 设置从片连接到主片的IR2引脚
   outb (PIC_S_DATA, 0x01);    // ICW4: 8086模式, 正常EOI
  
   /* IRQ2用于级联从片,必须打开,否则无法响应从片上的中断
   主片上打开的中断有IRQ0的时钟,IRQ1的键盘和级联从片的IRQ2,其它全部关闭 */
   outb (PIC_M_DATA, 0xf8);

   /* 打开从片上的IRQ14,此引脚接收硬盘控制器的中断 */
   outb (PIC_S_DATA, 0xbf);
}

void idt_des_init(){
    printk("idt_des_init_start!\n");
    for(int i = 0;i<MAX_IDT;i++){
        idt_set_gate(i,IDT_DES0,idt_entries[i]);
    }
    //系统调用
    //idt_set_date(0x80,IDT_DES3,system_call);
}

void idt_func_init(){
    printk("idt_func_init_start!\n");
    for(int i = 0;i<MAX_IDT;i++){
        idt_handlers[i] = general_intr_func;
        intr_name[i] = "undefined";
    }
    intr_name[0] = "#DE Divide Error";
    intr_name[1] = "#DB Debug Exception";
    intr_name[2] = "NMI Interrupt";
    intr_name[3] = "#BP Breakpoint Exception";
    intr_name[4] = "#OF Overflow Exception";
    intr_name[5] = "#BR BOUND Range Exceeded Exception";
    intr_name[6] = "#UD Invalid Opcode Exception";
    intr_name[7] = "#NM Device Not Available Exception";
    intr_name[8] = "#DF Double Fault Exception";
    intr_name[9] = "Coprocessor Segment Overrun";
    intr_name[10] = "#TS Invalid TSS Exception";
    intr_name[11] = "#NP Segment Not Present";
    intr_name[12] = "#SS Stack Fault Exception";
    intr_name[13] = "#GP General Protection Exception";
    intr_name[14] = "#PF Page-Fault Exception";
    // intr_name[15] 第15项是intel保留项，未使用
    intr_name[16] = "#MF x87 FPU Floating-Point Error";
    intr_name[17] = "#AC Alignment Check Exception";
    intr_name[18] = "#MC Machine-Check Exception";
    intr_name[19] = "#XF SIMD Floating-Point Exception";
}

void general_intr_func(uint8_t vect){
    if (vect == 0x27 || vect == 0x2f) {	// 0x2f是从片8259A上的最后一个irq引脚，保留
        return;		//IRQ7和IRQ15会产生伪中断(spurious interrupt),无须处理。
    }
    //console_clear();
    printk("!!!!!!!      exception message begin  !!!!!!!!\n");
    printk("exception numb:%d   name:",vect);
    printk(intr_name[vect]);
    if (vect == 14) {	  // 若为Pagefault,将缺失的地址打印出来并悬停
        int page_fault_vaddr = 0; 
        asm ("movl %%cr2, %0" : "=r" (page_fault_vaddr));	  // cr2是存放造成page_fault的地址
        printk("\npage fault addr is 0x %d",page_fault_vaddr);
    }
    printk("\n!!!!!!!      exception message end    !!!!!!!!\n");
    while(1);
}

void register_handler(uint8_t cnt,intr_handler function){
    idt_handlers[cnt] = function;
}

void idt_set_gate(uint16_t numb,uint16_t attr,intr_handler function){
    idt_secs[numb].always0=0;
    idt_secs[numb].flags=attr;
    idt_secs[numb].func_idt_low=(uint32_t)function & 0x0000ffff;
    idt_secs[numb].func_idt_high=((uint32_t)function & 0xffff0000)>>16;
    idt_secs[numb].idt_selector=SELECTOR_K_CODE;
}

void idt_init(){
    printk("----idt_init_start!----\n");
    idt_pic_init();
    idt_des_init();
    idt_func_init();

    uint64_t idt_operand = (sizeof(idt_secs)-1)|((uint64_t)((uint32_t)idt_secs<<16 ));
    asm volatile("lidt %0"::"m"(idt_operand));
    printk("----idt_init_end----\n");
}

enum intr_status get_intr_status(){
    uint32_t eflags;
    uint32_t mask_flag = 0x200;
    asm volatile("pushfl; popl %0":"=g"(eflags));
    return (eflags & mask_flag) ? INTR_ON :INTR_OFF;
}

enum intr_status set_intr_status(enum intr_status status){
    if(get_intr_status()==status){
        return status;
    }else if(get_intr_status()==INTR_ON){
        asm volatile("cli":::"memory");     //关
        return INTR_ON;
    }else{
        asm volatile("sti");    //开
        return INTR_OFF;
    }
}