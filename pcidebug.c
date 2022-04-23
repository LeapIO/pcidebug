#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include "pcidebug_driver.h"

int main(int argc,char *argv[]){
    printf("Open /dev/"DEVICE_NAME);
    int fd = open("/dev/"DEVICE_NAME, O_RDWR | O_SYNC);
    if(0 > fd){
        printf("Failed to open %s\n",DEVICE_NAME);
    }
    printf("Close %s\n",DEVICE_NAME);
    close(fd);
}