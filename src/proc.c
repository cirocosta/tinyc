#include "./proc.h"

int
tc_proc_init(tc_proc_t* proc)
{
	int sockets[2] = { 0 };

	_TC_DEBUG("initializing proc structure");

	_TC_MUST_P_GO(
	  (socketpair(AF_LOCAL, SOCK_SEQPACKET, 0, sockets)) == 0, "socketpair",
	  abort, "couldn't create socket pair for parent-child communication");

	proc->parent_ipc_socket = sockets[0];
	proc->child_ipc_socket = sockets[1];

	_TC_MUST_P_GO(
	  (fcntl(proc->parent_ipc_socket, F_SETFD, FD_CLOEXEC)) == 0, "fcntl",
	  abort, "couldn't set FD_CLOEXEC bit on parent-socket");

	_TC_DEBUG("socket pair set (parent=%d,child=%d)",
	          proc->parent_ipc_socket, proc->child_ipc_socket);

	_TC_MUST_P_GO((proc->stack = malloc(STACK_SIZE)), "malloc", abort,
	              "couldn't allocate memory to container process stack");

	_TC_DEBUG("process stack allocated (stack=%p)", proc->stack);

	return 0;

abort:
	_TC_INFO("failed initializing proc structure");
	return 1;
}

int
tc_proc_run(tc_proc_t* proc, int (*child_fn)(void*))
{
	_TC_DEBUG("starting to run process");

	tc_proc_show(proc);

	_TC_MUST_P_GO(
	  (proc->child_pid = clone(child_fn, proc->stack + STACK_SIZE,
	                           tc_proc_flags | SIGCHLD, proc)) != -1,
	  "clone", abort, "couldn't create child process");

	if (proc->disable_userns_remap == false) {
		_TC_MUST_GO(!tc_proc_handle_child_uid_remap(proc), abort,
		            "failed performing userns remap");
	}

	_TC_DEBUG("waiting child from pid %d", proc->child_pid);

	_TC_MUST_P_GO(waitpid(proc->child_pid, NULL, 0) != -1, "waitpid", abort,
	              "failed waiting on child_pid %d", proc->child_pid);

	return 0;

abort:
	_TC_INFO("failed running proc process");
	return 1;
}

void
tc_proc_cleanup(tc_proc_t* proc)
{
	_TC_DEBUG("performing proc cleanup");

	if (!proc) {
		return;
	}

	if (proc->stack) {
		free(proc->stack);
		proc->stack = NULL;
	}

	if (proc->parent_ipc_socket > 0) {
		close(proc->parent_ipc_socket);
		proc->parent_ipc_socket = -1;
	}

	if (proc->child_ipc_socket > 0) {
		close(proc->child_ipc_socket);
		proc->child_ipc_socket = -1;
	}
}

void
tc_proc_show(tc_proc_t* proc)
{
	fprintf(stderr, "Configuration:\n");
	fprintf(stderr, "  uid:                   %d\n"
	                "  argc:                  %d\n"
	                "  envpc:                 %d\n"
	                "  hostname:              %s\n"
	                "  rootfs:                %s\n"
	                "  child_pid:             %d\n"
	                "  parent_ipc_socket:     %d\n"
	                "  child_ipc_socket:      %d\n"
	                "  disable_seccomp:       %d\n"
	                "  disable_capabilities:  %d\n"
	                "  disable_userns_remap:  %d\n",
	        proc->uid, proc->argc, proc->envpc, proc->hostname,
	        proc->rootfs, proc->child_pid, proc->parent_ipc_socket,
	        proc->child_ipc_socket, proc->disable_seccomp,
	        proc->disable_capabilities, proc->disable_userns_remap);

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

/**
 * name of the files to use to set the userns mapping.
 */
const static char* tc_proc_userns_files[] = {
	"uid_map", "gid_map",
};

// TODO improve the communication pattern
//      between child and parent.
int
tc_proc_handle_child_uid_remap(tc_proc_t* proc)
{
	int uid_map_fd = 0;
	int has_userns = -1;
	char path[PATH_MAX] = { 0 };
	const char* file;
	int ns_file_formatting_res;
	const int ok = 0;

	_TC_DEBUG("waiting for child readiness to handle userns remapping");

	_TC_MUST_P_GO((read(proc->child_ipc_socket, &has_userns,
	                    sizeof(has_userns)) == sizeof(has_userns)),
	              "read", abort, "couldn't read userns config from child");

	if (!has_userns) {
		_TC_MUST_P_GO(
		  write(proc->child_ipc_socket, 0, sizeof(int)) == sizeof(int),
		  "write", abort, "failed writing back response to child");
		return 0;
	}

	_TC_DEBUG("starting userns remap");

	for (size_t ndx = 0; ndx < 2; ndx++) {
		file = tc_proc_userns_files[ndx];

		ns_file_formatting_res = snprintf(path, PATH_MAX, "/proc/%d/%s",
		                                  proc->child_pid, file);

		_TC_MUST_GO(ns_file_formatting_res > -1, abort,
		            "errored formating proc userns-map file %s", file);
		_TC_MUST_GO(
		  ns_file_formatting_res < (int)sizeof(path), abort,
		  "got path too long formating proc userns-map file %s", file);

		_TC_DEBUG("writing remapping to userns file %s", path);

		_TC_MUST_P_GO((uid_map_fd = open(path, O_WRONLY)) != -1, "open",
		              abort, "failed to open file %s", path);

		_TC_MUST_GO(dprintf(uid_map_fd, "0 %d %d\n", USERNS_OFFSET,
		                    USERNS_COUNT) != -1,
		            abort, "failed to write userns mapping to file %s",
		            path);

		close(uid_map_fd);
		uid_map_fd = -1;
	}

	_TC_DEBUG("finished userns remap");
	_TC_DEBUG("writing to child");

	_TC_MUST_P_GO(
	  write(proc->child_ipc_socket, &ok, sizeof(int)) == sizeof(int),
	  "write", abort, "failed writing back response to child (socket=%d)",
	  proc->child_ipc_socket);

	return 0;

abort:
	if (uid_map_fd != -1) {
		close(uid_map_fd);
		uid_map_fd = -1;
	}
	return 1;
}
