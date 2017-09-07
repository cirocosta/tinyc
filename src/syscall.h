#ifndef TC__SYSCALL_H
#define TC__SYSCALL_H

#define _GNU_SOURCE
#include <sys/syscall.h>
#include <unistd.h>

inline int
tc_syscall_pivot_root(const char* new_root, const char* put_old)
{
	return syscall(SYS_pivot_root, new_root, put_old);
}

#endif
