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

static int do_file_ops()
{
   char buffer[4096];
   struct stat buf;
   int fd;
   int result;
   pid_t pid = getpid();

   fd = open("/proc/self/maps", O_RDONLY);
   if (fd == -1) {
      perror("Could not open");
      return -1;
   }
   result = fstat(fd, &buf);
   if (result == -1) {
      perror("Could not fstat");
      return -1;
   }

   result = read(fd, buffer, 8);
   if (result == -1) {
      perror("Could not read");
      return -1;
   }

   close(fd);

   result = stat("/proc/self/status", &buf);
   if (result == -1) {
      perror("Could not stat");
      return -1;
   }

   result = lstat("/proc/self/exe", &buf);
   if (result == -1) {
      perror("Could not lstat");
      return -1;
   }

   memset(buffer, 0, sizeof(buffer));
   result = readlink("/proc/self/exe", buffer, sizeof(buffer));
   if (result == -1) {
      perror("Could not readlink");
      return -1;
   }

   if (pid == 0) {
      fprintf(stderr, "Unexpected pid is 0\n");
      return -1;
   }
   return 0;      
}
