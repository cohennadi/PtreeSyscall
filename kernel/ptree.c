#include "ptree.h"
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/printk.h>

DEFINE_SPINLOCK(global_ptree_func_lock);
ptree_func global_ptree_func = NULL;
EXPORT_SYMBOL(global_ptree_func);

int register_ptree(ptree_func func)
{
	const long SUCCESS = 0;
	int result = -EBUSY;

	spin_lock(&global_ptree_func_lock);

	if (!global_ptree_func)
	{
		global_ptree_func = func;
		result = SUCCESS;
	}

	spin_unlock(&global_ptree_func_lock);


	return result;
}
EXPORT_SYMBOL(register_ptree);

void unregister_ptree(ptree_func func)
{
	spin_lock(&global_ptree_func_lock);

	if (global_ptree_func == func)
	{
	       global_ptree_func = NULL;
	}

	spin_unlock(&global_ptree_func_lock);
}
EXPORT_SYMBOL(unregister_ptree);

SYSCALL_DEFINE3(ptree, struct prinfo __user *, buf, int __user *, nr, int, pid)
{
	const long SUCCESS = 0;
	long return_value = SUCCESS; 
	int result_copy = 0;
	int buffer_length = 0;
	struct prinfo * process_tree_data = NULL;

	if (!global_ptree_func) 
	{
		return_value = request_module("ptree_loadable_module");
		if (return_value != SUCCESS)
		{
			pr_err("kernel/ptree.c request_module faild, returned %ld", return_value);
			
			return -ENOSYS;
		}
	}

	result_copy = copy_from_user(&buffer_length, nr, sizeof(int));
	if (result_copy != 0) 
	{
		pr_err("kernel/ptree.c copy from user failed, result %d", result_copy);
		
		return -EFAULT;
	}

	process_tree_data = (struct prinfo *)kmalloc(buffer_length * sizeof(struct prinfo), GFP_KERNEL);
	if (!process_tree_data)
	{
		pr_err("kernel/ptree.c process_tree_data malloc failed");

		return -EFAULT;
	}

	spin_lock(&global_ptree_func_lock);
	if (!global_ptree_func)
	{
		spin_unlock(&global_ptree_func_lock);
		pr_err("kernel/ptree.c Function pointer does not exists");
		
		return -ENOSYS;
	}

	return_value = global_ptree_func(process_tree_data, &buffer_length, pid);

	spin_unlock(&global_ptree_func_lock);

	if (return_value != SUCCESS)
	{
		 pr_err("kernel/ptree.c ptree function failed");

		 return return_value;
	}
	
	result_copy = copy_to_user(buf, process_tree_data, buffer_length * sizeof(struct prinfo)); 
	if (result_copy != 0) 
	{
		pr_err("kernel/ptree.c copy to user failed, returned %d", result_copy);

                return -EFAULT;
	}
	

	result_copy = copy_to_user(nr, &buffer_length, sizeof(int));
        if (result_copy != 0)
        {
                pr_err("kernel/ptree.c copy to user failed, returned %d", result_copy);

		return -EFAULT;
        }

	return return_value;
}
