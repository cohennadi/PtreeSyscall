#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/prinfo.h>

#define PTREE_SYSCALL_NUM 449
#define DEFAULT_SIZE 256
#define SUCCESS 0
#define FAIL -1

int main(int argc, char **argv)
{
	int ptree_data_length = 0;
	struct prinfo *ptree_data;
	long result = 0;
	int i = 0;

	ptree_data= (struct prinfo *)malloc(DEFAULT_SIZE * sizeof(struct prinfo));
	if (!ptree_data)
	{
		return FAIL;	
	}

	ptree_data_length = DEFAULT_SIZE;
	result = SUCCESS;
	do {
		if (result != SUCCESS)
		{
			free(ptree_data);
			ptree_data_length = ptree_data_length * 2;
			ptree_data= (struct prinfo *)malloc(ptree_data_length * sizeof(struct prinfo));
			if (!ptree_data)
			{
				return FAIL;
			}
		}
		result = syscall(PTREE_SYSCALL_NUM, ptree_data, &ptree_data_length);
	} while(result != SUCCESS);

	for (int i = 0; i < ptree_data_length; ++i)
	{
		printf("%d,%s,%d,%d,%ld,%d,%d\n", ptree_data[i].level, ptree_data[i].comm,
			ptree_data[i].pid, ptree_data[i].parent_pid, ptree_data[i].state, 
			ptree_data[i].uid);
	}


	free(ptree_data);

	return SUCCESS;
}


