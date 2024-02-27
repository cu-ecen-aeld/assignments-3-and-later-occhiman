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
    exit(0);
}

int runServer (void)
{
    int socket_descriptor, dataAtchivDescriptor;
    struct sockaddr_in server_st;
    socklen_t addres_size;
    int fd, ret;

    setlogmask (LOG_UPTO (LOG_DEBUG));

    printf("Socket server started\n");

    /* socket creation*/
    if ((socket_descriptor = socket (PF_INET, SOCK_STREAM, 0)) < 0)
    {
        syslog (LOG_ERR, "Error opening socket");
        perror("error opening socket");
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
        
        fd = open("/var/tmp/aesdsocketdata", O_WRONLY| O_CREAT | O_APPEND, 0644);
        if (fd < 0)
        {
            perror("Open file file error");
            exit(EXIT_FAILURE);
        }
        
        /* read socket and write to /var/temp/aesdsocketdata */
        ret = xfer_data_server(dataAtchivDescriptor, fd);
        if (ret)
        {
            perror("Server error");
            close(dataAtchivDescriptor);
            close(socket_descriptor);
            exit(EXIT_FAILURE);
        }

        close(fd);

    } /* while(1) */

    return ret;
}

int main(int argc, char *argv[])
{
    pid_t pid;
    signal(SIGINT, intHandler);
    signal(SIGTERM, intHandler);
    signal(SIGSEGV, intHandler);
    int ret;
    
    if (argc < 2)
    {
        /* defaul normal process execution*/
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