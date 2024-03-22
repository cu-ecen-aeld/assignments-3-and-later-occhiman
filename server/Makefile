#
# Makefile of writer app
#

ARCH ?= arm64
CC := gcc
CFLAGS := -g
LDFLAGS := -pthread -lpthread -lrt

all : server.c helper_server.c
	${CROSS_COMPILE}$(CC) ${LDFLAGS} -o aesdsocket server.c helper_server.c
	 
clean :
	rm -f aesdsocket
