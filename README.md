# pcidebug
Linux: suit for 5.10

module parameter:
- vendor: vendor id, default 0x10ee
- device: device id, default 0x7038
- insmod pcidebug.ko vendor=0x10ee device=0x7038

ioctl:
ioctl commands:
- IOCTL_RDBAR
- IOCTL_WRBAR

arg ( rwbar_t ) includeing:
- barid : select bar
- offset : specify offset from start of this bar
- value : read return value / write value
- bitwidth : support 8/16/32/64

example:
```
uint8_t pcidebug_rdbar8(int fd, int id, uint64_t offset){
    rwbar_t data;
    data.barid = id;
    data.offset = offset;
    data.value = 0;
    data.bitwidth = 8;
    ioctl(fd, IOCTL_RDBAR, &data);
    return data.value;
}
```

test:
1. "cd ./test" && "make test"
2. type cmd : 
    - [r/w][bitwdth] [bar id] [offset] [value]
        - r : read ; w : write
        - bitwdth = 8 / 16 / 32 / 64
        - bar id : a decimal number
        - offset : a decimal/hexdecimal number, the hexdecimal number must begin with "0x"
        - value : a hexdecimal number begin with "0x"
    - show [bar id] [offset] *[length]
        - bar id : a decimal number
        - offset : a decimal/hexdecimal number, the hexdecimal number must begin with "0x"
        - length : optional argument, a decimal number, default = 24 , max = 256

3. like : "r8 0 0", "w16 0 0x0 0xff" , "show 0 0"