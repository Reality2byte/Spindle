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

#if !defined(FLUXMGR_H_)
#define FLUXMGR_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <flux/core.h>
   
int fluxmgr_init();
void fluxmgr_close();
int fluxmgr_is_headnode();
int fluxmgr_add_to_kvs(const char *daemon_args, const char *bootstrap_args, const char *default_session);
int fluxmgr_get_from_kvs(char **daemon_args, int timeout_seconds);
int fluxmgr_get_default_session(char **default_session);
int fluxmgr_get_bootstrap(flux_t *fhandle, char **bootstrap_args);   
int fluxmgr_rm_from_kvs();
int fluxmgr_is_active();
int fluxmgr_waitfor(int timeout);
char **fluxmgr_get_hostlist();
void fluxmgr_free_hostlist(char **hostlist);

#if defined(__cplusplus)
}
#endif
   
#endif
