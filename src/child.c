#include "./child.h"

/**
 *      tc_child_dropped_capabilities is a list of capabilities
 *      that are droppbed when a container is created.
 */
static const int tc_child_dropped_capabilities[] = {
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

static const size_t tc_child_dropped_capabilities_len =
  sizeof(tc_child_dropped_capabilities) /
  sizeof(*tc_child_dropped_capabilities);

int
tc_child_capabilities()
{
	cap_t caps;

	_TC_DEBUG("Dropping capabilities");

	for (size_t i = 0; i < tc_child_dropped_capabilities_len; i++) {
		_TC_MUST_P(
		  prctl(PR_CAPBSET_DROP, tc_child_dropped_capabilities[i], 0, 0,
		        0) != -1,
		  "prctl", "Couldn't drop capability %d for the current proc",
		  tc_child_dropped_capabilities[i]);
	}

	_TC_DEBUG("Setting inheritable capabilities");

	_TC_MUST_P((caps = cap_get_proc()) != NULL, "cap_get_proc",
	           "couldn't allocate proc capability state");

	_TC_MUST_P(cap_set_flag(caps, CAP_INHERITABLE,
	                        tc_child_dropped_capabilities_len,
	                        tc_child_dropped_capabilities, CAP_CLEAR) != -1,
	           "cap_set_flag", "couldn't set flag of desired"
	                           " capabilities to inheritable");

	_TC_MUST_P(cap_set_proc(caps) != -1, "cap_set_proc",
	           "couldn't set process capabilities from cap state");

	_TC_MUST_P(!cap_free(caps), "cap_free",
	           "couldn't release memory allocated for capabilities");

	_TC_DEBUG("Capabilities dropped");
	return 0;
}

int
tc_child_mounts(tc_proc_t* config)
{
	char mount_dir[] = "/tmp/tmp.XXXXXX";
	char inner_mount_dir[] = "/tmp/tmp.XXXXXX/oldroot.XXXXXX";

	_TC_DEBUG("remounting with MS_PRIVATE (mount_dir=%s)", mount_dir);

	_TC_MUST_P(!mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL), "mount",
	           "couldn't remount '/' with MS_PRIVATE (mount_dir=%s)",
	           mount_dir);

	_TC_DEBUG("making temporary directory to bind rootfs to");

	_TC_MUST_P(mkdtemp(mount_dir) != NULL, "mkdtemp",
	           "couldn't create temporary directory");

	// mounting [src:config->mount_dir, dst:tmp_dir]
	if (mount(config->rootfs, mount_dir, NULL, MS_BIND | MS_PRIVATE,
	          NULL)) {
		fprintf(stderr, "bind mount failed!\n");
		return -1;
	}

	// Make inner_mount_dir's prefix be equal to mount_dir
	// TODO errcheck the copying
	memcpy(inner_mount_dir, mount_dir, sizeof(mount_dir) - 1);

	// creating the temporary directory inside 'mount_dir' (tmp)
	if (!mkdtemp(inner_mount_dir)) {
		fprintf(stderr, "failed making the inner directory!\n");
		return -1;
	}

	_TC_DEBUG("created temporary inner directory (inner_mount_dir=%s)",
	          inner_mount_dir);
	_TC_DEBUG("Pivoting root (new=%s,put_old=%s)", mount_dir,
	          inner_mount_dir);

	// moves the '/' of the process to the directory 'inner_mount_dir'
	// and makes 'mount_dir' the new root filesystem.

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
	// _TC_MUST_GO(!tc_child_mounts(proc), abort, "couldn't set child
	// mounts");
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
