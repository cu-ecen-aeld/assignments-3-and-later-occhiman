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

//static const char workfile[] = {"/var/tmp/aesdsocketdata"};
static const char workfile[] = {"/dev/aesdchar"};


void intHandler(int dummy) {
    syslog(LOG_DEBUG, "Caught signal, exiting");

    /*
    if (remove(workfile) == 0)
    {
        syslog(LOG_DEBUG,"aesdsocket tmp file was deleted\n");
    }
    else
    {
        syslog(LOG_DEBUG,"unable to delete aesdsocket tmp file\n");
    }
    */
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

    fd = open(workfile, O_WRONLY | O_APPEND, 0644);
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

    //printf("Result string is \"%s\"", outstr);
    DEBUG_LOG(outstr);
    //printf("Size is \"%d\"", str_size);

    fp = fopen (workfile,"r+");
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
    int s;

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
    fd = open(workfile, O_WRONLY| O_CREAT | O_TRUNC, 0644);
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
        /* Initialize connection completed flag */
        connection_entry->completed = false;
        
        /* First connection flag check */
        if (firstConnectionDne == false)
        {    
            
            /* Insert at the head */
            SLIST_INSERT_HEAD(&head, connection_entry, entries);
            printf("New head in list was inserted \n");

            firstConnectionDne = true;
            
        }
        else
        {
           
           /* Insert after */
           SLIST_INSERT_AFTER(previous_connection, connection_entry, entries);
           
        }

        connection_entry->socket_descriptor = dataAtchivDescriptor;
        /* backup actual connection */
        previous_connection = connection_entry;

        thread = (pthread_t *)malloc(sizeof(pthread_t));
        if (thread == NULL)
        {
            perror("Malloc error \n");
            exit(EXIT_FAILURE);
        }

        // start the thread         
        rc = pthread_create(thread, NULL, threadfunc, (void *) connection_entry);
        if (rc != 0)
        {
            ERROR_LOG("Error creating thread");
            perror("Error creating thread");
            exit(EXIT_FAILURE);
        }
        DEBUG_LOG("Thread created");
        connection_entry->data = thread; 

        //printf("Started thread with id %ld\n",(long int) connection_entry->data);

        /* Set the first element on the list */
        actual_connection = SLIST_FIRST(&head);
        numOfThreads = 0;
        /* For thread in list */
        SLIST_FOREACH(actual_connection, &head, entries)
        {
            printf("Thread id found: %ld\n", (long int)actual_connection->data);
            // check for completed flag
            if (actual_connection->completed == true)
            {
                // call pthread_join
                s = pthread_join(*actual_connection->data, &retVal);
                if ( s != 0) 
                {
                    errno = s;
                    perror("pthread_join() error");
                    exit(EXIT_FAILURE);
                }

                actual_connection->completed = false;   
                printf("Rx/Tx session finshed, thread exited \n");
            }
            numOfThreads++;

        }

        printf ("Number of threads:%d\n", numOfThreads);

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
    
    if (argc < 2)
    {
        /* defaul normal process execution*/
        /*
        ret = timer_setup(timerid, td);
        if (ret)
        {
            perror("timer setup error\n");
            exit(EXIT_FAILURE);
        }
        */
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
            /*
            ret = timer_setup(timerid, td);
            if (ret)
            {
               perror("timer setup error\n");
               exit(EXIT_FAILURE);
            }
            */
            
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
