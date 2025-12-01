#include <pthread.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <signal.h>

void onsig(int sig)
{
   exit(-1);
}

void *thrdrun(void *arg)
{
   void *result;

   //Any audit-triggering operation will do
   result = dlopen("libm.so.6", RTLD_NOW);
   if (!result) {
      return NULL;
   }

   return NULL;
}

int main(int argc, char *argv[])
{
   pthread_t thrd;
   int result;

   signal(SIGSEGV, onsig);
   
   result = pthread_create(&thrd, NULL, thrdrun, NULL);
   if (result != 0) {
      return -1;
   }

   pthread_join(thrd, NULL);
   return 0;
}
