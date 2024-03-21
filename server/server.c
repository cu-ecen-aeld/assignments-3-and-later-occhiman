#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>
#include  <signal.h>
#include "helper_server.h"


void intHandler(int dummy) {
    syslog(LOG_DEBUG, "Caught signal, exiting");

    if (remove("/var/tmp/aesdsocketdata") == 0)
    {
        syslog(LOG_DEBUG,"aesdsocket tmp file was deleted\n");
    }
    else
    {
        syslog(LOG_DEBUG,"unable to delete aesdsocket tmp file\n");
    }
    exit(EXIT_SUCCESS);
}

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    int fd, dataAtchivDescriptor, ret;
    struct entry  *thread_func_args = (struct entry *) thread_param;
    
    //printf("New thread started, PID %d TID %d\n", getpid(), (pid_t)syscall(SYS_gettid));
    DEBUG_LOG("New thread started");
    //thread_func_args->thread_id = syscall(SYS_gettid);
    // wait
    if (thread_func_args == NULL)
    {
        ERROR_LOG ("Unvalid thread reference");
        exit(EXIT_FAILURE);
    }

    // obtain mutex
    //if ( pthread_mutex_lock(&thread_func_args->mutex) != 0 ) 
    //{
      //  ERROR_LOG ("pthread_mutex_lock failed");
        //exit(EXIT_FAILURE);
        //usleep(10);
    //}

    fd = open("/var/tmp/aesdsocketdata", O_WRONLY | O_APPEND, 0644);
    if (fd < 0)
    {
        perror("Open file file error");
        exit(EXIT_FAILURE);
    }
        
    /* read socket and write to /var/temp/aesdsocketdata */
    ret = xfer_data_server(thread_func_args->socket_descriptor, fd);
    if (ret)
    {
        perror("Server error");
        close(thread_func_args->socket_descriptor);
        exit(EXIT_FAILURE);
    }

    DEBUG_LOG("Succesfull socket Rx/Tx transaction");
    thread_func_args->completed = true;

    // release mutex  
    //if ( pthread_mutex_unlock(&thread_func_args->mutex) != 0 ) 
    //{
      //  ERROR_LOG ("pthread_mutex_unlock failed");
        //exit(EXIT_FAILURE);
    //}

    //DEBUG_LOG("pthread_mutex unlock successfull");

    close(fd);

    return thread_param;
}


/**
* A thread which runs every timer_period_ms milliseconds
* Assumes timer_create has configured for sigval.sival_ptr to point to the
* thread data used for the timer
*/
static void timer_thread ( union sigval sigval )
{
    struct thread_data *td = (struct thread_data*) sigval.sival_ptr;
    char outstr[100];
    time_t t;
    struct tm *tmp;
    int fd;
    int str_size;
    FILE *fp;

    t = time(NULL);
    tmp = localtime(&t);
    if (tmp == NULL) 
    {
        perror("localtime");
        exit(EXIT_FAILURE);
    }
        
    str_size = strftime(outstr, sizeof(outstr), "timestamp:%a, %d %b %Y %T %z \n", tmp);
    if (str_size == 0) 
    {
        fprintf(stderr, "strftime returned 0");
        exit(EXIT_FAILURE);
    }

    printf("Result string is \"%s\"", outstr);
    //printf("Size is \"%d\"", str_size);

    //if ( pthread_mutex_lock(&td->lock) != 0 ) 
    //{
      //  perror("Mutex lock error\n");
        //exit(EXIT_FAILURE);
    //} 

    fp = fopen ("/var/tmp/aesdsocketdata","r+");
    if (fp == NULL)
    {
        perror("Open file file error\n");
        exit(EXIT_FAILURE);
    }

    fseek(fp, 0, SEEK_END);
    if (fp == NULL)
    {
        printf("fp is null.\n");
        exit(EXIT_FAILURE);
    }

    if( fwrite(outstr,1u,str_size,fp) != str_size)
    {
            perror("Write file error \n");
            exit(EXIT_FAILURE);
    }
 
#if 0
        fd = open("/var/tmp/aesdsocketdata", O_WRONLY| O_CREAT| O_APPEND, 0644);
        if (fd < 0)
        {
           perror("Open file file error");
           exit(EXIT_FAILURE);
        }

        /* Print timestamp in tmp file */
        if(write (fd, outstr, str_size) != str_size)
        {
            perror("helper.c:xfer_data:write");
            exit(EXIT_FAILURE);
        }

#endif

    //if ( pthread_mutex_unlock(&td->lock) != 0 ) {
      //  perror("Mutex unlock error \n");
        //exit(EXIT_FAILURE);
    //}

    //printf("Timer mutex successfully unlocked \n");

    //close(fd);
    fclose(fp);
 
    
}

