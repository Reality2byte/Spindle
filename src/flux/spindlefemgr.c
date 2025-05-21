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

#include "spindlemgr.h"
#include "fluxmgr.h"
#include "procmgr.h"
#include "spindle_launch.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static char *encode_daemon_args(spindle_args_t *params);
static char *encode_bootstrap_args(int bootstrap_argc, char **bootstrap_argv);

int setup_args_on_fe(spindle_args_t *params, int is_default_session)
{
   char *daemon_args_str, *bootstrap_args_str;
   int bootstrap_argc;
   char **bootstrap_argv;
   char *session_arg = NULL;
   int result;

   spindle_debug_printf(2, "Getting daemon and bootstrap arguments\n");   
   result = getApplicationArgsFE(params, &bootstrap_argc, &bootstrap_argv);
   if (result == -1) {
      spindle_debug_printf(1, "ERROR: Could not get spindle bootstrap args\n");
      fluxmgr_close();      
      return -1;
   }
   bootstrap_args_str = encode_bootstrap_args(bootstrap_argc, bootstrap_argv);
   daemon_args_str = encode_daemon_args(params);

   if (is_default_session)
      session_arg = params->session_key;
   
   spindle_debug_printf(2, "Adding daemon and bootstrap args to flux KVS\n");   
   result = fluxmgr_add_to_kvs(daemon_args_str, bootstrap_args_str, session_arg);
   if (result == -1) {
      spindle_debug_printf(1, "ERROR: Could not add args to flux kvs\n");
      fluxmgr_close();      
      return -1;
   }

   free(daemon_args_str);
   free(bootstrap_args_str);
   
   return 0;
}

int run_fe(int argc, char **argv)
{
   spindle_args_t params;
   int result;
   char *errstr;
   char **hosts;

   spindle_debug_printf(1, "Initializing spindle fe in run_fe\n");
   result = fluxmgr_init();
   if (result == -1) {
      spindle_debug_printf(1, "ERROR: Could not init flux mgr\n");
      return -1;
   }

   memset(&params, 0, sizeof(params));
   result = fillInSpindleArgsCmdlineFE(&params, 0, argc, argv, &errstr);
   if (result == -1) {
      spindle_debug_printf(1, "ERROR: Could not parse spindle args: %s\n", errstr);
      fprintf(stderr, "Spindle session argument parsing error: %s\n", errstr);
      fluxmgr_close();
      return -1;
   }

   spindle_debug_printf(2, "Getting hostlist from flux\n");
   hosts = fluxmgr_get_hostlist();
   if (!hosts) {
      spindle_debug_printf(1, "ERROR: Could not get hostlist\n");
      fluxmgr_close();      
      return -1;
   }

   result = setup_args_on_fe(&params, 1);
   if (result == -1) {
      fluxmgr_close();
      return -1;
   }
   
   fluxmgr_close();      

   spindle_debug_printf(1, "Running spindleInitFE\n");
   result = spindleInitFE((const char **) hosts, &params);
   if (result == -1) {
      spindle_debug_printf(1, "ERROR: Failure under spindleInitFE\n");
      return -1;
   }

   fluxmgr_free_hostlist(hosts);
   hosts = NULL;

   spindle_debug_printf(1, "spindleInitFE completed successfully. Waiting for allocation close.\n");
   return spindleWaitForCloseFE(&params);
}

static char *encode_daemon_args(spindle_args_t *params)
{
   char buffer[256];
   snprintf(buffer, sizeof(buffer), "%u %u %lu %d", params->port, params->num_ports, params->unique_id, (int) OPT_GET_SEC(params->opts));
   buffer[sizeof(buffer)-1] = '\0';
   return strdup(buffer);
}

static char *encode_bootstrap_args(int bootstrap_argc, char **bootstrap_argv)
{
   char *buffer, *s;
   int len = 0, pos = 0, slen = 0;
   for (int i = 0; i < bootstrap_argc; i++) {
      if (bootstrap_argv[i])
         len += strlen(bootstrap_argv[i]) + 1;
      else
         len += 7; //strlen("[NULL]") + 1
   }
   len++;
   buffer = (char *) malloc(len);

   //Strcat all of the bootstrap_argv[i]s to one string
   for (int i = 0; i < bootstrap_argc; i++) {
      if (i != 0)
         buffer[pos++] = ' ';
      s = bootstrap_argv[i];
      if (!s)
         s = "[NULL]";
      slen = strlen(s);
      for (int j = 0; j < slen; j++)
         buffer[pos++] = s[j];
   }
   buffer[pos] = '\0';
   assert(pos < len);

   return buffer;
}
