# Author: hwj
# Description: Makefile for pcidebug test

NAME := pcidebug

all: lib$(NAME).so

clean:
	@- $(RM) lib$(NAME).so

# for test
lib$(NAME).so: $(NAME).c 
	$(CC) -shared -Wl,-soname,lib$(NAME) -o lib$(NAME).so -fpic $(NAME).c 

test: lib$(NAME).so
	python $(NAME).py

