#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    
    //printf("New thread started, PID %d TID %d\n", getpid(), (pid_t)syscall(SYS_gettid));
    DEBUG_LOG("New thread started");
    //thread_func_args->thread_id = syscall(SYS_gettid);
    // wait
    if (thread_func_args == NULL)
    {
        ERROR_LOG ("Malloc error");
        return thread_param;
    }

    usleep(thread_func_args->wait_to_obtain_ms * 1000);
    
    // obtain mutex
    if ( pthread_mutex_lock(thread_func_args->mutex) != 0 ) {
         ERROR_LOG ("pthread_mutex_lock failed");
         thread_func_args->thread_complete_success = false;
         return thread_param;
    }

      // wait to release
    usleep(thread_func_args->wait_to_release_ms * 1000);

    // release mutex  
    if ( pthread_mutex_unlock(thread_func_args->mutex) != 0 ) {
            ERROR_LOG ("pthread_mutex_unlock failed");
            thread_func_args->thread_complete_success = false;
            return thread_param;
    }

    DEBUG_LOG("pthread_mutex unlock successfull");
    thread_func_args->thread_complete_success = true;


    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */

    struct thread_data* data_shared = (struct thread_data*) malloc(sizeof(struct thread_data));
    int rc = 0;
 
    // malloc to pass data to the thread
    DEBUG_LOG("Main thread, PID TID", getpid(), (pid_t)syscall(SYS_gettid));

    if (data_shared == NULL)
    {
        ERROR_LOG("Malloc error");
        return false;
    }

    data_shared->wait_to_obtain_ms = wait_to_obtain_ms;
    data_shared->wait_to_release_ms = wait_to_release_ms;
    data_shared->mutex = mutex;

    DEBUG_LOG("Mutex successfully initialized");

    // start the thread         
    rc = pthread_create(thread, NULL, threadfunc, (void *) data_shared);
    if (rc != 0)
    {
        ERROR_LOG("Error creating thread");
        return false;
    }
    DEBUG_LOG("Thread created");

    //return the status of threat creation, true or false
    return true;
}

