#ifndef _IDT_H_
#define _IDT_H_

#define MAX_IDT 0x81      // 目前总共支持的中断数

#include "types.h"

typedef void* intr_handler ;


typedef struct idt_sec{
    uint16_t func_idt_low;
    uint16_t idt_selector;
    uint8_t always0;
    uint8_t flags;
    uint16_t func_idt_high;
}__attribute__((PACKED)) idt_sec;

enum intr_status{
    INTR_ON,
    INTR_OFF
};

void idt_init();
void idt_des_init();
void idt_pic_init();
void idt_func_init();
void general_intr_func(uint8_t vect);
void register_handler(uint8_t cnt,intr_handler function);
void idt_set_gate(uint16_t numb,uint16_t attr,intr_handler function);

enum intr_status get_intr_status();
enum intr_status set_intr_status(enum intr_status status);

#endif