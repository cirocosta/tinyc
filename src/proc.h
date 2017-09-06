#ifndef TC__PROC_H
#define TC__PROC_H

#include <linux/sched.h>
#include <stdio.h>
#include <sys/types.h>

/**
 *      tc_proc_t encapsulates the configurations
 *      of the container.
 */
typedef struct proc_t {
	// user id (userns-remap).
	uid_t uid;

	// argc is the argument counter that
	// indicates how many arguments 'argv'.
	// has.
	int argc;

	// argv container the arguments specified by
	// the user to be executed in the container.
	char** argv;

	// envp is a list of environment variables
	// to be set as the container environ.
	char** envp;

	// envpc counts the number of environment
	// variables that 'envp' holds..
	int envpc;

	// hostname is the hostname used by the
	// container.
	char hostname[255];

	// roofs is the directory to be
	// mounted as the rootfs inside the container
	// as the '/'.
	char rootfs[255];

	// parent_socket references a socket to
	// communicate with the parent process so
	// that IPC can be performed.
	int parent_socket;

	// stack is the stack previously allocated
	// to the container init process.
	char* stack;
} tc_proc_t;

/**
 *      tc_proc_flags is a bitset-ted int that is
 *      used when 'clone(2)'ing to have the new process
 *      spwaned with a set of namespaces set.
 */
static const int tc_proc_flags = CLONE_NEWNS | CLONE_NEWCGROUP | CLONE_NEWPID |
                                 CLONE_NEWIPC | CLONE_NEWNET | CLONE_NEWUTS;

/**
 *      dumps to 'stderr' the configuration that will
 *      be used by the process to be spawned.
 */
void
tc_proc_show(tc_proc_t* proc)
{
	fprintf(stderr, "Configuration:\n");
	fprintf(stderr, "  uid:             %d\n"
	                "  argc:            %d\n"
	                "  envpc:           %d\n"
	                "  hostname:        %s\n"
	                "  rootfs:          %s\n"
	                "  parent_socket:   %d\n",
	        proc->uid, proc->argc, proc->envpc, proc->hostname,
	        proc->rootfs, proc->parent_socket);

	fprintf(stderr, "  argv:           ");
	for (int i = 0; i < proc->argc; i++) {
		fprintf(stderr, " %s", proc->argv[i]);
	}
	fprintf(stderr, "\n");

	fprintf(stderr, "  envp:          ");
	for (int i = 0; i < proc->envpc; i++) {
		fprintf(stderr, " %s", proc->envp[i]);
	}
	fprintf(stderr, "\n\n");
}

#endif
