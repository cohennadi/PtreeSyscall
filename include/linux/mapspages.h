#ifndef _MAPSPAGES_H_
#define _MAPSPAGES_H_

#include <stddef.h>

typedef int (*mapspages_func)(unsigned long start, unsigned long end, char *buf, size_t size);
int register_mapspages(mapspages_func func);
void unregister_mapspages(mapspages_func func);

#endif
