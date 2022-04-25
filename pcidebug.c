#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "pcidebug_driver.h"

uint8_t pcidebug_rdbar8(int fd, int id, uint64_t offset){
    printf("pcidebug_rdbar8");
    rwbar_t data;
    data.barid = id;
    data.offset = offset;
    data.value = 0;
    printf(",&data =%p\n",&data);
    ioctl(fd, IOCTL_RDBAR8, &data);
    return data.value;
}

uint16_t pcidebug_rdbar16(int fd, int id, uint64_t offset){
    rwbar_t data;
    data.barid = id;
    data.offset = offset;
    data.value = 0;
    ioctl(fd, IOCTL_RDBAR16, &data);
    return data.value;
}

uint32_t pcidebug_rdbar32(int fd, int id, uint64_t offset){
    rwbar_t data;
    data.barid = id;
    data.offset = offset;
    data.value = 0;
    ioctl(fd, IOCTL_RDBAR32, &data);
    return data.value;
}

uint64_t pcidebug_rdbar64(int fd, int id, uint64_t offset){
    rwbar_t data;
    data.barid = id;
    data.offset = offset;
    data.value = 0;
    ioctl(fd, IOCTL_RDBAR64, &data);
    return data.value;
}

void pcidebug_wrbar8(int fd, int id, uint64_t offset, uint8_t value){
    rwbar_t data;
    data.barid = id;
    data.offset = offset;
    data.value = value;
    ioctl(fd, IOCTL_WRBAR8, &data);
}

void pcidebug_wrbar16(int fd, int id, uint64_t offset, uint16_t value){
    rwbar_t data;
    data.barid = id;
    data.offset = offset;
    data.value = value;
    ioctl(fd, IOCTL_WRBAR16, &data);
}

void pcidebug_wrbar32(int fd, int id, uint64_t offset, uint32_t value){
    rwbar_t data;
    data.barid = id;
    data.offset = offset;
    data.value = value;
    ioctl(fd, IOCTL_WRBAR32, &data);
}

void pcidebug_wrbar64(int fd, int id, uint64_t offset, uint64_t value){
    rwbar_t data;
    data.barid = id;
    data.offset = offset;
    data.value = value;
    ioctl(fd, IOCTL_WRBAR64, &data);
}

int main(int argc,char *argv[]){
    int fd = open("/dev/"DEVICE_NAME, O_RDWR | O_SYNC);
    if(0 > fd){
        printf("Failed to open %s\n",DEVICE_NAME);
        return 0;
    }
    printf("Open /dev/"DEVICE_NAME"\n");
    printf("BAR0:0x0 = %x\n",pcidebug_rdbar8(fd,2,0));
    //printf("BAR0:0x0 = %x\n",pcidebug_rdbar16(fd,2,0));
    //printf("BAR0:0x0 = %x\n",pcidebug_rdbar32(fd,2,0));
    //printf("BAR0:0x0 = %lx\n",pcidebug_rdbar64(fd,2,0));
    //printf("Close %s\n",DEVICE_NAME);
    close(fd);
}