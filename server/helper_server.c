#include "helper_server.h"

int xfer_data_server(int srcfd, int tgtfd)
{
    const char buf[BUFFER_SIZE];
    char *readBuf;
    int count = 0;
    int len = 0;
    int size = 0;
    int prev = 0;
    size_t ret;
    FILE *fp;
    unsigned char fileOpenFlag = 0;
    pthread_mutex_t *server_mutex;

    server_mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    
    //memset(server_mutex,0u,sizeof(pthread_mutex_t));
    pthread_mutex_init(server_mutex, NULL);
    
    /* Read socket data until socket connection is broken */
    while ((count = read(srcfd, (void *) buf, sizeof(buf))) > 0)
    {
       //printf("number of read bytes %d\n", count);
       if (len < 0)
        {
           perror("helper.c:xfer_data:read");
           exit(EXIT_FAILURE);
        }

        // obtain mutex
        if ( pthread_mutex_lock(server_mutex) != 0 ) 
        {
           ERROR_LOG ("pthread_mutex_lock failed");
           perror("Server Mutex log failed \n");
           exit(EXIT_FAILURE);
        }
        
        if((len = write(tgtfd, buf, count)) != count)
        {
            perror("helper.c:xfer_data:write");
            exit(EXIT_FAILURE);
        }

        /* check if the file was previously opened */
        if (!fileOpenFlag)
        {
            fp = fopen ("/var/tmp/aesdsocketdata","r+");
            if (fp == NULL)
            {
                perror("Open file file error");
                exit(EXIT_FAILURE);
            }

            fileOpenFlag = 1;
        }

        /* Check file size based on current file stream pointer */
        prev = ftell(fp);
        fseek(fp, 0L, SEEK_END);
        size = ftell(fp) - prev;
        fseek(fp,prev,SEEK_SET);

        readBuf = (char *) calloc(size,sizeof(char));
        if (readBuf == NULL)
        {
           printf("Memory not allocated.\n");
           exit(EXIT_FAILURE);
        }

        //printf("Size of temp file is %d\n",size);
        /* read tmp file content based on actual file pointer location */
        ret = fread(readBuf, sizeof(*readBuf), size, fp);
        //printf("Number of read bytes in tmp file: %ld\n",ret);
        if (ret != size) 
        {
            perror("helper.c:xfer_data:read");
            exit(EXIT_FAILURE);
        }

        /* Place file pointer at the end of the current file*/
        fseek(fp, 0, SEEK_END);
        if (fp == NULL)
        {
           printf("fp is null.\n");
           exit(EXIT_FAILURE);
        }

        /* Send the tmp file data back to the socket client */
        if(write (srcfd, readBuf, size) != size)
        {
            perror("helper.c:xfer_data:write");
            exit(EXIT_FAILURE);
        }
        
        // release mutex  
        if ( pthread_mutex_unlock(server_mutex) != 0 ) 
        {
           ERROR_LOG("pthread_mutex_unlock failed");
           perror("Server mutex unlock failed\n");
           exit(EXIT_FAILURE);
        }

        DEBUG_LOG("pthread_mutex unlock successfull");

        pthread_mutex_destroy(server_mutex);
        
        free(readBuf);

    }

    printf("Socket connection closed\n");
    fclose(fp);
    
    return 0;

}