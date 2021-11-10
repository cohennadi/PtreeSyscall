#ifndef _PTREE_H_
#define _PTREE_H_

#include <linux/prinfo.h>

typedef int (*ptree_func)(struct prinfo *buf, int *nr, int pid);
int register_ptree(ptree_func func);
void unregister_ptree(ptree_func func);

#endif

