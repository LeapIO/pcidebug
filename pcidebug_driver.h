#ifndef PCIDEBUG_DRIVER_H
#define PCIDEBUG_DRIVER_H

#define DEVICE_NAME "pcidebugdev"

#define SUCCESS 0
#define ERROR -1

#define BARS_MAXNUM   6

#define KPRINTF_MAX 1024 
#define SHOW_MAX 256

// struct used for read/write bar space
typedef struct{
    int barid;
    uint64_t offset;
    uint64_t value;
    int bitwidth;
} rwbar_t;


// commands nr
enum{
    RDBAR,
    WRBAR,
};

// ioctl commands
#define IOCTL_TYPE 0x13
#define IOCTL_RDBAR _IOR(IOCTL_TYPE, RDBAR, rwbar_t)
#define IOCTL_WRBAR _IOW(IOCTL_TYPE, WRBAR, rwbar_t)

#endif //PCIDEBUG_DRIVER_H