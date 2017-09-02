#define _GNU_SOURCE
#include <fcntl.h>
#include <grp.h>
#include <linux/capability.h>
#include <linux/limits.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/capability.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

// TODO this could come from a '='-separated
//      configuration file.
#define USERNS_OFFSET 10000
#define USERNS_COUNT 2000

#include "./common.h"

#define STACK_SIZE _TC_MB(1)

typedef struct proc_t {
	uid_t uid;
	int argc;
	int parent_socket;
	char** argv;
	char** envp;
	char* rootfs;
	char* hostname;
	char* mount_dir;
	char* stack;
} tc_proc_t;

typedef struct tc_t {
	pid_t child_pid;
	int sockets[2];
	int err;
} tc_tc_t;

static const int tc_proc_flags = CLONE_NEWNS | CLONE_NEWCGROUP | CLONE_NEWPID |
                                 CLONE_NEWIPC | CLONE_NEWNET | CLONE_NEWUTS;

// TODO this could be configured
static const int tc_dropped_capabilities[] = {
	CAP_AUDIT_CONTROL,   // enable/disable kernel auditing
	CAP_AUDIT_READ,      // reading kernel audit log via netlink socket
	CAP_AUDIT_WRITE,     // write records to the kernel auditing log
	CAP_BLOCK_SUSPEND,   // allows preventing system to suspend
	CAP_DAC_READ_SEARCH, // bypass file permission checks
	CAP_FSETID,          // TODO understand what can be done with this
	CAP_IPC_LOCK,        // allows locking more memory than allowed
	CAP_MAC_ADMIN,       // override mandatory access control
	CAP_MAC_OVERRIDE,    // change access control
	CAP_MKNOD,           // create special files
	CAP_SETFCAP,         // set file capabilities
	CAP_SYSLOG,          // privileged syslog operations
	CAP_SYS_ADMIN,       // things like mount, setns, quotactl, swapon/off
	CAP_SYS_BOOT,        // reboot the sys, load kernel mods ...
	CAP_SYS_MODULE,      // kernel module operations
	CAP_SYS_NICE,        // set higher priorities in the scheduler
	CAP_SYS_RAWIO,       // full access to system memory
	CAP_SYS_RESOURCE, // circumventing kernel-wide limits, quotas, rlimits
	CAP_SYS_TIME,     // system clock ...
	CAP_WAKE_ALARM,   // interfere with suspended state
};

int
tc_child_capabilities()
{
	size_t num_caps =
	  sizeof(tc_dropped_capabilities) / sizeof(*tc_dropped_capabilities);
	cap_t caps;

	_TC_DEBUG("Dropping capabilities");

	for (size_t i = 0; i < num_caps; i++) {
		_TC_MUST_P(prctl(PR_CAPBSET_DROP, tc_dropped_capabilities[i], 0,
		                 0, 0) != -1,
		           "prctl",
		           "Couldn't drop capability %d for the current proc",
		           tc_dropped_capabilities[i]);
	}

	_TC_DEBUG("Setting inheritable capabilities");

	_TC_MUST_P((caps = cap_get_proc()) != NULL, "cap_get_proc",
	           "couldn't allocate proc capability state");

	_TC_MUST_P(cap_set_flag(caps, CAP_INHERITABLE, num_caps,
	                        tc_dropped_capabilities, CAP_CLEAR) != -1,
	           "cap_set_flag", "couldn't set flag of desired"
	                           " capabilities to inheritable");

	_TC_MUST_P(cap_set_proc(caps) != -1, "cap_set_proc",
	           "couldn't set process capabilities from cap state");

	_TC_MUST_P(cap_free(caps), "cap_free",
	           "couldn't release memory allocate for capabilities");

	_TC_DEBUG("Capabilities dropped");
	return 0;
}

int
pivot_root(const char* new_root, const char* put_old)
{
	return syscall(SYS_pivot_root, new_root, put_old);
}

