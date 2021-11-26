#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/prinfo.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/export.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/cred.h>
#include <linux/mapspages.h>

#define SUCCESS 0
#define ERROR -1

MODULE_DESCRIPTION ("Mapspages loadable module");
MODULE_LICENSE ("GPL");
MODULE_INFO(intree, "Y");

extern int register_mapspages(mapspages_func func);
extern void unregister_mapspages(mapspages_func func);

int get_mapspages(unsigned long start, unsigned long end, char __user *buf, size_t size) 
{
	return 0;
}

static int mapspages_module_init (void)
{
	int result = register_mapspages(&get_mapspages);
	if (!result) 
	{
		pr_info("mapspages module loaded\n");
	}

	return result;
}

static void mapspages_module_exit (void)
{
	unregister_mapspages(&get_mapspages);
	pr_info("mapspages module unloaded\n");
}

module_init (mapspages_module_init);
module_exit (mapspages_module_exit);