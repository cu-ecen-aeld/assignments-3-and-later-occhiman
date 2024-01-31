#include <syslog.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main (int argc, char **argv)
{
   int returnVal;
   setlogmask (LOG_UPTO (LOG_DEBUG));
   
   if (argv[1] == NULL || argv[2] == NULL)
   {
       printf("Error! Not enough arguments\n");
      syslog (LOG_ERR, "Non valid number of arguments");
      return 1;
   }

   openlog ("exampleprog", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

   //syslog (LOG_NOTICE, "Program started by User %d", getuid ());
   
   FILE *out_file = fopen(argv[1], "w"); // write only
          
   // test for files not existing.
   if (out_file == NULL)
   {  
      printf("Error! Could not open file\n");
      syslog (LOG_ERR, "File couldn't be opened");
      return 1;
   }
          
   // write to file
   returnVal = fprintf(out_file, "%s\n", argv[2]);
   if (returnVal > 0)
   {
     printf("Successfull file writing\n");
     syslog (LOG_INFO, "Successful file writing");
   }
   else
   {
      printf("Unsuccessfull file writing\n");
      syslog (LOG_ERR, "File couldn't be written");
      return 1;
   }
    
   fclose(out_file);
   closelog ();
   
   return 0;

}



