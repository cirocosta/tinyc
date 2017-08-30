#include <stdio.h>
#include <stdlib.h>
#include "./common.h"

#define STACK_SIZE (1024 * 1024)

typedef struct proc_t {
	int argc;
	char** argv;
	char** envp;
	char* rootfs;
	char* hostname;
	char* stack;
} proc_t;

int
main()
{
	proc_t proc = { 0 };

	_TC_MUST_P((proc.stack = malloc(STACK_SIZE)), "malloc",
	           "couldn't allocate memory to container process stack");

	printf("hello\n");
	return 0;
}
