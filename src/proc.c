#include "./proc.h"

int
tc_proc_run(tc_proc_t* proc)
{
	int sockets[2] = {
		proc->parent_ipc_socket, proc->child_ipc_socket,
	};

	_TC_MUST_P(
	  (socketpair(AF_LOCAL, SOCK_SEQPACKET, 0, sockets)) == 0, "socketpair",
	  "couldn't create socket pair for parent-child communication");

	_TC_MUST_P_GO((fcntl(sockets[0], F_SETFD, FD_CLOEXEC)) == 0, "fcntl",
	              cleanup, "couldn't set FD_CLOEXEC bit");

	_TC_MUST_P_GO((proc->stack = malloc(STACK_SIZE)), "malloc", cleanup,
	              "couldn't allocate memory to container process stack");

	_TC_MUST_P_GO(
	  (proc->child_pid = clone(tc_child, proc->stack + STACK_SIZE,
	                           tc_proc_flags | SIGCHLD, &proc)) != -1,
	  "clone", cleanup, "couldn't create child process");

	return 0;

cleanup:
	if (sockets[0]) {
		close(sockets[0]);
		sockets[1] = -1;
	}

	if (sockets[1]) {
		close(sockets[1]);
		sockets[1] = -1;
	}

	return 1;
}

/**
 *      dumps to 'stderr' the configuration that will
 *      be used by the process to be spawned.
 */
void
tc_proc_show(tc_proc_t* proc)
{
	fprintf(stderr, "Configuration:\n");
	fprintf(stderr, "  uid:                 %d\n"
	                "  argc:                %d\n"
	                "  envpc:               %d\n"
	                "  hostname:            %s\n"
	                "  rootfs:              %s\n"
	                "  child_pid:           %d\n"
	                "  parent_ipc_socket:   %d\n"
	                "  child_ipc_socket:    %d\n",
	        proc->uid, proc->argc, proc->envpc, proc->hostname,
	        proc->rootfs, proc->child_pid, proc->parent_ipc_socket,
	        proc->child_ipc_socket);

	fprintf(stderr, "  argv:           ");
	for (int i = 0; i < proc->argc; i++) {
		fprintf(stderr, " '%s'", proc->argv[i]);
	}
	fprintf(stderr, "\n");

	fprintf(stderr, "  envp:          ");
	for (int i = 0; i < proc->envpc; i++) {
		fprintf(stderr, " '%s'", proc->envp[i]);
	}
	fprintf(stderr, "\n\n");
}

static char* tc_proc_userns_files[] = {
	"uid_map", "gid_map",
};

// TODO improve the communication pattern
//      between child and parent.
int
tc_handle_child_uid_map(pid_t child_pid, int fd)
{
	int uid_map = 0;
	int has_userns = -1;

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

	_TC_MUST_GO(write(fd, 0, sizeof(int)) == sizeof(int), abort,
	            "failed writing back response to child");

	return 0;

abort:
	if (uid_map != -1) {
		close(uid_map);
		uid_map = -1;
	}
	return 1;
}
