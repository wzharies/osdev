file kernel.bin
target remote :1234
display/i $pc
break *0x7c00
