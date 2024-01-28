#
# Makefile of writer app
#

ARCH ?= arm64
CC := gcc

all : writer.c
	${CROSS_COMPILE}$(CC) -o writer writer.c
	 
clean :
	rm -f writer
