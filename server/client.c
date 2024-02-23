#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include "helper.h"


int main (int argc, char *argv[])
{
   int socket_descriptor;
   struct sockaddr_in client_st;
   socklen_t address_size;
   setlogmask (LOG_UPTO (LOG_DEBUG));

if (argc != 2)
{
    puts("Wrtie_tcpip <IP Address>");
    syslog (LOG_ERR, "Non valid number of arguments");
    exit(EXIT_FAILURE);
}

/* Socket creation */
if ((socket_descriptor = socket(PF_INET, SOCK_STREAM, 0)) < 0)
{
    syslog(LOG_ERR, "Socket opening error");
    perror("socket opening error");
}
/* Client configuration */
memset(&client_st, 0, sizeof(client_st));
client_st.sin_family = AF_INET;
client_st.sin_port = htons (9000);
if (!(inet_aton(argv[1],&client_st.sin_addr)))
{
    /*not valid IP address*/
    syslog (LOG_ERR, "Non valid IP Address");
    perror("unvalid address");
}

address_size = sizeof(client_st);
if(connect(socket_descriptor, (struct sockaddr *)&client_st, address_size))
{
    syslog(LOG_ERR, "Connection error");
    perror("connection error");
}

puts("Connection a socket TCP/IP");
printf("\tPort %d\n", ntohs(client_st.sin_port));
printf("\tAddress %s\n", inet_ntoa(client_st.sin_addr));

xfer_data(fileno(stdin), socket_descriptor);
exit(EXIT_SUCCESS);

}