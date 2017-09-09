#ifndef TC__CHILD_H
#define TC__CHILD_H

#define _GNU_SOURCE

#include <errno.h>
#include <grp.h>
#include <libgen.h>
#include <seccomp.h>
#include <stdlib.h>
#include <string.h>
#include <sys/capability.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "./common.h"
#include "./proc.h"
#include "./syscall.h"

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

typedef struct child_seccomp_mas {
	uint32_t action;
	int syscall;
	unsigned int arg_cnt;
	struct scmp_arg_cmp cmp;
} tc_child_seccomp_mask;

static const tc_child_seccomp_mask tc_child_seccomp_masks[] =
  {
    {.action = SCMP_ACT_ERRNO(EPERM),
     .syscall = SCMP_SYS(chmod),
     1,
     SCMP_A1(SCMP_CMP_MASKED_EQ, S_ISUID, S_ISUID) },
    {
      .action = SCMP_ACT_ERRNO(EPERM),
      .syscall = SCMP_SYS(chmod),
      1,
      SCMP_A1(SCMP_CMP_MASKED_EQ, S_ISGID, S_ISGID),
    },
    {.action = SCMP_ACT_ERRNO(EPERM),
     .syscall = SCMP_SYS(fchmod),
     1,
     SCMP_A1(SCMP_CMP_MASKED_EQ, S_ISUID, S_ISUID) },
    {.action = SCMP_ACT_ERRNO(EPERM),
     .syscall = SCMP_SYS(fchmod),
     1,
     SCMP_A1(SCMP_CMP_MASKED_EQ, S_ISGID, S_ISGID) },
    {.action = SCMP_ACT_ERRNO(EPERM),
     .syscall = SCMP_SYS(fchmodat),
     1,
     SCMP_A2(SCMP_CMP_MASKED_EQ, S_ISUID, S_ISUID) },
    {.action = SCMP_ACT_ERRNO(EPERM),
     .syscall = SCMP_SYS(fchmodat),
     1,
     SCMP_A2(SCMP_CMP_MASKED_EQ, S_ISGID, S_ISGID) },
    {.action = SCMP_ACT_ERRNO(EPERM),
     .syscall = SCMP_SYS(unshare),
     1,
     SCMP_A0(SCMP_CMP_MASKED_EQ, CLONE_NEWUSER, CLONE_NEWUSER) },
    {.action = SCMP_ACT_ERRNO(EPERM),
     .syscall = SCMP_SYS(clone),
     1,
     SCMP_A0(SCMP_CMP_MASKED_EQ, CLONE_NEWUSER, CLONE_NEWUSER) },
    {.action = SCMP_ACT_ERRNO(EPERM),
     .syscall = SCMP_SYS(ioctl),
     1,
     SCMP_A1(SCMP_CMP_MASKED_EQ, TIOCSTI, TIOCSTI) },
    {.action = SCMP_ACT_ERRNO(EPERM), .syscall = SCMP_SYS(keyctl), 0 },
    {.action = SCMP_ACT_ERRNO(EPERM), .syscall = SCMP_SYS(add_key), 0 },
    {.action = SCMP_ACT_ERRNO(EPERM), .syscall = SCMP_SYS(request_key), 0 },
    {.action = SCMP_ACT_ERRNO(EPERM) },
    {.action = SCMP_ACT_ERRNO(EPERM), .syscall = SCMP_SYS(ptrace), 0 },
    {.action = SCMP_ACT_ERRNO(EPERM), .syscall = SCMP_SYS(mbind), 0 },
    {.action = SCMP_ACT_ERRNO(EPERM), .syscall = SCMP_SYS(migrate_pages), 0 },
    {.action = SCMP_ACT_ERRNO(EPERM), .syscall = SCMP_SYS(move_pages), 0 },
    {.action = SCMP_ACT_ERRNO(EPERM), .syscall = SCMP_SYS(set_mempolicy), 0 },
    {.action = SCMP_ACT_ERRNO(EPERM), .syscall = SCMP_SYS(userfaultfd), 0 },
    {.action = SCMP_ACT_ERRNO(EPERM), .syscall = SCMP_SYS(perf_event_open), 0 },
  };

static const size_t tc_child_seccomp_masks_len =
  sizeof(tc_child_seccomp_masks) / sizeof(*tc_child_seccomp_masks);

int tc_child_mounts(tc_proc_t* config);

int tc_child_capabilities();

int tc_child_main(void* arg);

int tc_child_block_syscalls();

int tc_child_set_userns(tc_proc_t* config);

int tc_child_mount_procfs();

#endif
