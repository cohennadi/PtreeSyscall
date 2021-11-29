#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/prinfo.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/export.h>
#include <linux/sched.h>
#include <linux/mm_types.h>
#include <linux/mm.h>
#include <linux/pagewalk.h>
#include <asm/current.h>
#include <linux/mapspages.h>
#include <linux/slab.h>
#include <asm/page.h>

#define SUCCESS 0
#define ERROR -1
#define READ_INDEX 0
#define WRITE_INDEX 1
#define EXEC_INDEX 2
#define SHARED_INDEX 3
#define PERMISSIONS_LEN SHARED_INDEX + 1
#define DEVICE_LEN 6

#define CEIL(a, b)     (((a) + (b-1)) / (b))


MODULE_DESCRIPTION ("Mapspages loadable module");
MODULE_LICENSE ("GPL");
MODULE_INFO(intree, "Y");

extern int register_mapspages(mapspages_func func);
extern void unregister_mapspages(mapspages_func func);

struct vma_counter
{
	size_t count_vma;
	//size_t count_pages;
	size_t max_pages;
};

struct pages_descriptor
{
	char* refcount_representations;
	size_t refcount_representations_count;
	size_t refcount_representations_size;
};

struct vma_descriptor 
{
	unsigned long start;
	unsigned long end;
	char permissions[PERMISSIONS_LEN]; // r = read, w = write, x = execute, s = shared, p = private (copy on write)
	unsigned long offest; // The offset field is the offset into the file/whatever;
	char device[DEVICE_LEN]; // the device (major:minor)
	unsigned long inode; // inode is the inode on that device.  0 indicates that no inode is associated with the memory region. 
	struct pages_descriptor pages_data; // extra data for the exercise
};

struct memory_range_descriptor
{
	struct vma_descriptor* vma_descriptors;
	size_t vma_descriptors_count;
	size_t vma_descriptors_size;
};

void set_permissions(struct vma_descriptor *vma_desc, vm_flags_t flags)
{
	vma_desc->permissions[READ_INDEX] = (flags & VM_READ) ? 'r' : '-';
	vma_desc->permissions[READ_INDEX] = (flags & VM_WRITE) ? 'w' : '-';
	vma_desc->permissions[READ_INDEX] = (flags & VM_EXEC) ? 'x' : '-';
	vma_desc->permissions[READ_INDEX] = (flags & VM_SHARED) ? 's' : 'p';
	vma_desc->permissions[READ_INDEX] = '\0';
}

void set_device(struct vma_descriptor *vma_desc, dev_t dev)
{
	snprintf(vma_desc->device, DEVICE_LEN, "%.2d:%.2d", MAJOR(dev), MINOR(dev)); ;

}

int check_vma_callback(unsigned long addr, unsigned long next, struct mm_walk *walk)
{
	struct vm_area_struct *current_vma = walk->vma;
	struct file *file = current_vma->vm_file;
	vm_flags_t flags = current_vma->vm_flags;
	unsigned long ino = 0;
	unsigned long long pgoff = 0;
	dev_t dev = 0;
	struct inode *inode = NULL;
	struct memory_range_descriptor *memory_range_desc = (struct memory_range_descriptor*)walk->private;
	size_t current_vma_index = memory_range_desc->vma_descriptors_count;
	struct vma_descriptor *vma_desc = memory_range_desc->vma_descriptors + current_vma_index;

	if (current_vma_index >= memory_range_desc->vma_descriptors_size)
	{
		pr_err("%s: current_vma_index %zu larger than expected size %zu\n", 
			__FILE__, 
			current_vma_index, 
			memory_range_desc->vma_descriptors_size);

		return 1; // skip current vma
	}

	++memory_range_desc->vma_descriptors_count;

	if (file) 
	{
		inode = file_inode(current_vma->vm_file);
		dev = inode->i_sb->s_dev;
		ino = inode->i_ino;
		pgoff = ((loff_t)current_vma->vm_pgoff) << PAGE_SHIFT;
	}

	vma_desc->start = current_vma->vm_start;
	vma_desc->end = current_vma->vm_end;
	vma_desc->offest = pgoff;
	vma_desc->inode = ino;
	set_permissions(vma_desc, flags);
	set_device(vma_desc, dev);

	return 0;
}

