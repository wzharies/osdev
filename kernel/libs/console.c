#include "console.h"
#include "io.h"


/* 引用其他人的描述
 * VGA(Video Graphics Array，视频图形阵列)是使用模拟信号的一种视频传输标准，内核可以通过它来控制屏幕上字符或者图形的显示。
 * 在默认的文本模式(Text-Mode)下，VGA控制器保留了一块内存(0xB8000~0xB8fa0)作为屏幕上字符显示的缓冲区，
 * 若要改变屏幕上字符的显示，只需要修改这块内存就好了。
 *
 */

static uint16_t *video_memory = (uint16_t *)(0xB8000);

static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;  //列

static void move_cursor(){
    // 屏幕是 80 字节宽
	uint16_t cursorLocation = cursor_y * 80 + cursor_x;
	
	// VGA 内部的寄存器多达300多个，显然无法一一映射到I/O端口的地址空间。
	// 对此 VGA 控制器的解决方案是，将一个端口作为内部寄存器的索引：0x3D4，
	// 再通过 0x3D5 端口来设置相应寄存器的值。
	// 在这里用到的两个内部寄存器的编号为14与15，分别表示光标位置的高8位与低8位。

	outb(0x3D4, 14);                  	// 告诉 VGA 我们要设置光标的高字节
	outb(0x3D5, cursorLocation >> 8); 	// 发送高 8 位
	outb(0x3D4, 15);                  	// 告诉 VGA 我们要设置光标的低字节
	outb(0x3D5, cursorLocation);     	// 发送低 8 位
}

//屏幕滚动操作
static  void scroll(){
    uint8_t attribute_byte = (0 << 4) | (15 & 0x0F);   //黑底白字  15是白，0是黑
    uint16_t blank = 0x20|(attribute_byte << 8);  //0x20表示空格

    if(cursor_y>=25){
        int i;
        for(i =0;i<24*80;i++){
            video_memory[i] = video_memory[i+80];
        }
        for(i = 24*80;i<25*80;i++){
            video_memory[i] = blank;
        }
        cursor_y = 24;
    }
}

void console_clear(){
    uint8_t attribute_byte = (0 << 4) | (15 & 0x0F);
	uint16_t blank = 0x20 | (attribute_byte << 8);

	int i;
	for (i = 0; i < 80 * 25; i++) {
	      video_memory[i] = blank;
	}

	cursor_x = 0;
	cursor_y = 0;
	move_cursor();
}

void console_putc_color(char c, real_color_t back, real_color_t fore){
    uint8_t back_color = (uint8_t)back;
	uint8_t fore_color = (uint8_t)fore;

	uint8_t attribute_byte = (back_color << 4) | (fore_color & 0x0F);
	uint16_t attribute = attribute_byte << 8;

    //0x08 退格键 0x09 tab键
    if(c==0x08 && cursor_x){
        cursor_x--;
    }else if(c==0x09){
        cursor_x = (cursor_x+8)& ~(7);   //向前移动8格后，将小于8格的位数清零
    }else if(c=='\r'){
        cursor_x = 0;
    }else if(c=='\n'){
        cursor_x = 0;
        cursor_y++;
    }else if(c >= ' '){
        video_memory[cursor_y*80+cursor_x] = c|attribute;
        cursor_x++;
    }

    if(cursor_x>=80){
        cursor_x=0;
        cursor_y++;
    }
    scroll();
    move_cursor();
}

void console_write(char *cstr){
    while(*cstr){
        console_putc_color(*cstr++,rc_black,rc_white);
    }
}

void console_write_color(char *cstr, real_color_t back, real_color_t fore){
	while (*cstr) {
	      console_putc_color(*cstr++, back, fore);
	}
}

void console_write_hex(uint32_t n, real_color_t back, real_color_t fore){
    console_write_color("0x",back,fore);
    char noZeroes = 1;
    int i,tmp;
    for(i=28;i>=0;i-=4){
        tmp = (n>>i)&0xF;
        if(tmp==0&&noZeroes!=0){
            continue;
        }
        noZeroes = 0;
        if(tmp>=0xA){
            console_putc_color(tmp-0xA+'a', back, fore);
        }else{
            console_putc_color(tmp+'0', back, fore);
        }
    }
}

void console_write_dec(uint32_t n, real_color_t back, real_color_t fore){
    char temp[32],ans[32];
    int i=0;
    while(n>0){
        temp[i++] = n%10+'0';
        n/=10;
    }
    i--;
    int j = 0;
    while(i>=0){
        ans[j++] = temp[i--];
    }
    ans[j]=0;
    console_write_color(ans,back,fore);
}
