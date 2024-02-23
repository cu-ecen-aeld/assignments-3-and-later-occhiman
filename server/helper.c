#include <unistd.h>
#include "helper.h"

void xfer_data(int srcfd, int tgtfd)
{
    char buf[1024];
    int count, len;
    /*read from input file and write to output file*/
    while ((count = read(srcfd, buf, sizeof(buf))) > 0)
    {
        if (len < 0)
        perror("helper.c:xfer_data:read");
        if((len = write(tgtfd, buf, count)) != count)
        perror("helper.c:xfer_data:write");
    }

}