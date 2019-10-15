#include "timer.h"
#include "printk.h"
#include "io.h"

#define IRQ0_FREQUENCY	   1
#define INPUT_FREQUENCY	   1193180
#define COUNTER0_VALUE	   INPUT_FREQUENCY / IRQ0_FREQUENCY
#define COUNTER0_PORT 0x40
#define COUNTER0_NO 0
#define COUNTER0_MODE 2
#define READ_WRITE_LATCH 3
#define PIT_CONTROL_PORT 0x43
static int frequency_set(uint8_t counter_port,uint8_t counter_no,uint8_t rwl,uint8_t counter_mode,uint16_t counter_value){
    /* 往控制字寄存器端口0x43中写入控制字 */
   outb(PIT_CONTROL_PORT, (uint8_t)(counter_no << 6 | rwl << 4 | counter_mode << 1));
    /* 先写入counter_value的低8位 */
   outb(counter_port, (uint8_t)(counter_value & 0xFF));
    /* 再写入counter_value的高8位 */
   outb(counter_port, (uint8_t)(counter_value >> 8) & 0xFF);
}

void timer_handler(){
    static uint32_t cnt=0;
    printk("Tick :%d\n",cnt++);
}

int timer_init(){
    printk("timer_init\n");
    frequency_set(COUNTER0_PORT, COUNTER0_NO, READ_WRITE_LATCH, COUNTER0_MODE, COUNTER0_VALUE);
    register_handler(0x20, timer_handler);
}