#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "./common.h"

#define STACK_SIZE _TC_MB(1)

typedef struct proc_t {
	int argc;
	char** argv;
	char** envp;
	char* rootfs;
	char* hostname;
	char* stack;
} tc_proc_t;

static const int tc_proc_flags = CLONE_NEWNS | CLONE_NEWCGROUP | CLONE_NEWPID |
                                 CLONE_NEWIPC | CLONE_NEWNET | CLONE_NEWUTS;

int
tc_child(void* __attribute__((unused)) arg)
{
	return 0;
}

int
main()
{
	pid_t container_pid;
	tc_proc_t proc = { 0 };

	_TC_MUST_P((proc.stack = malloc(STACK_SIZE)), "malloc",
	           "couldn't allocate memory to container process stack");

	if ((container_pid = clone(tc_child, proc.stack + STACK_SIZE,
	                           tc_proc_flags | SIGCHLD, &proc)) == -1) {
		fprintf(stderr, "=> clone failed! %m\n");
		// wow, failed
	}

	printf("hello\n");
	return 0;
}
