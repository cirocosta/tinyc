#ifndef TC__PROC_H
#define TC__PROC_H

#define _GNU_SOURCE
#include <fcntl.h>
#include <linux/limits.h>
#include <linux/sched.h>
#include <sched.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "./common.h"

#define TC_DEFAULT_USERNS_OFFSET 10000
#define TC_DEFAULT_USERNS_COUNT 2000

#define TC_DEFAULT_CGROUP_MEMORY "1073741824"
#define TC_DEFAULT_CGROUP_KERNEL_MEMORY "1073741824"
#define TC_DEFAULT_CGROUP_CPU_SHARES "256"
#define TC_DEFAULT_CGROUP_PIDS "64"
#define TC_DEFAULT_CGROUP_BLKIO_WEIGHT "10"

#define TC_DEFAULT_RLIMIT_NOFILE_COUNT 64

#define TC_DEFAULT_STACK_SIZE _TC_MB(1)

/**
 *      tc_proc_t encapsulates the configurations
 *      of the container.
 */
typedef struct proc_t {
	// disables syscall filtering.
	bool disable_seccomp;

	// disables setting of capabilities.
	bool disable_capabilities;

	// disables userns remapping
	bool disable_userns_remap;

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
	char* rootfs;

	// child_pid holds the pid of the child that
	// has been created.
	pid_t child_pid;

	// parent_ipc_socket references a socket to
	// communicate with the parent process.
	int parent_ipc_socket;

	// child_ipc_socket references a socket to
	// communicate with the child process.
	int child_ipc_socket;

	// stack is the stack previously allocated
	// to the container init process.
	char* stack;
} tc_proc_t;

/**
 *      key-value representation of a setting that a
 *      specific part of a cgroup subsystem can take.
 */
typedef struct proc_cgroup_setting {
	char name[256];
	char value[256];
} tc_proc_cgroup_setting;

/**
 *      aggregation of all the settings that a cgroup
 *      subsystem can take
 */
typedef struct proc_cgroup {
	char subsystem[256];
	tc_proc_cgroup_setting** settings;
} tc_proc_cgroup;

/**
 *      list of all the cgroup substystem with their
 *      default settings.
 */
static const tc_proc_cgroup* tc_proc_cgroups[] =
  { &(tc_proc_cgroup){
      .subsystem = "memory",
      .settings =
	(tc_proc_cgroup_setting* []){
	  &(tc_proc_cgroup_setting){.name = "memory.limit_in_bytes",
	                            .value = TC_DEFAULT_CGROUP_MEMORY },
	  &(tc_proc_cgroup_setting){.name = "memory.kmem.limit_in_bytes",
	                            .value = TC_DEFAULT_CGROUP_MEMORY },
	  &(tc_proc_cgroup_setting){.name = "tasks", .value = "0" }, NULL } },
    &(tc_proc_cgroup){
      .subsystem = "cpu",
      .settings =
	(tc_proc_cgroup_setting* []){
	  &(tc_proc_cgroup_setting){.name = "cpu.shares",
	                            .value = TC_DEFAULT_CGROUP_CPU_SHARES },
	  &(tc_proc_cgroup_setting){.name = "tasks", .value = "0" }, NULL } },
    &(tc_proc_cgroup){
      .subsystem = "pids",
      .settings =
	(tc_proc_cgroup_setting* []){
	  &(tc_proc_cgroup_setting){.name = "pids.max",
	                            .value = TC_DEFAULT_CGROUP_PIDS },
	  &(tc_proc_cgroup_setting){.name = "tasks", .value = "0" }, NULL } },
    &(tc_proc_cgroup){
      .subsystem = "blkio",
      .settings =
	(tc_proc_cgroup_setting* []){
	  &(tc_proc_cgroup_setting){.name = "blkio.weight",
	                            .value = TC_DEFAULT_CGROUP_BLKIO_WEIGHT },
	  &(tc_proc_cgroup_setting){.name = "tasks", .value = "0" }, NULL } },
    NULL };

/**
 *      tc_proc_flags is a bitset-ted int that is
 *      used when 'clone(2)'ing to have the new process
 *      spwaned with a set of namespaces set.
 */
static const int tc_proc_flags = CLONE_NEWNS | CLONE_NEWCGROUP | CLONE_NEWPID |
                                 CLONE_NEWIPC | CLONE_NEWNET | CLONE_NEWUTS;

/**
 *      initializes the 'proc' structure, assigning
 *      real socket file descriptors to parent and
 *      child ipc as well as allocating a stack to
 *      the child proccess.
 */
int tc_proc_init(tc_proc_t* proc);

/**
 *      runs the child process whose entrypoint is specified
 *      as a function (argument 'fn').
 *
 *      note.:  must have 'tc_proc_t' properly initialized
 *              before.
 */
int tc_proc_run(tc_proc_t* proc, int (*fn)(void*));

/**
 *      handles the userns remapping of the child from the
 *      parent.
 */
int tc_proc_handle_child_uid_remap(tc_proc_t* proc);

/**
 *      dumps to 'stderr' the configuration that will
 *      be used by the process to be spawned.
 */
void tc_proc_show(tc_proc_t* proc);

/**
 *      cleans resources that were allocated to 'proc'.
 */
void tc_proc_cleanup(tc_proc_t* proc);

/**
 *      checks if a directory exists.
 */
int tc_proc_dir_exists(char* dir);

#endif
