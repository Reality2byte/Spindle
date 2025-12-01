/*
This file is part of Spindle.  For copyright information see the COPYRIGHT 
file in the top level directory, or at 
https://github.com/hpc/Spindle/blob/master/COPYRIGHT

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License (as published by the Free Software
Foundation) version 2.1 dated February 1999.  This program is distributed in the
hope that it will be useful, but WITHOUT ANY WARRANTY; without even the IMPLIED
WARRANTY OF MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the terms 
and conditions of the GNU Lesser General Public License for more details.  You should 
have received a copy of the GNU Lesser General Public License along with this 
program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA
*/


//Tune these #defines to enable heap debugging
//#define HEAP_DEBUGGING_ENABLED
//#define CHECK_MEM
//#define REDZONE
//#define REDZONE_L
//#define CHECK_ALL

#ifdef HEAP_DEBUGGING_ENABLED

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <execinfo.h>
#include <sys/mman.h>

extern void *__libc_malloc(size_t);
extern void __libc_free(void *);
extern void *__libc_realloc(void *, size_t);
extern void *__libc_calloc(size_t nmemb, size_t size);

#define AUDIT_EXPORT __attribute__((__visibility__("default")))
void *malloc(size_t size) AUDIT_EXPORT;
void free(void *ptr) AUDIT_EXPORT;
void *realloc(void *ptr, size_t newsize) AUDIT_EXPORT;
void *calloc(size_t nmemb, size_t size) AUDIT_EXPORT;

#define HEADER 0xfe6a99423cc650f1
#define FOOTER 0xda4481c3c421ab65


#ifdef CHECK_MEM
static unsigned int count = 0;

static int should_print(int c) {
   return 0;
   //   return (c < 100 || c > 4000 || c % 100 == 0);
}
static void *mark_up_malloc(void *result, size_t size, int is_calloc)
{
   unsigned long *header, *footer, *sizep;
   sizep = (unsigned long *) result;
   header = ((unsigned long *) result) + 1;
   footer = (unsigned long *) (((unsigned char *) result) + 16 + size);
   *sizep = size;
   *header = HEADER;
   *footer = FOOTER;
   void *uresult = (void *) (((unsigned long *) result) + 2);
   if (should_print(count))
      fprintf(stderr, "%u. %s(%lu) = %p (%p)\n",
              count, is_calloc ? "calloc" : "malloc",
              size, uresult, result);
   count++;
   return uresult;
}

static ssize_t check(void *result, int is_free)
{
   unsigned long *header, *footer, *sizep;
   size_t size;
   unsigned long *resultp = (((unsigned long *) result) - 2);

   sizep = resultp;
   header = resultp+1;
   size = *sizep;
   footer = (unsigned long *) (((unsigned char *) resultp) + 16 + size);   

   if (*header != HEADER) {
      fprintf(stderr, "CORRUPT HEADER on %p\n", result);
   }
   if (*footer != FOOTER) {
      fprintf(stderr, "CORRUPT FOOTER on %p\n", result);
   }
   if (*header != HEADER || *footer != FOOTER) {
      return -1;
   }  

   if (is_free)
      memset(resultp, 0xac, size + 24);
   return size;
}
#endif

