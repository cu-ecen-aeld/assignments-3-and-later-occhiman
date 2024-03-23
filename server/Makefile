#
# Makefile of writer app
#

ARCH ?= arm64
CC := gcc
CFLAGS := -g
LDFLAGS := -pthread -lpthread

all : server.c helper_server.c
	${CROSS_COMPILE}$(CC) ${LDFLAGS} -o aesdsocket server.c helper_server.c -lrt
	 
clean :
	rm -f aesdsocket
