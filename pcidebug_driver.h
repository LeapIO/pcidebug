#ifndef PCIDEBUG_DRIVER_H
#define PCIDEBUG_DRIVER_H

#define DEVICE_NAME "pcidebugdev"

#define SUCCESS 0
#define ERROR -1

#define HAVE_MEM_REGION 0x01
#define HAVE_REGION 0x02

#define BARS_MAXNUM   6

// struct used for read/write bar space
typedef struct{
    int barid;
    uint64_t offset;
    uint64_t value;
    int bitwidth;
} rwbar_t;

// ioctl commands
enum{
    IOCTL_RDBAR,

    IOCTL_WRBAR,
};

#endif //PCIDEBUG_DRIVER_H