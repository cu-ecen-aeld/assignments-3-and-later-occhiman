#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>

//#define BUFFER_SIZE 24576u
#define BUFFER_SIZE 4096u

extern int xfer_data_server(int srcfd, int tgtfd);