#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

MODULE_DESCRIPTION ("Ptree loadable module");
MODULE_LICENSE ("GPL");
MODULE_INFO(intree, "Y");

static int ptree_module_init (void)
{
	pr_info("module loaded\n");
	return 0;
}

static void ptree_module_exit (void)
{
	pr_info("module unloaded\n");
}

module_init (ptree_module_init);
module_exit (ptree_module_exit);
