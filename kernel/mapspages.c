#include <linux/mapspages.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/printk.h>



DEFINE_SPINLOCK(global_mapspages_func_lock);
mapspages_func global_mapspages_func = NULL;
EXPORT_SYMBOL(global_mapspages_func);

int register_mapspages(mapspages_func func)
{
	const long SUCCESS = 0;
	int result = -EBUSY;

	spin_lock(&global_mapspages_func_lock);

	if (!global_mapspages_func)
	{
		global_mapspages_func = func;
		result = SUCCESS;
	}

	spin_unlock(&global_mapspages_func_lock);

	return result;
}
EXPORT_SYMBOL(register_mapspages);

void unregister_mapspages(mapspages_func func)
{
	spin_lock(&global_mapspages_func_lock);

	if (global_mapspages_func == func)
	{
	       global_mapspages_func = NULL;
	}

	spin_unlock(&global_mapspages_func_lock);
}
EXPORT_SYMBOL(unregister_mapspages);

SYSCALL_DEFINE4(mapspages, unsigned long, start, unsigned long, end, char __user *, buf, size_t, size)
{
	const long SUCCESS = 0;
	long return_value = SUCCESS; 
	int result_copy = 0;
	char * mapspages_result_buf = NULL;
	long bytes_copied = 0;

	if (start > end)
	{
		return -EINVAL;
	}

	if (!global_mapspages_func) 
	{
		return_value = request_module("mapspages_loadable_module");
		if (return_value != SUCCESS)
		{
			pr_err("kernel/mapspages.c request_module faild, returned %ld", return_value);
			
			return -ENOSYS;
		}
	}

	mapspages_result_buf = (char *)kmalloc(size, GFP_KERNEL);
	if (!mapspages_result_buf)
	{
		pr_err("kernel/mapspages.c mapspages_result_buf malloc failed");

		return -EFAULT;
	}

	spin_lock(&global_mapspages_func_lock);
	if (!global_mapspages_func)
	{
		spin_unlock(&global_mapspages_func_lock);
		pr_err("kernel/mapspages.c Function pointer does not exists");
		
		return -ENOSYS;
	}

	bytes_copied = global_mapspages_func(start, end, mapspages_result_buf, size);

	spin_unlock(&global_mapspages_func_lock);

	if (bytes_copied < 0)
	{
		 pr_err("kernel/mapspages.c mapspages function failed");

		 return bytes_copied;
	}
	
	result_copy = copy_to_user(buf, mapspages_result_buf, bytes_copied); 
	if (result_copy != 0) 
	{
		pr_err("kernel/mapspages.c copy to user failed, returned %d", result_copy);

        return -EFAULT;
	}

	return bytes_copied;
}
