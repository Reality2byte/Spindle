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

#ifndef LDCS_CACHE_H
#define LDCS_CACHE_H

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum {
  LDCS_CACHE_DIR_PARSED_AND_EXISTS,
  LDCS_CACHE_DIR_PARSED_AND_NOT_EXISTS,
  LDCS_CACHE_DIR_NOT_PARSED,
  LDCS_CACHE_FILE_FOUND,
  LDCS_CACHE_FILE_NOT_FOUND,
  LDCS_CACHE_UNKNOWN
} ldcs_cache_result_t;

typedef enum {
  LDCS_CACHE_OBJECT_STATUS_NOT_SET,
  LDCS_CACHE_OBJECT_STATUS_LOCAL_PATH,
  LDCS_CACHE_OBJECT_STATUS_GLOBAL_PATH,
  LDCS_CACHE_OBJECT_STATUS_UNKNOWN
} ldcs_hash_object_status_t;

typedef enum {
   LDCS_CACHE_FILEOBJ_DSO,
   LDCS_CACHE_FILEOBJ_FILE,
   LDCS_CACHE_FILEOBJ_EITHER
} ldcs_hash_fileobj_t;
   
ldcs_cache_result_t ldcs_cache_findDirInCache(char *dirname);
ldcs_cache_result_t ldcs_cache_findFileDirInCache(char *filename, char *dirname, ldcs_hash_fileobj_t objt, char **localpath, int *errcode);
ldcs_cache_result_t ldcs_cache_getAlias(char *filename, char *dirname, char **alias);
ldcs_cache_result_t ldcs_cache_isReplicated(char *filename, char *dirname, int *replication);

ldcs_cache_result_t ldcs_cache_processDirectory(char *dirname, size_t *bytesread);


ldcs_cache_result_t ldcs_cache_updateAlias(char *filename, char *dirname, char *alias_to);
ldcs_cache_result_t ldcs_cache_updateBuffer(char *filename, char *dirname, char *localname, void *buffer, size_t buffer_size, int errcode, int is_stripped);
ldcs_cache_result_t ldcs_cache_updateReplication(char *filename, char *dirname, int replication);
ldcs_cache_result_t ldcs_cache_updateErrcode(char *filename, char *dirname, int errcode);
   

int ldcs_cache_encodeDirContents(char *dir, char **data, int *len);
int ldcs_cache_decodeDirContents(char *buffer, size_t len,
                                 char *dirbuffer, size_t dirbuffer_sz);   

int ldcs_cache_init();
int ldcs_cache_dump(char *filename);

int ldcs_cache_get_buffer(char *dirname, char *filename, ldcs_hash_fileobj_t objt, void **buffer, size_t *size, char **alias_to);

char *ldcs_cache_result_to_str(ldcs_cache_result_t res);
void ldcs_cache_addFileDir(char *dname, char *fname);

int ldcs_cache_pickone_get(char *key);
void ldcs_cache_pickone_set(char *key);

#if defined(__cplusplus)
}
#endif

#endif
