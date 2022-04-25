# Author: hwj
# Description: Makefile for pcidebug driver

NAME := pcidebug
KERNEL_VER := $(shell uname -r)
KERNEL_DIR := /lib/modules/$(KERNEL_VER)/build

obj-m += $(NAME).o
$(NAME)-y := pcidebug_driver.o

all: $(NAME).ko
	
$(NAME).ko: $(NAME)_driver.c $(NAME)_driver.h
	make -C $(KERNEL_DIR) M=$(shell pwd) modules

clean:
	make -C $(KERNEL_DIR) M=$(shell pwd) clean
	@- $(RM) $(NAME) *.o.d

load: $(NAME).ko
	sudo insmod $(NAME).ko vendor=0x10ee device=0x7018

unload:
	sudo rmmod $(NAME)


