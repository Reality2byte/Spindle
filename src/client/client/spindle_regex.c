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

#include <regex.h>
#include <stdlib.h>
#include "spindle_regex.h"
#include "client_heap.h"
#include "spindle_debug.h"

static int is_regex(const char *regex_str)
{
   int i = 0;
   for (i = 0; regex_str[i] != '\0'; i++) {
      char c = regex_str[i];
      if (c == '*' || c == '[' || c == '+' || c == '^' || c == '$' || c == '?')
         return 1;
   }
   return 0;
}

#define STR(X) STR2(X)
#define STR2(X) #X

#define STRCASE(X) case X: return STR(X)
const char *regex_error_str(int errcode) {
   switch (errcode) {
      STRCASE(REG_BADPAT);
      STRCASE(REG_BADRPT);
      STRCASE(REG_ECOLLATE);
      STRCASE(REG_ECTYPE);
      STRCASE(REG_EESCAPE);
      STRCASE(REG_ESUBREG);
      STRCASE(REG_EBRACK);
      STRCASE(REG_EPAREN);
      STRCASE(REG_EBRACE);
      STRCASE(REG_ERANGE);
      STRCASE(REG_ESPACE);
      default: return "[UNKNOWN ERROR CODE]";
   }
}

spindle_regex_t parse_spindle_regex_str(const char *regex_str)
{
   regex_t *compiled_regex = NULL;
   int result;
   
   if (!is_regex(regex_str)) {
      return NULL;
   }

   compiled_regex = (regex_t *) spindle_malloc(sizeof(regex_t));
   result = regcomp(compiled_regex, regex_str, REG_NOSUB);
   if (result != 0) {
      err_printf("Failed to parse regular expression. Error is %s (%d)\n", regex_error_str(result), result);
      return NULL;
   }

   debug_printf2("Compiled regex from %s\n", regex_str);
   return (spindle_regex_t) compiled_regex;
}

int matches_spindle_regex(spindle_regex_t regex, const char *str)
{
   int result;
   regex_t *compiled_regex = (regex_t *) regex;

   result = regexec(compiled_regex, str, 0, NULL, 0);
   return (result == 0) ? 1 : 0;
}
