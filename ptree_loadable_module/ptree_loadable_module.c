#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/prinfo.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/export.h>
#include <linux/sched.h>
#include <linux/list.h>

#define SUCCESS 0
#define ERROR -1

MODULE_DESCRIPTION ("Ptree loadable module");
MODULE_LICENSE ("GPL");
MODULE_INFO(intree, "Y");

typedef int (*ptree_func)(struct prinfo *buf, int *nr, int pid);
extern int register_ptree(ptree_func func);
extern void unregister_ptree(ptree_func func);

struct  process_traverce_node {
	struct list_head list;
	struct task_struct *task;
	int level;
};

void insert_process_task(struct process_traverce_node * process_node, struct task_struct *task, int level)
{
        process_node->level = level + 1;
	process_node->task = task;
}

int init_process_visit_list(struct list_head * process_visit_list, int max_nodes)
{
	int i = 0;
	struct process_traverce_node *process_node = NULL;

	INIT_LIST_HEAD(process_visit_list);

	for (i  = 0; i < max_nodes; ++i)
	{
		process_node = NULL;

		process_node = (struct process_traverce_node *)kmalloc(sizeof(struct process_traverce_node), GFP_KERNEL);
	        if (!process_node)
	        {
	                pr_err("ptree_loadable_module.c failed to malloc node");

	                return ERROR;
	        }

		process_node->level = -1;
		list_add(&(process_node->list), process_visit_list);
		process_node->task = NULL;
	}

	return SUCCESS;
}

void delete_list(struct list_head * process_visit_list)
{
	struct process_traverce_node * current_node = NULL;

	current_node = list_first_entry_or_null(&process_visit_list, struct process_traverce_node, list);
	while (current_node)
	{
		list_del(&(current_node->list));
		kfree(current_node);

		current_node = list_first_entry_or_null(&process_visit_list, struct process_traverce_node, list);
	}
}


void extract_task_data_to_prinfo_tree(struct task_struct * task, struct prinfo *ptree_data, int tree_level)
{
	ptree_data->parent_pid = (task->parent)->pid;
	ptree_data->pid = task->pid;
	ptree_data->state = task->state;
	ptree_data->uid = (task->cred->uid).val;
	strncpy(ptree_data->comm, task->comm, sizeof(ptree_data->comm) / sizeof(char));
	ptree_data->level = tree_level;

}

int getptree(struct prinfo *buf, int *nr, int pid) 
{
	const int SUCCESS = 0;
	int return_result = SUCCESS;
	int current_index = 0;
	struct task_struct *task = NULL;
	struct task_struct *current_task = NULL;
	struct list_head * current_task_list = NULL;
	struct list_head process_visit_list;
	int result = SUCCESS;
	int current_level = 0;
	struct process_traverce_node * current_node = NULL;
	struct process_traverce_node * last_edited_node = NULL;


	if (buf == NULL || nr == NULL || *nr < 1)
	{
		return -EINVAL;
	}

	result = init_process_visit_list(&process_visit_list, *nr);
	if (result != SUCCESS)
	{
		pr_err("ptree_loadable_module.c init process visit list failed");

	        return -EFAULT;
	}
 
	task = get_pid_task(find_get_pid(PID),PIDTYPE_PID);
	if (!task)
	{
		pr_err("ptree_loadable_module.c get_pid_task failed, pid %d", pid);

		return -EFAULT;
	}

	// Added The first process data
	extract_task_data_to_prinfo_tree(task, buf, 0);
	current_index++;
	current_node = list_first_entry_or_null(&process_visit_list, struct process_traverce_node, list);
	insert_process_task(&current_node, task, current_level);
	last_edited_node = current_node;

	while (current_node)
	{
		
		current_level = current_node->level + 1;

		rcu_read_lock();
		list_for_each(current_task_list, &task->children) 
		{
			// Finish 
			if (current_index >= *nr)
			{
				rcu_read_unlock();
				goto finish;
			}
			
			current_task = list_entry(current_task_list, struct task_struct, sibling);
			extract_task_data_to_prinfo_tree(current_task, buf + current_index++, current_level);
			last_edited_node = list_next_entry(last_edited_node, list);

			// Check we didn't reach the limit.
			if (last_edited_node) 
			{	
				insert_process_task(last_edited_node, task, current_level);
			}	
		}

		rcu_read_unlock();
		
		
		current_node = list_next_entry(current_node, list);		
	}

finish:
	delete_list(&process_visit_list);

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
