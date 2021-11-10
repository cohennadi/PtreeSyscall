#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

MODULE_DESCRIPTION ("mine");
MODULE_LICENSE ("GPL");
MODULE_INFO(intree, "Y");

static int sample_init (void)
{
	    pr_info("module loaded\n");
	        return 0;
		 }

static void sample_exit (void)
{
	    pr_info("module unloaded\n");
}

module_init (sample_init);
module_exit (sample_exit);
