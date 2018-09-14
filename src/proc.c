#include "./proc.h"

int
tc_proc_init(tc_proc_t* proc)
{
	int err = 0;

	_TC_DEBUG("initializing proc structure");

	if (proc->rootfs != NULL) {
		err = tc_proc_dir_exists(proc->rootfs);
		if (err) {
			_TC_INFO("specified rootfs does not exists - %s",
			         proc->rootfs);
			goto abort;
		}
	}

	if (proc->disable_userns_remap == false) {
		err = tc_proc_init_ipc(proc);
		if (err) {
			goto abort;
		}
	}

	if (proc->disable_cgroups == false) {
		err = tc_proc_set_cgroups(proc);
		if (err) {
			goto abort;
		}
	}

	_TC_MUST_P_GO((proc->stack = malloc(TC_DEFAULT_STACK_SIZE)),
	              "malloc",
	              abort,
	              "couldn't allocate memory to container process stack");

	_TC_DEBUG("process stack allocated (stack=%p)", proc->stack);

	return 0;

abort:
	_TC_INFO("failed initializing proc structure");
	return 1;
}

int
tc_proc_init_ipc(tc_proc_t* proc)
{
	int sockets[2] = { 0 };

	_TC_MUST_P_GO(
	  (socketpair(AF_LOCAL, SOCK_SEQPACKET, 0, sockets)) == 0,
	  "socketpair",
	  abort,
	  "couldn't create socket pair for parent-child communication");

	proc->parent_ipc_socket = sockets[0];
	proc->child_ipc_socket = sockets[1];

	_TC_MUST_P_GO((fcntl(proc->parent_ipc_socket, F_SETFD, FD_CLOEXEC)) ==
	                0,
	              "fcntl",
	              abort,
	              "couldn't set FD_CLOEXEC bit on parent-socket");

	_TC_DEBUG("socket pair set (parent=%d,child=%d)",
	          proc->parent_ipc_socket,
	          proc->child_ipc_socket);
	return 0;

abort:
	_TC_INFO("failed initializing ipc components");
	return 1;
}

int
tc_proc_set_cgroups(tc_proc_t* proc)
{
	char path[PATH_MAX] = { 0 };
	char dir[PATH_MAX] = { 0 };
	int fd = 0;

	tc_proc_cgroup_setting** setting;
	tc_proc_cgroup const** cgrp = tc_proc_cgroups;

	_TC_INFO("configuring cgroups");

	for (; *cgrp; cgrp++) {
		_TC_INFO("[cgroups] configuring %s", (*cgrp)->subsystem);

		// TODO separate this into a function
		if (snprintf(dir,
		             sizeof(dir),
		             "/sys/fs/cgroup/%s/%s",
		             (*cgrp)->subsystem,
		             proc->hostname) == -1) {
			goto abort;
		}

		if (mkdir(dir, S_IRUSR | S_IWUSR | S_IXUSR)) {
			fprintf(stderr, "mkdir %s failed: %m\n", dir);
			goto abort;
		}

		for (setting = (*cgrp)->settings; *setting; setting++) {
			_TC_INFO("[cgroups] configuring property %s",
			         (*setting)->name);

			// TODO separate this into a function
			if (snprintf(path,
			             sizeof(path),
			             "%s/%s",
			             dir,
			             (*setting)->name) == -1) {
				fprintf(stderr, "snprintf failed: %m\n");
				goto abort;
			}
			if ((fd = open(path, O_WRONLY)) == -1) {
				fprintf(
				  stderr, "opening %s failed: %m\n", path);
				goto abort;
			}
			if (write(fd,
			          (*setting)->value,
			          strlen((*setting)->value)) == -1) {
				fprintf(
				  stderr, "writing to %s failed: %m\n", path);
				close(fd);
				goto abort;
			}

			close(fd);
			memset(path, 0, PATH_MAX);
		}

		memset(dir, 0, PATH_MAX);
	}

	return 0;

abort:
	_TC_INFO("failed configuring cgroups");
	return 1;
}

int
tc_proc_clean_cgroups(tc_proc_t* proc)
{
	tc_proc_cgroup const** cgrp = tc_proc_cgroups;
	char dir[PATH_MAX] = { 0 };
	char task[PATH_MAX] = { 0 };
	int task_fd = 0;

	_TC_INFO("cleaning cgroups");

	for (; *cgrp; cgrp++) {
		if (snprintf(dir,
		             sizeof(dir),
		             "/sys/fs/cgroup/%s/%s",
		             (*cgrp)->subsystem,
		             proc->hostname) == -1 ||
		    snprintf(task,
		             sizeof(task),
		             "/sys/fs/cgroup/%s/tasks",
		             (*cgrp)->subsystem) == -1) {
			fprintf(stderr, "snprintf failed: %m\n");
			goto abort;
		}

		if ((task_fd = open(task, O_WRONLY)) == -1) {
			fprintf(stderr, "opening %s failed: %m\n", task);
			goto abort;
		}

		if (write(task_fd, "0", 2) == -1) {
			fprintf(stderr, "writing to %s failed: %m\n", task);
			close(task_fd);
			goto abort;
		}

		close(task_fd);
		if (rmdir(dir)) {
			fprintf(stderr, "rmdir %s failed: %m", dir);
			goto abort;
		}

		memset(dir, 0, PATH_MAX);
		memset(task, 0, PATH_MAX);
	}

	return 0;

abort:
	_TC_INFO("failed cleaning cgroups");
	return 1;
}

