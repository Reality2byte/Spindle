#define _GNU_SOURCE
#include <link.h>
#include <stdio.h>
#include <string.h>

extern int isspace(int c);

unsigned int la_version(unsigned int v)
{
   return LAV_CURRENT;
}

unsigned int la_objopen(struct link_map *map, Lmid_t lmid, uintptr_t *cookie)
{
   //Need something that accesses TLS through R_X86_64_TPOFF64, which
   //isspace does by getting the locale
   int result;
   if (strstr(map->l_name, "libm.so")) {
      result = isspace(' ');
   }
   return 0;
}
