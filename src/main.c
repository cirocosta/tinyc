#define _GNU_SOURCE
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <sys/socket.h>
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
	int parent_socket;
} tc_proc_t;

typedef struct tc_t {
	pid_t child_pid;
	int sockets[2];
	int err;
} tc_tc_t;

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
	tc_proc_t proc = { 0 };
	tc_tc_t program = {
		.sockets = { 0 }, .err = 0,
	};

	_TC_MUST_P(
	  (socketpair(AF_LOCAL, SOCK_SEQPACKET, 0, program.sockets)) == 0,
	  "socketpair",
	  "couldn't create socket pair for parent-child communication");

	_TC_MUST_P_GO((fcntl(program.sockets[0], F_SETFD, FD_CLOEXEC)) == 0,
	              "fcntl", cleanup, "couldn't set FD_CLOEXEC bit");

	_TC_MUST_P_GO((proc.stack = malloc(STACK_SIZE)), "malloc", cleanup,
	              "couldn't allocate memory to container process stack");
	proc.parent_socket = program.sockets[1];

	_TC_MUST_P_GO(
	  (program.child_pid = clone(tc_child, proc.stack + STACK_SIZE,
	                             tc_proc_flags | SIGCHLD, &proc)) != -1,
	  "clone", cleanup, "couldn't create child process");

	printf("hello\n");
	return 0;

cleanup:
	if (program.sockets[0])
		close(program.sockets[0]);
	if (program.sockets[1])
		close(program.sockets[1]);
	return 1;
}
