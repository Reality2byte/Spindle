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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

static pid_t (*getpid_fptr)();
static int (*open_fptr)(const char *, int flags, ...);
static ssize_t (*read_fptr)(int fd, void *buffer, size_t len);
static int (*close_fptr)(int fd);
static void* (*memset_fptr)(void *s, int c, size_t n);
static ssize_t (*readlink_fptr)(const char *path, char *buf, size_t size);
static void (*perror_fptr)(const char *s);
static int (*fprintf_fptr)(FILE *stream, const char *format, ...);

static int do_file_ops()
{
   char buffer[4096];
   int fd;
   int result;
   pid_t pid = getpid_fptr();

   fd = open_fptr("/proc/self/maps", O_RDONLY);
   if (fd == -1) {
      perror_fptr("Could not open");
      return -1;
   }
   result = read_fptr(fd, buffer, 8);
   if (result == -1) {
      perror_fptr("Could not read");
      return -1;
   }

   close_fptr(fd);

   memset_fptr(buffer, 0, sizeof(buffer));
   result = readlink_fptr("/proc/self/exe", buffer, sizeof(buffer));
   if (result == -1) {
      perror_fptr("Could not readlink");
      return -1;
   }

   if (pid == 0) {
      fprintf_fptr(stderr, "Unexpected pid is 0\n");
      return -1;
   }
   return 0;      
}

static void dowork() __attribute__((constructor));
static void dowork()
{
   getpid_fptr = getpid;
   open_fptr = open;
   read_fptr = read;
   close_fptr = close;
   memset_fptr = memset;
   readlink_fptr = readlink;
   perror_fptr = perror;
   fprintf_fptr = fprintf;
   
   do_file_ops();
}
