#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#define RPL0 0
#define RPL1 1
#define RPL2 2
#define RPL3 3
//-------------------IDT------------------
#define IDT_DES_P 1
#define IDT_DES_DPL_0 0
#define IDT_DES_DPL_3 3
#define IDT_DES_S   0
#define IDT_DES_TYPE 0xE//中断
#define IDT_DES0 ((IDT_DES_P<<7) + (IDT_DES_S<<6) + (IDT_DES_DPL_0<<5) + (IDT_DES_TYPE))
#define IDT_DES3 ((IDT_DES_P<<7) + (IDT_DES_S<<6) + (IDT_DES_DPL_3<<5) + (IDT_DES_TYPE))

//------------------selector--------------
#define TI_GDT 0
#define TI_LDT 1

#define SELECTOR_K_CODE	   ((1 << 3) + (TI_GDT << 2) + RPL0)
#define SELECTOR_K_DATA	   ((2 << 3) + (TI_GDT << 2) + RPL0)
#define SELECTOR_K_STACK   SELECTOR_K_DATA 
#define SELECTOR_K_GS	   ((3 << 3) + (TI_GDT << 2) + RPL0)

#endif