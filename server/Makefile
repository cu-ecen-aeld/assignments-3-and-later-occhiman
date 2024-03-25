#
# Makefile of writer app
#

ARCH ?= arm64
CC := gcc
CFLAGS := -g
LDFLAGS := -pthread -lpthread
CROSS_COMPILE := aarch64-none-linux-gnu-

all : server.c helper_server.c
	${CROSS_COMPILE}$(CC) ${LDFLAGS} -o aesdsocket server.c helper_server.c -lrt
	 
clean :
	rm -f aesdsocket
