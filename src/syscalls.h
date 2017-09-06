#ifndef TC__SYSCALLS_H
#define TC__SYSCALLS_H

#define _GNU_SOURCE
#include <sys/syscall.h>
#include <unistd.h>

__attribute__((always_inline)) int
pivot_root(const char* new_root, const char* put_old)
{
	return syscall(SYS_pivot_root, new_root, put_old);
}

#endif