#define PAGE_SIZE 4096
#if defined(REDZONE) || defined(CHECK_ALL)
static void *page_alloc(size_t size)
{
   void *result;
   int iresult;
   size_t rsize;
#if defined(REDZONE_L) || defined(REDZONE_R)
   while (size % 16 != 0) size++;
   rsize = size + PAGE_SIZE;
#else
   rsize = size;
#endif
   result = mmap(NULL, rsize, PROT_READ|PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
   if (result == MAP_FAILED) {
      fprintf(stderr, "[MMAP FAILED DURING ALLOC]\n");
      return NULL;
   }
#if defined(REDZONE_L)
   iresult = mprotect(result, PAGE_SIZE, PROT_NONE);
   if (iresult == -1) {
      printf("mprotect failed\n");
      return NULL;
   }
   return (void *) (((unsigned char *) result) + PAGE_SIZE);
#elif defined(REDZONE_R)
   unsigned long last_page_addr = ((unsigned long) result) + rsize;
   unsigned long last_page = last_page_addr & ~(PAGE_SIZE - 1);
   if (last_page_addr == last_page)
      last_page = last_page_addr - PAGE_SIZE;
   iresult = mprotect((void*) last_page, 4096, PROT_NONE);
   if (iresult == -1) {
      printf("mprotect failed\n");
      return NULL;
   }
   unsigned char *cur = result;
   while (cur + size != (unsigned char*) last_page) cur++;
   return (void *) cur;
#else
   return result;
#endif
   
}
#endif
#if defined(REDZONE)
static void page_free(void *mem, size_t size) {
   int result;
   unsigned long addr = (unsigned long) mem;
   addr &= ~(PAGE_SIZE - 1);
   result = mprotect((void *) addr, size, PROT_NONE);
   if (result == -1) {
      printf("mprotect error\n");
   }
}
#endif

#if defined(CHECK_ALL)
#define NUM_BUFFERS 4096
#define BUFFER_SIZE 131072
static void** all_buffers[4096];

void check_all(void *ptr)
{
   unsigned int i, j;
   unsigned int num_checked = 0;
   void **found = NULL;
   for (i = 0; i < NUM_BUFFERS && all_buffers[i]; i++) {
      void **cur_buffer = all_buffers[i];
      for (j = 0; j < BUFFER_SIZE; j++) {
         if (!cur_buffer[j])
            continue;
         if (cur_buffer[j] == ptr) {
            if (found) {
               fprintf(stderr, "[CHECK ALL ERROR - Ptr %p found twice in buffer list]\n", ptr);               
            }
            found = cur_buffer + j;
         }
         num_checked++;
         check(cur_buffer[j], 0);
      }
   }
   if (found)
      *found = NULL;
   if (should_print(count))
      fprintf(stderr, "Checked %u live allocs\n", num_checked);
   
}

void add_to_check_all(void *ptr)
{
   static int errd = 0;
   unsigned int i, j;
   for (i = 0; i < NUM_BUFFERS; i++) {
      void **cur_buffer = all_buffers[i];
      if (cur_buffer == NULL) {
         all_buffers[i] = page_alloc(BUFFER_SIZE * sizeof(void*));
         all_buffers[i][0] = ptr;
         return;
      }
      for (j = 0; j < BUFFER_SIZE; j++) {
         if (!cur_buffer[j]) {
            cur_buffer[j] = ptr;
            return;
         }
      }
   }
   if (!errd) {
      fprintf(stderr, "[CHECK ALL ERROR - Buffer list full\n]");
      errd = 1;
   }
}

#endif

void *malloc(size_t size)
{
   void *result;
#ifdef CHECK_MEM
# ifdef CHECK_ALL
   check_all(NULL);
# endif   
# ifdef REDZONE
   result = page_alloc(size + 24);
# else
   if (should_print(count))
      fprintf(stderr, "%u. malloc start(%lu)\n", count, size);
   result = __libc_malloc(size + 24);
# endif
   result = mark_up_malloc(result, size, 0);
# ifdef CHECK_ALL
   add_to_check_all(result);
# endif
#else
   result = __libc_malloc(size);
#endif
   
   return result;
}

void free(void *ptr) {
#ifdef CHECK_MEM
   ssize_t result;
# ifdef CHECK_ALL
   check_all(ptr);
# endif   
   if (should_print(count))
      fprintf(stderr, "%u. free(%p)\n", count, ptr);  
   if (!ptr)
      return;
   result = check(ptr, 1);
   if (result == -1) {
      __libc_free(ptr);
      return;
   }

# ifdef REDZONE
   page_free(((unsigned char *) ptr) - 16, result + 24);
# else
   __libc_free(((unsigned char *) ptr) - 16);
# endif
#else
   __libc_free(((unsigned char *) ptr));   
#endif   
}

void *realloc(void *ptr, size_t newsize)
{
#ifdef CHECK_MEM
   void *newbuf;
   ssize_t result;

   if (!ptr) {
      void *mresult = malloc(newsize);
      if (should_print(count))   
         fprintf(stderr, "%u. realloc(%p, %lu) = %p\n", count, ptr, newsize, mresult);
      return mresult;
   }
   if (ptr && !newsize) {
      if (should_print(count))   
         fprintf(stderr, "%u. realloc(%p, %lu) = %p\n", count, ptr, newsize, NULL);      
      free(ptr);
      return NULL;
   }
      
   result = check(ptr, 0);
   if (result == -1) {
      return __libc_realloc(ptr, newsize);
   }
   newbuf = malloc(newsize);
   memcpy(newbuf, ptr, result);
   if (should_print(count))   
      fprintf(stderr, "realloc(%p, %lu) = %p\n", ptr, newsize, newbuf);
   free(ptr);
   return newbuf;
#else
   return __libc_realloc(ptr, newsize);
#endif
}

void *calloc(size_t nmemb, size_t size) {
#ifdef CHECK_MEM
   void *newptr;
# if defined(REDZONE)
   newptr = page_alloc((nmemb * size) + 24);
# else
   newptr = __libc_malloc((nmemb * size) + 24);
#endif
   newptr = mark_up_malloc(newptr, nmemb * size, 1);
   memset(newptr, 0, nmemb * size);
   return newptr;
#else
   return __libc_calloc(nmemb, size);
#endif
}

#endif
