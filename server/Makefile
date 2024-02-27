#
# Makefile of writer app
#

ARCH ?= arm64
CC := gcc

all : server.c helper_server.c
	${CROSS_COMPILE}$(CC) -o aesdsocket server.c helper_server.c
	 
clean :
	rm -f aesdsocket