char page_refcount_char_representation(int page_refcount)
{
	if (page_refcount == 0)
	{
		return '.'; 
	}

	if (page_refcount > 9)
	{
		return 'X'; 
	}

	return page_refcount + '0';
}

int pte_entry_callback(pte_t *pte, unsigned long addr, unsigned long next, struct mm_walk *walk)
{
	struct page *current_page = NULL;
	int page_refcount = 0;
	char refcount_representation = '.';
	struct memory_range_descriptor* memory_range_desc = (struct memory_range_descriptor*)walk->private;
	size_t current_vma_index = memory_range_desc->vma_descriptors_count - 1;
	size_t *current_refcount_index = &(memory_range_desc->vma_descriptors[current_vma_index].pages_data.refcount_representations_count);
	size_t refcount_max_size = memory_range_desc->vma_descriptors[current_vma_index].pages_data.refcount_representations_size;

	if (*current_refcount_index + 1 > refcount_max_size)
	{
		pr_err("%s: current_refcount_index %zu larger than expected size %zu\n", 
			__FILE__, 
			*current_refcount_index, 
			refcount_max_size);

		return 1; 
	}

	current_page = pte_page(*pte);
	page_refcount = page_count(current_page);
	refcount_representation = page_refcount_char_representation(page_refcount);

	memory_range_desc->vma_descriptors[current_vma_index].pages_data.refcount_representations[*current_refcount_index] = refcount_representation;
	memory_range_desc->vma_descriptors[current_vma_index].pages_data.refcount_representations[++(*current_refcount_index)] = '\0';

	return 0;
}

int count_callback(unsigned long addr, unsigned long next, struct mm_walk *walk)
{
	struct vma_counter *counters = (struct vma_counter*)walk->private;
	size_t total_pages = CEIL((next - addr), PAGE_SIZE);
	
	counters->count_vma++;

	if (total_pages > counters->max_pages)
	{
		counters->max_pages = total_pages;	
	}

	return 0;
}

int write_buf_report(struct memory_range_descriptor* reports, char *buf, size_t size)
{
	int index = 0;
	int written_bytes = 0;
	int total_written_bytes = 0;

	for (index = 0; index < reports->vma_descriptors_size; ++index)
	{
		written_bytes = snprintf(buf,
                 	 		   size,
                 	 		   "%lx-%lx %s %lx %s %lx %s\n",
                 	 		   reports->vma_descriptors[index].start,
                 	 		   reports->vma_descriptors[index].end,
                 	 		   reports->vma_descriptors[index].permissions,
                 	 		   reports->vma_descriptors[index].offest, 
                 	 		   reports->vma_descriptors[index].device,
                 	 		   reports->vma_descriptors[index].inode,
                 	 		   reports->vma_descriptors[index].pages_data.refcount_representations);

		if (written_bytes < 0)
		{
			pr_err("%s: snprintf failed with result %d\n", __FILE__, written_bytes);

			return ERROR;
		}

		total_written_bytes += written_bytes;
	}

	return total_written_bytes + 1; 	
}

int get_counters(unsigned long start, unsigned long end, struct vma_counter *counters)
{
	int locking_result = 0;	
	int walk_page_range_result = 0;
	struct mm_walk_ops mapspages_operations = {
        	.test_walk = count_callback
    	};

	locking_result = down_read_killable(&current->mm->mmap_sem);
	if (locking_result != 0)
	{
		pr_err("%s: down_read_killable failed with result %d\n", __FILE__, locking_result);

		return ERROR
	}

	walk_page_range_result = walk_page_range(current->mm, start, end, &mapspages_operations, counters); 
	if (walk_page_range_result != SUCCESS)
	{
		pr_err("%s: walk_page_range failed with result %d\n", __FILE__, walk_page_range_result);

		return ERROR;
	}

	// release a read lock.
	up_read(&current->mm->mmap_sem);

	return SUCCESS;
}

