#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PTREE_SYSCALL_NUM 449
#define DEFAULT_SIZE 3
#define SUCCESS 0
#define FAIL -1
#define PID_INDEX 1
#define ARGS_TOTAL 2

struct prinfo {
	        pid_t parent_pid;       /* process id of parent */
	        pid_t pid;              /* process id */
	        long state;             /* current state of process */
	        uid_t uid;              /* user id of process owner */
	        char comm[16];          /* name of program executed */
	        int level;              /* level of this process in the subtree */
};


int main(int argc, char **argv)
{
	int ptree_data_length = 0;
	int original_length = 0;
	struct prinfo *ptree_data;
	long result = 0;
	int i = 0;
	int pid = 1;
	int should_continue = 0;

	if (argc == ARGS_TOTAL)
	{
		pid = strtol(argv[PID_INDEX], NULL, 10);
	}

	ptree_data= (struct prinfo *)malloc(DEFAULT_SIZE * sizeof(struct prinfo));
	if (!ptree_data)
	{
		return FAIL;	
	}

	ptree_data_length = original_length = DEFAULT_SIZE;
	do {
		original_length = ptree_data_length;
		result = syscall(PTREE_SYSCALL_NUM, ptree_data, &ptree_data_length, pid);
		if (result != SUCCESS)
		{
			printf("syscall failed %ld\n", result);

			return FAIL;
		}

		if (ptree_data_length == original_length)
		{
			free(ptree_data);
			ptree_data_length = original_length * 2;
			ptree_data= (struct prinfo *)malloc(ptree_data_length * sizeof(struct prinfo));
			if (!ptree_data)
			{
				return FAIL;
			}
			
			should_continue = 1;
		} 
		else
		{
			should_continue = 0;
		}
	} while(should_continue);

	for (int i = 0; i < ptree_data_length; ++i)
	{
		printf("%d,%s,%d,%d,%ld,%d\n", ptree_data[i].level, ptree_data[i].comm,
			ptree_data[i].pid, ptree_data[i].parent_pid, ptree_data[i].state, 
			ptree_data[i].uid);
	}


	free(ptree_data);

	return SUCCESS;
}