int runServer (void)
{
    int socket_descriptor = 0;
    int dataAtchivDescriptor = 0;
    struct sockaddr_in server_st;
    socklen_t addres_size;
    int ret = 0;
    int rc = 0;
    int fd = 0;
    pthread_t *thread;
    struct entry *connection_entry, *previous_connection, *actual_connection;
    SLIST_HEAD(listhead, entry);
    struct listhead head;
    bool firstConnectionDne = false;
    void *retVal;
    int numOfThreads = 0;

    SLIST_INIT(&head);                       /* Initialize the list */
    
    setlogmask (LOG_UPTO (LOG_DEBUG));

    printf("Socket server started\n");

    /* socket creation*/
    if ((socket_descriptor = socket (PF_INET, SOCK_STREAM, 0)) < 0)
    {
        syslog (LOG_ERR, "Error opening socket");
        perror("error opening socket");
        exit(EXIT_FAILURE);
    }
    
    /* server initialization */
    memset(&server_st, 0, sizeof(server_st));
    server_st.sin_family = AF_INET;
    server_st.sin_port = htons(9000);

    /* Bind socket to an address*/
    addres_size = sizeof(server_st);
    if ((bind(socket_descriptor, (struct sockaddr *)&server_st, addres_size)) < 0)
    {
        syslog (LOG_ERR, "Error binding");
        perror("error binding");
        exit(EXIT_FAILURE);
    }

    /* clean tmp file */
    fd = open("/var/tmp/aesdsocketdata", O_WRONLY| O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
    {
       perror("Open file file error");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        /* wait for existing connections */
        if ((listen(socket_descriptor, 5) ) < 0)
        {
            syslog (LOG_ERR, "Error listening");
            perror("error during listening");
            exit(EXIT_FAILURE);
        }
        
        puts("Available TCP/P socket");
        printf("\tPort %d\n", ntohs(server_st.sin_port));
        printf("\tAddress IP %s\n", inet_ntoa(server_st.sin_addr));
        
        /* Accept connection */
        if((dataAtchivDescriptor = accept(socket_descriptor, (struct sockaddr *)&server_st, &addres_size)) >=0)
        {
            syslog (LOG_DEBUG, "Accepted connection from %s", inet_ntoa(server_st.sin_addr));
            puts("new connection granted");
        }

        connection_entry = malloc(sizeof(struct entry));
        if (connection_entry == NULL)
        {
            printf("Malloc Error\n");
            exit(EXIT_FAILURE);
        }
        /* Initialize connection completed flag & data */
        //memset((bool *) connection_entry->completed, 0, sizeof(connection_entry->completed) );

        //connection_entry->data = 0;
        //connection_entry->socket_descriptor = 0;
        //connection_entry->completed = 0;
        
        /* First connection flag check */
        if (firstConnectionDne == false)
        {    

            /* Insert at the head */
            SLIST_INSERT_HEAD(&head, connection_entry, entries);

            connection_entry->socket_descriptor = dataAtchivDescriptor;
            previous_connection = connection_entry;

            firstConnectionDne = true;
            
        }
        else
        {
           
           /* Insert after */
           
           SLIST_INSERT_AFTER(previous_connection, connection_entry, entries);
           connection_entry-> socket_descriptor = dataAtchivDescriptor;
           previous_connection = connection_entry;
           
        }

        thread = (pthread_t *)malloc(sizeof(pthread_t));
        if (thread == NULL)
        {
            perror("Malloc error \n");
            exit(EXIT_FAILURE);
        }

        memset(thread,0,sizeof(pthread_t));

        // start the thread         
        rc = pthread_create(thread, NULL, threadfunc, (void *) connection_entry);
        if (rc != 0)
        {
            ERROR_LOG("Error creating thread");
            return false;
        }
        DEBUG_LOG("Thread created");
        connection_entry->data = (long int) thread; 

        //printf("Started thread with id %ld\n",connection_entry->data);

        //For thread in list
        SLIST_FOREACH(connection_entry, &head, entries)
        {
            printf("Thread id found: %ld\n", connection_entry->data);
            // check for completed flag
            if (connection_entry->completed == true)
            {
                // call pthread_join
                if (pthread_join(*thread, &retVal) != 0) 
                {
                    perror("pthread_create() error");
                    exit(EXIT_FAILURE);
                }
                    
                printf("Rx/Tx session finshed, thread exited \n");
            }
            numOfThreads++;

        }

        //SLIST_FOREACH_SAFE(connection_entry, &head, entries);
        printf ("Number of threads:%d\n", numOfThreads);

        actual_connection = SLIST_FIRST(&head);
        if (actual_connection->completed == true)
        {
           SLIST_REMOVE_HEAD(&head, entries);
           actual_connection->completed = false;
           free(actual_connection);
           numOfThreads--;
           firstConnectionDne = false;
        }

        while (numOfThreads > 1u)
        {
            actual_connection = SLIST_NEXT(actual_connection, entries);

            if (actual_connection->completed == true)
            {
                SLIST_REMOVE(&head, actual_connection, entry, entries);
                actual_connection->completed = false;
                free(actual_connection);
            }

            numOfThreads--;

        }
        SLIST_INIT(&head);
        free(connection_entry);

    } /* while(1) */

    return ret;
}

int timer_setup(timer_t timerid, struct thread_data td)
{
    struct sigevent sev;
    int res;

        if ( pthread_mutex_init(&td.lock,NULL) != 0 ) 
        {
            printf("Error %d (%s) initializing thread mutex!\n",errno,strerror(errno));
            exit(EXIT_FAILURE);
        } else 
        {
            int clock_id = CLOCK_MONOTONIC;
            memset(&sev,0,sizeof(struct sigevent));
            struct itimerspec its = {   .it_value.tv_sec  = 10,
                            .it_value.tv_nsec = 0,
                            .it_interval.tv_sec  = 10,
                            .it_interval.tv_nsec = 0
                        };
        
        /**
        * Setup a call to timer_thread passing in the td structure as the sigev_value
        * argument
        */
        sev.sigev_notify = SIGEV_THREAD;
        sev.sigev_value.sival_ptr = &td;
        sev.sigev_notify_function = timer_thread;
        if ( timer_create(clock_id,&sev,&timerid) != 0 ) {
            printf("Error %d (%s) creating timer!\n",errno,strerror(errno));
            exit(EXIT_FAILURE);
           
        } else {

            printf("timer was sucessfully created \n");
            
            /* start timer */
            res = timer_settime(timerid, 0, &its, NULL);
            if (res != 0)
            {
                fprintf(stderr, "Error timer_settime: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            printf("setting timer successful\n");
            
        }
    }

    return 0;

}

int main(int argc, char *argv[])
{
    pid_t pid;
    signal(SIGINT, intHandler);
    signal(SIGTERM, intHandler);
    signal(SIGSEGV, intHandler);
    int ret = 0;
    struct thread_data td;
    timer_t timerid;

    memset(&td,0,sizeof(struct thread_data));
#if(0)

    td = (struct thread_data *) malloc(sizeof(struct thread_data));
    if (td == NULL)
    {
        printf("Malloc error \n");
        exit(EXIT_FAILURE);
    }

    timerid = (timer_t *) malloc (sizeof(timer_t));
    if (timerid == NULL)
    {
        printf("Malloc error \n");
        exit(EXIT_FAILURE);
    }
    memset(&td,0,sizeof(struct thread_data));
    if ( pthread_mutex_init(&td.lock,NULL) != 0 ) {
        printf("Error %d (%s) initializing thread mutex!\n",errno,strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        int clock_id = CLOCK_MONOTONIC;
        memset(&sev,0,sizeof(struct sigevent));
        
        struct itimerspec its = {   .it_value.tv_sec  = 10,
                                .it_value.tv_nsec = 0,
                                .it_interval.tv_sec  = 10,
                                .it_interval.tv_nsec = 0
                            };
        
        /**
        * Setup a call to timer_thread passing in the td structure as the sigev_value
        * argument
        */
        sev.sigev_notify = SIGEV_THREAD;
        sev.sigev_value.sival_ptr = &td;
        sev.sigev_notify_function = timer_thread;
        if ( timer_create(clock_id,&sev,&timerid) != 0 ) {
            printf("Error %d (%s) creating timer!\n",errno,strerror(errno));
            exit(EXIT_FAILURE);
           
        } else {

            printf("timer was sucessfully created \n");
            
            /* start timer */
            res = timer_settime(timerid, 0, &its, NULL);
            if (res != 0)
            {
                fprintf(stderr, "Error timer_settime: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            
        }
    }
#endif
    
    if (argc < 2)
    {
        /* defaul normal process execution*/
        ret = timer_setup(timerid, td);
        if (ret)
        {
            perror("timer setup error\n");
            exit(EXIT_FAILURE);
        }
        ret = runServer();
    }
    else
    {
        //fwrite(argv[1], 2, 1, stdout);
        if (0 == strcmp(argv[1], "-d"))
        {
            switch (pid = fork()) {
            case -1:
            /* On error fork */
            perror("fork failed");
            exit(EXIT_FAILURE);
            
            case 0:
            /* child code here*/
            printf("Child deamon execution \n");
            ret = timer_setup(timerid, td);
            if (ret)
            {
               perror("timer setup error\n");
               exit(EXIT_FAILURE);
            }
            
            ret = runServer();
            exit(ret);
            
            default:
            /* parent code */
            printf("Exit Parent\n");
            exit(EXIT_SUCCESS);


            }

        }
        else 
        {
            printf("Argument not surpported\n");
            exit(EXIT_FAILURE);
        }   

    }

   return ret;

}
