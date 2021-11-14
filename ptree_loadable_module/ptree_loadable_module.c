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

#define SUCCESS 0
#define ERROR -1

MODULE_DESCRIPTION ("Ptree loadable module");
MODULE_LICENSE ("GPL");
MODULE_INFO(intree, "Y");

typedef int (*ptree_func)(struct prinfo *buf, int *nr, int pid);
extern int register_ptree(ptree_func func);
extern void unregister_ptree(ptree_func func);
void extract_task_data_to_prinfo_tree(struct task_struct * task, struct prinfo *ptree_data, int tree_level)
{
        ptree_data->parent_pid = (task->parent)->pid;
        ptree_data->pid = task->pid;
        ptree_data->state = task->state;
        ptree_data->uid = (task->cred->uid).val;
        strncpy(ptree_data->comm, task->comm, sizeof(ptree_data->comm) / sizeof(char));
        ptree_data->level = tree_level;
}

int extract_children(int parent_pid, int level, struct prinfo *ptree_data, int* io_current_index, int buffer_length) 
{
	struct task_struct *parent_task = NULL;
	struct task_struct *current_child = NULL;
	int result = SUCCESS;

	if (!ptree_data || !io_current_index || *io_current_index >= buffer_length)
	{
		return ERROR;
	}

	rcu_read_lock();
	parent_task = get_pid_task(find_get_pid(parent_pid),PIDTYPE_PID);
	if (!parent_task)
	{
		pr_err("ptree_loadable_module.c get_pid_task failed, parent pid %d",parent_pid);

	        result = ERROR;
		goto extract_finish;
        }

	list_for_each_entry(current_child, &parent_task->children, sibling)
        {
		if (*io_current_index >= buffer_length)
		{
			goto extract_finish;		
		}

		extract_task_data_to_prinfo_tree(current_child, ptree_data + *io_current_index, level);
		++(*io_current_index);
	}

extract_finish:	
	rcu_read_unlock();

	return result;
}

int getptree(struct prinfo *buf, int *nr, int pid) 
{
	int return_result = SUCCESS;
	int current_parent = 0;
	int current_index = 0;
	struct task_struct *task = NULL;
	int result = SUCCESS;

	if (buf == NULL || nr == NULL || *nr < 1)
	{
		return -EINVAL;
	}
	
	task = get_pid_task(find_get_pid(pid),PIDTYPE_PID);
	if (!task)
	{
		pr_err("ptree_loadable_module.c get_pid_task failed, pid %d", pid);

		return -EFAULT;
	}

	// Added The first process data
	extract_task_data_to_prinfo_tree(task, buf, 0);
	current_index++;

	while (current_parent < current_index)
	{
		result = extract_children(buf[current_parent].pid, buf[current_parent].level + 1, buf, &current_index, *nr);
		if (result != SUCCESS)
		{
			pr_err("extract children failed");

			return -EFAULT;
		}

		++current_parent;
	}

	*nr = current_index;
	
	return return_result;
}

static int ptree_module_init (void)
{
	int result = register_ptree(&getptree);
	if (!result) 
	{
		pr_info("ptree module loaded\n");
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
