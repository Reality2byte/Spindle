#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char placeholder[256] =
"SPINDLE_PLACEHOLDERxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

int check_hostname()
{
   char hostname[256];
   memset(hostname, 0, sizeof(hostname));
   gethostname(hostname, sizeof(hostname));
   hostname[sizeof(hostname)-1] = '\0';

   if (strcmp(placeholder, hostname) != 0) {
      fprintf(stderr, "ERROR: Problem during liblocal test, placeholder name did not match hostname: %s != %s\n",
              placeholder, hostname);
      return 0;
   }
   return 1;
}

