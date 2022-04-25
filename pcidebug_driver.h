#ifndef PCIDEBUG_DRIVER_H
#define PCIDEBUG_DRIVER_H

#define DEVICE_NAME "pcidebugdev"
#define SUCCESS 0
#define ERROR -1

#define BARS_MAXNUM   6

// struct used for read/write bar space
typedef struct{
    int barid;
    uint64_t offset;
    uint64_t value;
} rwbar_t;

// ioctl commands
enum{
    IOCTL_RDBAR8,
    IOCTL_RDBAR16,
    IOCTL_RDBAR32,
    IOCTL_RDBAR64,
    IOCTL_WRBAR8,
    IOCTL_WRBAR16,
    IOCTL_WRBAR32,
    IOCTL_WRBAR64,
};

#endif //PCIDEBUG_DRIVER_H