int
tc_proc_set_rlimits()
{
	_TC_INFO("setting resource limits");

	struct rlimit limits = {
		.rlim_max = TC_DEFAULT_RLIMIT_NOFILE_COUNT,
		.rlim_cur = TC_DEFAULT_RLIMIT_NOFILE_COUNT,
	};

	if (setrlimit(RLIMIT_NOFILE, &limits)) {
		fprintf(stderr, "failed: %m\n");
		goto abort;
	}

	return 0;

abort:
	_TC_INFO("failed setting resource limits");
	return 1;
}

int
tc_proc_dir_exists(char* dir)
{
	struct stat s = { 0 };
	int err = stat(dir, &s);

	if (err) {
		return 1;
	}

	if (s.st_mode & S_IFDIR) {
		return 0;
	}

	return 1;
}

int
tc_proc_run(tc_proc_t* proc, int (*child_fn)(void*))
{
	_TC_DEBUG("starting to run process");

	tc_proc_show(proc);

	_TC_MUST_P_GO(
	  (proc->child_pid = clone(child_fn,
	                           proc->stack + TC_DEFAULT_STACK_SIZE,
	                           tc_proc_flags | SIGCHLD,
	                           proc)) != -1,
	  "clone",
	  abort,
	  "couldn't create child process");

	if (proc->disable_userns_remap == false) {
		_TC_MUST_GO(!tc_proc_handle_child_uid_remap(proc),
		            abort,
		            "failed performing userns remap");
	}

	_TC_DEBUG("waiting child from pid %d", proc->child_pid);

	_TC_MUST_P_GO(waitpid(proc->child_pid, NULL, 0) != -1,
	              "waitpid",
	              abort,
	              "failed waiting on child_pid %d",
	              proc->child_pid);

	_TC_DEBUG("child from pid %d just finished.", proc->child_pid);
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

	if (proc->disable_cgroups == false) {
		tc_proc_clean_cgroups(proc);
	}
}

void
tc_proc_show(tc_proc_t* proc)
{
	fprintf(stderr, "Configuration:\n");
	fprintf(stderr,
	        "  uid:                   %d\n"
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
	        proc->uid,
	        proc->argc,
	        proc->envpc,
	        proc->hostname,
	        proc->rootfs,
	        proc->child_pid,
	        proc->parent_ipc_socket,
	        proc->child_ipc_socket,
	        proc->disable_seccomp,
	        proc->disable_capabilities,
	        proc->disable_userns_remap);

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
	"uid_map",
	"gid_map",
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

	_TC_MUST_P_GO(
	  (read(proc->child_ipc_socket, &has_userns, sizeof(has_userns)) ==
	   sizeof(has_userns)),
	  "read",
	  abort,
	  "couldn't read userns config from child");

	if (!has_userns) {
		_TC_MUST_P_GO(write(proc->child_ipc_socket, 0, sizeof(int)) ==
		                sizeof(int),
		              "write",
		              abort,
		              "failed writing back response to child");
		return 0;
	}

	_TC_DEBUG("starting userns remap");

	for (size_t ndx = 0; ndx < 2; ndx++) {
		file = tc_proc_userns_files[ndx];

		ns_file_formatting_res = snprintf(
		  path, PATH_MAX, "/proc/%d/%s", proc->child_pid, file);

		_TC_MUST_GO(ns_file_formatting_res > -1,
		            abort,
		            "errored formating proc userns-map file %s",
		            file);
		_TC_MUST_GO(
		  ns_file_formatting_res < (int)sizeof(path),
		  abort,
		  "got path too long formating proc userns-map file %s",
		  file);

		_TC_DEBUG("writing remapping to userns file %s", path);

		_TC_MUST_P_GO((uid_map_fd = open(path, O_WRONLY)) != -1,
		              "open",
		              abort,
		              "failed to open file %s",
		              path);

		_TC_MUST_GO(dprintf(uid_map_fd,
		                    "0 %d %d\n",
		                    TC_DEFAULT_USERNS_OFFSET,
		                    TC_DEFAULT_USERNS_COUNT) != -1,
		            abort,
		            "failed to write userns mapping to file %s",
		            path);

		close(uid_map_fd);
		uid_map_fd = -1;
	}

	_TC_DEBUG("finished userns remap");
	_TC_DEBUG("writing to child");

	_TC_MUST_P_GO(write(proc->child_ipc_socket, &ok, sizeof(int)) ==
	                sizeof(int),
	              "write",
	              abort,
	              "failed writing back response to child (socket=%d)",
	              proc->child_ipc_socket);

	return 0;

abort:
	if (uid_map_fd != -1) {
		close(uid_map_fd);
		uid_map_fd = -1;
	}
	return 1;
}
