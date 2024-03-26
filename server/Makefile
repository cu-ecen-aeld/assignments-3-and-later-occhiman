#
# Makefile of writer app
#

ARCH ?= arm64
CC ?= ${CROSS_COMPILE}gcc
CFLAGS ?= -g
LDFLAGS ?= -pthread -lpthread

all : server.c helper_server.c
	$(CC) ${LDFLAGS} -o aesdsocket server.c helper_server.c -lrt
	 
clean :
	rm -f aesdsocket
