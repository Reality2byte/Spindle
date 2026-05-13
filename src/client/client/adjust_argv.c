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

#include "config.h"
#include "client_api.h"
#include "client.h"

extern int is_in_spindle_cache(const char *pathname);
extern int ldcsid;

#if defined(HAVE_DL_ARGV)
extern char **_dl_argv;
static char **get_argv()
{
   return _dl_argv;
}
#else
static char **get_argv()
{
   return NULL;
}
#endif

static void do_adjust(int arg_i, char **argv)
{
   int result;
   char newpath[MAX_PATH_LEN+1], *s;

   if (!argv[arg_i])
      return;

   debug_printf2("Checking if argv[%d] (%s) needs to be fixed up\n", arg_i, argv[0]);
   if (!is_in_spindle_cache(argv[arg_i])) {
      return;
   }

   newpath[0] = '\0';
   result = send_orig_path_request(ldcsid, argv[arg_i], newpath);
   if (result == -1) {
      debug_printf("Warning, not adjusting argv because server communication failed\n");
      return;
   }

   if (strlen(argv[arg_i]) > strlen(newpath)) {
      strcpy(argv[arg_i], newpath);
      debug_printf2("Replaced argv[%d] with %s in place\n", arg_i, newpath);
   }
   else {
      s = strdup(newpath);
      argv[arg_i] = s;
      debug_printf2("Replaced argv[%d] with %s from heap\n", arg_i, s);
   }
}

void adjust_argv_if_needed()
{
   char **argv;
   
   argv = get_argv();
   if (!argv) {
      return;
   }

   do_adjust(0, argv);
   do_adjust(1, argv);
}
