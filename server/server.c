#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <fcntl.h>
#include "helper.h"

int main(void)
{
    int socket_descriptor, dataAtchivDescriptor;
    struct sockaddr_in server_st;
    socklen_t addres_size;
    int fd;
    setlogmask (LOG_UPTO (LOG_DEBUG));

    printf("Socket server started\n");

    /* socket creation*/
    if ((socket_descriptor = socket (PF_INET, SOCK_STREAM, 0)) < 0)
    {
        syslog (LOG_ERR, "Error opening socket");
        perror("error opening socket");
    }
    

    /*server initialization */
    memset(&server_st, 0, sizeof(server_st));
    server_st.sin_family = AF_INET;
    server_st.sin_port = htons(9000);

    /* Bind socket to an address*/
    addres_size = sizeof(server_st);
    if ((bind(socket_descriptor, (struct sockaddr *)&server_st, addres_size)) < 0)
    {
        syslog (LOG_ERR, "Error binding");
        perror("error binding");
    }
    
    /* wait for existing connections */
    if ((listen(socket_descriptor, 5) ) < 0)
    {
        syslog (LOG_ERR, "Error listening");
        perror("error lisening");
    }
    
    puts("Available TCP/P socket");
    printf("/tPort %d\n", ntohs(server_st.sin_port));
    printf("/tAddress IP %s\n", inet_ntoa(server_st.sin_addr));

    /* Accept connection */
    if((dataAtchivDescriptor = accept(socket_descriptor, (struct sockaddr *)&server_st, &addres_size)) >=0)
    {
        syslog (LOG_DEBUG, "Accepted connection from %s", inet_ntoa(server_st.sin_addr));
        puts("new connection granted");
    }
    
    fd = open("/var/tmp/aesdsocketdata", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
    {
        perror("Open file file error");
        exit(1);
    }
    /* read socket and write to stdout */
    xfer_data(dataAtchivDescriptor, fd);

    close(fd);

    exit(EXIT_SUCCESS);





}