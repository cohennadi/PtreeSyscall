#include "ptree.h"
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/errno.h>

ptree_func global_ptree_func = NULL;
EXPORT_SYMBOL(global_ptree_func);

int register_ptree(ptree_func func)
{
	int result = EBUSY;

	if (global_ptree_func == NULL)
	{
		global_ptree_func = func;
		result = 0; // SUCCESS
	}


	return result;
}
EXPORT_SYMBOL(register_ptree);

void unregister_ptree(ptree_func func)
{
	if (global_ptree_func == func)
	{
	       global_ptree_func = NULL;
	}
}
EXPORT_SYMBOL(unregister_ptree);

SYSCALL_DEFINE3(ptree, struct prinfo __user *, buf, int *, nr, int, pid)
{
	// here implement the syscall
}