int init_memory_range_descriptor(unsigned long start, unsigned long end, struct memory_range_descriptor *out_descriptor)
{
	int index = 0;
	struct vma_counter counters = {0};
	int result = SUCCESS;

	result = get_counters(start, end, &counters);
	if (result != SUCCESS)
	{
		pr_err("%s: get_counters failed\n", __FILE__);

		return ERROR;
	}

	out_descriptor->vma_descriptors = (struct vma_descriptor *)kcalloc(counters.count_vma, sizeof(struct vma_descriptor), GFP_KERNEL);
	if (!out_descriptor->vma_descriptors)
	{
		pr_err("%s: kcalloc failed\n", __FILE__);

		return ERROR;
	}

	out_descriptor->vma_descriptors_size = counters.count_vma;
	out_descriptor->vma_descriptors_count = 0;

	for (index = 0; index < counters.count_vma; ++index)
	{
		out_descriptor->vma_descriptors[index].pages_data.refcount_representations = (char *)kcalloc(counters.max_pages + 1, sizeof(char), GFP_KERNEL);
		if (!out_descriptor->vma_descriptors[index].pages_data.refcount_representations)
		{
			pr_err("%s: kcalloc failed\n", __FILE__);

			return ERROR;
		}

		out_descriptor->vma_descriptors[index].pages_data.refcount_representations_size = counters.max_pages + 1;
		out_descriptor->vma_descriptors[index].pages_data.refcount_representations_count = 0;
	}

	return SUCCESS;
}

void clean_memory_range_descriptor(struct memory_range_descriptor *descriptor)
{
	int index = 0;

	for (index = 0; index < descriptor->vma_descriptors_size; ++index)
	{
		kfree(descriptor->vma_descriptors[index].pages_data.refcount_representations);
	}

	kfree(descriptor->vma_descriptors);
}


int get_mapspages(unsigned long start, unsigned long end, char *buf, size_t size) 
{
	int written_bytes = 0;
	int locking_result = 0;	
	int walk_page_range_result = 0;
	int init_result = SUCCESS;
	struct memory_range_descriptor memory_desc = {0};
	struct mm_walk_ops mapspages_operations = {
        	.pte_entry = pte_entry_callback,
        	.test_walk = check_vma_callback
    	};
    	
    	init_result = init_memory_range_descriptor(start, end, &memory_desc);
    	if (init_result != SUCCESS)
    	{
    		pr_err("%s: init_memory_range_descriptor failed\n", __FILE__);

		return ERROR;
    	}

	// Locks a read lock.
	// Mmap_sem could be locked for a long time or forever if something goes wrong. 
	// Using a killable lock permits cleanup of stuck tasks and simplifies investigation.
	locking_result = down_read_killable(&current->mm->mmap_sem);
	if (locking_result != 0)
	{
		pr_err("%s: down_read_killable failed with result %d\n", __FILE__, locking_result);

		return ERROR;
	}

	walk_page_range_result = walk_page_range(current->mm, start, end, &mapspages_operations, &memory_desc);
	if (walk_page_range_result != SUCCESS)
	{
		pr_err("%s: walk_page_range failed with result %d\n", __FILE__, walk_page_range_result);

		return ERROR;
	}

	// release a read lock.
	up_read(&current->mm->mmap_sem);

	written_bytes = write_buf_report(&memory_desc, buf, size); 
	if (written_bytes < 0)
	{
		pr_err("%s: write_buf_report failed\n", __FILE__);

		return ERROR;
	}

	clean_memory_range_descriptor(&memory_desc);	

	return written_bytes;
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
