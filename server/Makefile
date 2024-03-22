#
# Makefile of writer app
#

ARCH ?= arm64
CC := gcc
CFLAGS := -g

all : server.c helper_server.c
	$(CC) -o aesdsocket server.c helper_server.c
	 
clean :
	rm -f aesdsocket
