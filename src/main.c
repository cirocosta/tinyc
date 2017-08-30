#define _GNU_SOURCE
#include <fcntl.h>
#include <linux/limits.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// TODO this could come from a '='-separated
//      configuration file.
#define USERNS_OFFSET 10000
#define USERNS_COUNT 2000

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

static char* tc_proc_userns_files[] = {
	"uid_map", "gid_map",
};

int
tc_handle_child_uid_map(pid_t child_pid, int fd)
{
	int uid_map = 0;
	int has_userns = -1;

	// TODO do we really need to do this communication?
	_TC_MUST_GO(
	  (read(fd, &has_userns, sizeof(has_userns)) == sizeof(has_userns)),
	  abort, "couldn't read userns config from child");

	if (has_userns) {
		char path[PATH_MAX] = { 0 };
		char* file;
		int ns_file_formatting_res;

		for (size_t ndx = 0; ndx < 2; ndx++) {
			file = tc_proc_userns_files[ndx];

			ns_file_formatting_res = snprintf(
			  path, sizeof(*path), "/proc/%d/%s", child_pid, file);

			_TC_MUST_GO(ns_file_formatting_res != -1, abort,
			            "errored formating proc userns-map file %s",
			            file);
			_TC_MUST_GO(
			  ns_file_formatting_res < (int)sizeof(path), abort,
			  "got path too long formating proc userns-map file %s",
			  file);

			_TC_MUST_P_GO((uid_map = open(path, O_WRONLY)) != -1,
			              "open", abort, "failed to open file %s",
			              path);

			_TC_MUST_GO(dprintf(uid_map, "0 %d %d\n", USERNS_OFFSET,
			                    USERNS_COUNT) != -1,
			            abort,
			            "failed to write userns mapping to file %s",
			            path);

			close(uid_map);
			uid_map = -1;
		}
	}


	if (write(fd, &(int){ 0 }, sizeof(int)) != sizeof(int)) {
		fprintf(stderr, "couldn't write: %m\n");
		return -1;
	}
	return 0;

abort:
	if (uid_map != -1) {
		close(uid_map);
		uid_map = -1;
	}
	return 1;
}

int
tc_set_userns(struct tc_proc_t* config)
{
	int has_userns = !unshare(CLONE_NEWUSER);
	int result = 0;

	if (write(config->fd, &has_userns, sizeof(has_userns)) !=
	    sizeof(has_userns)) {
		fprintf(stderr, "couldn't write: %m\n");
		return -1;
	}

	if (read(config->fd, &result, sizeof(result)) != sizeof(result)) {
		fprintf(stderr, "couldn't read: %m\n");
		return -1;
	}
	if (result)
		return -1;
	if (has_userns) {
		fprintf(stderr, "done.\n");
	} else {
		fprintf(stderr, "unsupported? continuing.\n");
	}
	fprintf(stderr, "=> switching to uid %d / gid %d...", config->uid,
	        config->uid);
	if (setgroups(1, &(gid_t){ config->uid }) ||
	    setresgid(config->uid, config->uid, config->uid) ||
	    setresuid(config->uid, config->uid, config->uid)) {
		fprintf(stderr, "%m\n");
		return -1;
	}
	fprintf(stderr, "done.\n");
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
	if (program.sockets[0]) {
		close(program.sockets[0]);
		program.sockets[1] = -1;
	}

	if (program.sockets[1]) {
		close(program.sockets[1]);
		program.sockets[1] = -1;
	}

	return 1;
}
