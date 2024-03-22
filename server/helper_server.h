#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdbool.h>
#include "list.h"
#include <sys/queue.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

//#define BUFFER_SIZE 24576u
#define BUFFER_SIZE 4096u

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

 struct entry {
           bool completed;
           int  socket_descriptor;
           pthread_t *data;
           pthread_mutex_t mutex;
           SLIST_ENTRY(entry) entries;              /* List */
};

struct thread_data
{
    /**
    * a mutex to use when accessing the structure
    */
    pthread_mutex_t lock;
};

//extern SLIST_HEAD(listhead, entry);

extern int xfer_data_server(int srcfd, int tgtfd);