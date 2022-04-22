# Author: hwj
# Description: Makefile for pcidebug driver

NAME := pcidebug
KERNEL_VER := $(shell uname -r)
KERNEL_DIR := /lib/modules/$(KERNEL_VER)/build

obj-m += $(NAME).o
$(NAME)-y := pcidebug_driver.o

all: $(NAME).ko 
	

clean:
	make -C $(KERNEL_DIR) M=$(shell pwd) clean

$(NAME).ko: *.c *.h
	make -C $(KERNEL_DIR) M=$(shell pwd) modules

load: $(NAME).ko
	insmod $(NAME).ko

unload:
	rmmod $(NAME)