#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PTREE_SYSCALL_NUM 449
#define DEFAULT_SIZE 10
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
	struct prinfo *ptree_data;
	long result = 0;
	int i = 0;
	int pid = 1;

	printf("arc %d\n", argc);
	if (argc == ARGS_TOTAL)
	{
		printf("pid as srtingn %s\n", argv[PID_INDEX]);
		pid = strtol(argv[PID_INDEX], NULL, 10);
		printf("pid as int %d\n", pid);	
	}

	ptree_data= (struct prinfo *)malloc(DEFAULT_SIZE * sizeof(struct prinfo));
	if (!ptree_data)
	{
		return FAIL;	
	}

	ptree_data_length = DEFAULT_SIZE;
	//result = SUCCESS;
	//do {
	//	if (result != SUCCESS)
	//	{
	//		free(ptree_data);
	//		ptree_data_length = ptree_data_length * 2;
	//		ptree_data= (struct prinfo *)malloc(ptree_data_length * sizeof(struct prinfo));
	//		if (!ptree_data)
	//		{
	//			return FAIL;
	//		}
	//	}
	//	
	//	printf("before calling syscall\n");
	//	result = syscall(PTREE_SYSCALL_NUM, ptree_data, &ptree_data_length);
	//	printf("after calliing syscall, retured %ld\n", result);
	//} while(result != SUCCESS);

	result = syscall(PTREE_SYSCALL_NUM, ptree_data, &ptree_data_length, pid);
	printf("after calliing syscall, retured %ld, data length\n", result);

	for (int i = 0; i < ptree_data_length; ++i)
	{
		printf("%d,%s,%d,%d,%ld,%d\n", ptree_data[i].level, ptree_data[i].comm,
			ptree_data[i].pid, ptree_data[i].parent_pid, ptree_data[i].state, 
			ptree_data[i].uid);
	}


	free(ptree_data);

	return SUCCESS;
}


