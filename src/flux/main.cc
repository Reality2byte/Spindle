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


//This file is c++ just to fool the automake linker to use c++ linkage

extern "C" {
#include "sessionmgr.h"   
}
#include <cstdio>
#include <cstring>

int main(int argc, char *argv[])
{
   const char *session_dir = get_session_dir();
   if (argc >= 2 && strcmp(argv[1], "start") == 0) {
      char **new_argv = strip_start_from_argv(argc, argv);
      return spindle_session_start(argc - 1, new_argv, session_dir);
   }
   else if (argc >= 2 && strcmp(argv[1], "stop") == 0) {
      return spindle_session_stop(session_dir);
   }

   fprintf(stderr, "Usage: %s {start|stop} [SPINDLE OPTIONS]\n", argc ? argv[0] : "[NULL]");
   return -1;
}
