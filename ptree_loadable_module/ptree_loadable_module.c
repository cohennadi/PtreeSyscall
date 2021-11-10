#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/prinfo.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/export.h>

MODULE_DESCRIPTION ("Ptree loadable module");
MODULE_LICENSE ("GPL");
MODULE_INFO(intree, "Y");

typedef int (*ptree_func)(struct prinfo *buf, int *nr, int pid);
extern int register_ptree(ptree_func func);
extern void unregister_ptree(ptree_func func);

int getptree(struct prinfo *buf, int *nr, int pid) 
{
	int i =0;
	
	if (buf == NULL || nr == NULL || *nr < 1)
	{
		return EINVAL;
	}

	// Do dummy layout
	for (i = 0; i < *nr; ++i) 
	{
		buf[i].parent_pid = i;
	        buf[i].pid = i + 1;
		buf[i].state = 0;
		buf[i].uid = 0;
		strcpy(buf[i].comm, "dummy") ;
		buf[i].level = i;	
	}

	return 0;
}

static int ptree_module_init (void)
{
	int result = register_ptree(&getptree);
	if (!result) 
	{
		pr_info("module loaded\n");
	}

	return result;
}

static void ptree_module_exit (void)
{
	unregister_ptree(&getptree);
	pr_info("module unloaded\n");
}

module_init (ptree_module_init);
module_exit (ptree_module_exit);