int
tc_child_mounts(tc_proc_t* config)
{
	_TC_DEBUG("remounting with MS_PRIVATE");

	if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL)) {
		fprintf(stderr, "failed! %m\n");
		return -1;
	}

	_TC_DEBUG("remounted");
	_TC_DEBUG("making a temp directory and a bind mount there");

	char mount_dir[] = "/tmp/tmp.XXXXXX";
	if (!mkdtemp(mount_dir)) {
		fprintf(stderr, "failed making a directory!\n");
		return -1;
	}

	if (mount(config->mount_dir, mount_dir, NULL, MS_BIND | MS_PRIVATE,
	          NULL)) {
		fprintf(stderr, "bind mount failed!\n");
		return -1;
	}

	char inner_mount_dir[] = "/tmp/tmp.XXXXXX/oldroot.XXXXXX";
	memcpy(inner_mount_dir, mount_dir, sizeof(mount_dir) - 1);
	if (!mkdtemp(inner_mount_dir)) {
		fprintf(stderr, "failed making the inner directory!\n");
		return -1;
	}

	_TC_DEBUG("done");
	_TC_DEBUG("pivoting root...");

	if (pivot_root(mount_dir, inner_mount_dir)) {
		fprintf(stderr, "failed!\n");
		return -1;
	}
	fprintf(stderr, "done.\n");

	char* old_root_dir = basename(inner_mount_dir);
	char old_root[sizeof(inner_mount_dir) + 1] = { "/" };
	strcpy(&old_root[1], old_root_dir);

	fprintf(stderr, "=> unmounting %s...", old_root);
	if (chdir("/")) {
		fprintf(stderr, "chdir failed! %m\n");
		return -1;
	}
	if (umount2(old_root, MNT_DETACH)) {
		fprintf(stderr, "umount failed! %m\n");
		return -1;
	}
	if (rmdir(old_root)) {
		fprintf(stderr, "rmdir failed! %m\n");
		return -1;
	}
	fprintf(stderr, "done.\n");
	return 0;
}

int
tc_child(void* arg)
{
	tc_proc_t* proc = arg;

	_TC_MUST_P_GO(!sethostname(proc->hostname, strlen(proc->hostname)),
	              "sethostname", abort, "couldn't set hostname to %s",
	              proc->hostname);
	_TC_MUST_GO(!tc_child_mounts(proc), abort, "couldn't set child mounts");
	// tc_child_userns(proc)
	_TC_MUST_GO(!tc_child_capabilities(), abort,
	            "couldn't set capabilities");
	// tc_child_syscalls();

	if (close(proc->parent_socket)) {
		fprintf(stderr, "close failed: %m\n");
		return -1;
	}

	if (execve(proc->argv[0], proc->argv, NULL)) {
		fprintf(stderr, "execve failed! %m.\n");
		return -1;
	}

	return 0;

abort:
	close(proc->parent_socket);
	return 1;
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

// TODO improve socket communication with enum
//      to better handle information sharing.
int
tc_set_userns(tc_proc_t* config)
{
	int result = 0;
	int has_userns = !unshare(CLONE_NEWUSER);
	gid_t gid = (gid_t)config->uid;

	_TC_MUST_P(write(config->parent_socket, &has_userns,
	                 sizeof(has_userns)) == sizeof(has_userns),
	           "write", "failed to write to parent");

	_TC_MUST_P(read(config->parent_socket, &result, sizeof(result)) ==
	             sizeof(result),
	           "read", "failed to read from parent");

	if (result != 0) {
		return -1;
	}

	_TC_DEBUG("=> switching to uid %d / gid %d...", config->uid,
	          config->uid);

	_TC_MUST_P((!setgroups(1, &gid)), "setgroups",
	           "failed to set process user group");
	_TC_MUST_P((!setresgid(config->uid, config->uid, config->uid)),
	           "setresgid", "failed to set real gid");
	_TC_MUST_P(!(setresuid(config->uid, config->uid, config->uid)),
	           "setresuid", "failed to set real uid");

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
