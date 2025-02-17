#include <linux/types.h>
#include <linux/pid.h>

struct prinfo {
	pid_t parent_pid;       /* process id of parent */
	pid_t pid;              /* process id */
	long state;             /* current state of process */
	uid_t uid;              /* user id of process owner */
	char comm[16];          /* name of program executed */
	int level;              /* level of this process in the subtree */
};
