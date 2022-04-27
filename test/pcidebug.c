#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include "../pcidebug_driver.h"

int pcidebug_open(){
    int fd = open("/dev/"DEVICE_NAME, O_RDWR | O_SYNC);
    if(0 > fd){
        printf("Failed to open %s\n",DEVICE_NAME);
    }
    return fd;
}

void pcidebug_close(int fd){
    close(fd);
    printf("Close %s\n",DEVICE_NAME);
}

uint8_t pcidebug_rdbar8(int fd, int id, uint64_t offset){
    rwbar_t data;
    data.barid = id;
    data.offset = offset;
    data.value = 0;
    data.bitwidth = 8;
    ioctl(fd, IOCTL_RDBAR, &data);
    return data.value;
}

uint16_t pcidebug_rdbar16(int fd, int id, uint64_t offset){
    rwbar_t data;
    data.barid = id;
    data.offset = offset;
    data.value = 0;
    data.bitwidth = 16;
    ioctl(fd, IOCTL_RDBAR, &data);
    return data.value;
}

uint32_t pcidebug_rdbar32(int fd, int id, uint64_t offset){
    rwbar_t data;
    data.barid = id;
    data.offset = offset;
    data.value = 0;
    data.bitwidth = 32;
    ioctl(fd, IOCTL_RDBAR, &data);
    return data.value;
}

uint64_t pcidebug_rdbar64(int fd, int id, uint64_t offset){
    rwbar_t data;
    data.barid = id;
    data.offset = offset;
    data.value = 0;
    data.bitwidth = 64;
    ioctl(fd, IOCTL_RDBAR, &data);
    return data.value;
}

uint8_t pcidebug_wrbar8(int fd, int id, uint64_t offset, uint8_t value){
    rwbar_t data;
    data.barid = id;
    data.offset = offset;
    data.value = value;
    data.bitwidth = 8;
    ioctl(fd, IOCTL_WRBAR, &data);
    return (uint8_t)data.value;
}

uint16_t pcidebug_wrbar16(int fd, int id, uint64_t offset, uint16_t value){
    rwbar_t data;
    data.barid = id;
    data.offset = offset;
    data.value = value;
    data.bitwidth = 16;
    ioctl(fd, IOCTL_WRBAR, &data);
    return (uint16_t)data.value;
}

uint32_t pcidebug_wrbar32(int fd, int id, uint64_t offset, uint32_t value){
    rwbar_t data;
    data.barid = id;
    data.offset = offset;
    data.value = value;
    data.bitwidth = 32;
    ioctl(fd, IOCTL_WRBAR, &data);
    return (uint32_t)data.value;
}

uint64_t pcidebug_wrbar64(int fd, int id, uint64_t offset, uint64_t value){
    rwbar_t data;
    data.barid = id;
    data.offset = offset;
    data.value = value;
    data.bitwidth = 64;
    ioctl(fd, IOCTL_WRBAR, &data);
    return (uint64_t)data.value;
}

// int main(int argc,char *argv[]){
//     int fd = open("/dev/"DEVICE_NAME, O_RDWR | O_SYNC);
//     if(0 > fd){
//         printf("Failed to open %s\n",DEVICE_NAME);
//         return 0;
//     }
//     printf("Open /dev/"DEVICE_NAME"\n");
//     // for(int i=0;i<8;i++){
//     //     printf("BAR0:0x0 = %x\n",pcidebug_rdbar8(fd,0,i));
//     // }
//     // for(int i =0;i<4;i++){
//     //     printf("BAR0:0x0 = %x\n",pcidebug_rdbar16(fd,0,i*2));
//     // }
//     // pcidebug_wrbar64(fd, 0, 0, 0xff000000ee800000);
//     // printf("r8 BAR0:0x0 = 0x%02x\n",pcidebug_rdbar8(fd,0,0));
//     // printf("r16 BAR0:0x0 = 0x%04x\n",pcidebug_rdbar16(fd,0,0));
//     // printf("r32 BAR0:0x0 = 0x%08x\n",pcidebug_rdbar32(fd,0,0));
//     // printf("r32 BAR0:0x0 = 0x%08x\n",pcidebug_rdbar32(fd,0,4));
//     // printf("r64 BAR0:0x0 = 0x%016lx\n",pcidebug_rdbar64(fd,0,0));
//     char cmd[10];
//     int id;
//     uint64_t offset;
//     uint64_t val;
//     printf("pcidebug$ ");
//     while(scanf("%s",cmd)!=EOF){
//         id = offset = val = 0; //reset
//         if(strcmp(cmd,"r8")==0){
//             scanf("%d %lu", &id, &offset);
//             printf("val = 0x%02x\n",pcidebug_rdbar8(fd,id,offset));
//         }else if(strcmp(cmd,"r16")==0){
//             scanf("%d %lu", &id, &offset);
//             printf("val = 0x%04x\n",pcidebug_rdbar16(fd,id,offset));
//         }else if(strcmp(cmd,"r32")==0){  
//             scanf("%d %lu", &id, &offset);
//             printf("val = 0x%08x\n",pcidebug_rdbar32(fd,id,offset));
//         }else if(strcmp(cmd,"r64")==0){
//             scanf("%d %lu", &id, &offset);
//             printf("val = 0x%016lx\n",pcidebug_rdbar64(fd,id,offset));
//         }else if(strcmp(cmd,"w8")==0){
//             scanf("%d %lu %lx", &id, &offset, &val);
//             pcidebug_wrbar8(fd,id,offset,val);
//             printf("wrbar8 0x%02x done\n",(uint8_t)val);
//         }else if(strcmp(cmd,"w16")==0){
//             scanf("%d %lu %lx", &id, &offset, &val);
//             pcidebug_wrbar16(fd,id,offset,val);
//             printf("wrbar16 0x%04x done\n",(uint16_t)val);
//         }else if(strcmp(cmd,"w32")==0){  
//             scanf("%d %lu %lx", &id, &offset, &val);
//             pcidebug_wrbar32(fd,id,offset,val);
//             printf("wrbar32 0x%08x done\n",(uint32_t)val);
//         }else if(strcmp(cmd,"w64")==0){
//             scanf("%d %lu %lx", &id, &offset, &val);
//             pcidebug_wrbar64(fd,id,offset,val);
//             printf("wrbar64 0x%016lx done\n",val);
//         }else if(strcmp(cmd,"exit")==0){
//             break;
//         }else{
//             printf("\n");
//         }
//         printf("pcidebug$ ");
//     }
    
//     printf("Close %s\n",DEVICE_NAME);
//     close(fd);
// }