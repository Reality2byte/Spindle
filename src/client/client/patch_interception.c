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

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <link.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <sys/stat.h>

/**
 * This source file is mostly self contained so it can be compiled and run as a autoconf test.
 * Installing assembly patches is dangerous business, and we'd like to check the system for
 * compatibility before doing this by doing an autoconf tes.t
 * The CONFIG_TEST macro will be enabled during the autoconf test, and will turn this file into
 * a standalone test program.
 **/
#if !defined(CONFIG_TEST)
#include "spindle_debug.h"
#else
#define err_printf(...)
#define debug_printf(...)
#define debug_printf2(...)
#define debug_printf3(...)
#endif

static int get_patch_asm(void *patch_to, unsigned char *buffer, int buffer_size, int *patch_size)
{
   unsigned char *start, *end;
   int i;
   uint64_t key = 0;
#if defined(__x86_64)
   key = 0xcccccccccccccccc;
   __asm__ volatile ("jmp 2f\n"
           "1:\n"
           "movq $0xcccccccccccccccc, %%rax\n"
           "jmpq *%%rax\n"
           "2:\n"
           "movq $1b-get_patch_asm, %0\n"
           "movq $2b-get_patch_asm, %1\n"
           : "=r" (start), "=r" (end));
   start = start + (uint64_t) get_patch_asm;
   end = end + (uint64_t) get_patch_asm;
#else
   err_printf("Incompatible architecture for ld.so patching\n");
   return -1;
#endif
         
   *patch_size = end - start;
   if (*patch_size > buffer_size) {
      err_printf("ld.so patching failed, patch_size of %lu was greater than buffer sizeof %lu\n",
                 (unsigned long) *patch_size, (unsigned long) buffer_size);
      return -1;
   }
   memcpy(buffer, start, *patch_size);

   for (i = 0; i < *patch_size; i++) {
      if (memcmp(buffer + i, &key, sizeof(key)) == 0) {
         memcpy(buffer + i, &patch_to, sizeof(void*));
         return 0;
      }
   }
   
   err_printf("ld.so patching failed. Did not find key in asm buffer\n");
   return -1;
}

static int install_patch(void *patch_from, void *patch_to)
{
   unsigned char buffer[1024];
   int patch_size, result;
   size_t memregion_size;
   void *pagestart;

   debug_printf2("Installing asm patch to ld.so at %p that jumps to %p\n", patch_from, patch_to); 
   result = get_patch_asm(patch_to, buffer, sizeof(buffer), &patch_size);
   if (result == -1) {
      return -1;
   }

   pagestart = (void *) (((uint64_t) patch_from) & ~(((uint64_t) getpagesize())-1));
   memregion_size = (((size_t) patch_from) + patch_size) - (size_t) pagestart;
   result = mprotect(pagestart, memregion_size, PROT_READ | PROT_WRITE | PROT_EXEC);
   if (result == -1) {
      err_printf("Could not mprotect ld.so patch at %p for size %lu\n", pagestart, (unsigned long) memregion_size);
      return -1;
   }

   memcpy(patch_from, buffer, patch_size);

   result = mprotect(pagestart, memregion_size, PROT_READ  | PROT_EXEC);
   if (result == -1) {
      err_printf("Could not mprotect restore ld.so at %p for size %lu\n", pagestart, (unsigned long) memregion_size);
      return -1;
   }
   debug_printf("Successfully installed patch\n");
   return 0;
}

static void* get_ldso_base() {
   return (void*) _r_debug.r_ldbase;
}

int install_ldso_patch(unsigned long ldso_offset, void *patch_to)
{
   void *patch_from;
   patch_from = (void*) (((char *) get_ldso_base()) + ldso_offset);
   return install_patch(patch_from, patch_to);
}

int *calc_ldso_errno(unsigned long errno_offset)
{
   return (int *) (((unsigned char *) get_ldso_base()) + errno_offset);
}




#if defined(CONFIG_TEST)
static int *rtlderrno = NULL;

int num_slashes = 0;
int myxstat(int vers, const char *name, struct stat *buf)
{
   int result;
   int i;
   
   for (i = 0; name[i]; i++) {
      if (name[i] == '/')
         num_slashes++;
   }
   memset(buf, 0, sizeof(struct stat));
          
   result = stat(name, buf);
   if (result == -1) {
      if (*rtlderrno)
         *rtlderrno = errno;
      return -1;
   }
   return 0;
}

int mylxstat(int vers, const char *name, struct stat *buf)
{
   int result;
   int i;

   for (i = 0; name[i]; i++) {
      if (name[i] == '/')
         num_slashes++;
   }
   memset(buf, 0, sizeof(struct stat));
   
   result = lstat(name, buf);
   if (result == -1) {
      if (*rtlderrno)
         *rtlderrno = errno;
      return -1;
   }
   return 0;
}

static void on_fault(int sig)
{
   exit(-1);
}

int main(int argc, char *argv[])
{
   unsigned long stat_offset, lstat_offset, errno_offset;
   if (argc != 4) {
      return -1;
   }
   signal(SIGSEGV, on_fault);
   signal(SIGBUS, on_fault);
   
   errno = 0;
   stat_offset = strtol(argv[1], NULL, 16);
   if (errno) return -1;
   lstat_offset = strtol(argv[2], NULL, 16);
   if (errno) return -1;
   errno_offset = strtol(argv[3], NULL, 16);
   if (errno) return -1;

   rtlderrno = calc_ldso_errno(errno_offset);
   
   install_ldso_patch(stat_offset, myxstat);
   install_ldso_patch(lstat_offset, mylxstat);

   dlopen("libfoo.so", RTLD_NOW);
   dlopen("/not/exist/libbar.so", RTLD_NOW);
   dlopen("libm.so", RTLD_NOW);
   dlopen("libpthread.so", RTLD_NOW);
   if (num_slashes) {
      return 0;
   }
   return -1;
          
}
#endif
