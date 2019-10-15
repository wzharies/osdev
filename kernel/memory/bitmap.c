#include "bitmap.h"
#include "types.h"
#include "printk.h"
#include "string.h"

void bitmap_init(struct bitmap* btmp){
    memset(btmp->bits,0,btmp->btmp_bytes_len);
}
int bitmap_scan_test(struct bitmap* btmp, uint32_t bit_idx){
    uint32_t bit_x,bit_y;
    bit_x = bit_idx/8;
    bit_y = bit_idx%8;
    return (btmp->bits[bit_x] & (1<<bit_y));
}

//申请连续cnt个位
int bitmap_scan(struct bitmap* btmp, uint32_t cnt){
    uint32_t flag = 0,temp_cnt=0;
    for(uint32_t i = 0;i<btmp->btmp_bytes_len;i++){
        if(btmp->bits[i]==0xff){
            continue;
        }else{ 
            for(uint32_t k=0;k<8;k++){
                if(!(btmp->bits[i]&(1<<k))){    // 此处没有使用
                    if(flag==0){                //找到第一个空位，开始统计
                        flag=1;
                    }
                    temp_cnt++;
                    if(temp_cnt==cnt){          //达到需要的数量
                        return i*8+k-cnt+1;
                    }
                }else if(flag){
                    flag = 0;
                    temp_cnt==0;
                }
                //printk("cnt = %d",temp_cnt);
            }
        }
    }
    return -1;
}

void bitmap_set(struct bitmap* btmp, uint32_t bit_idx, int8_t value){
    uint32_t bit_x,bit_y;
    bit_x = bit_idx/8;
    bit_y = bit_idx%8;
    if(value){
        btmp->bits[bit_x] |= (1<<bit_y);
    }else{
        btmp->bits[bit_x] &= ~(1<<bit_y);
    }
}