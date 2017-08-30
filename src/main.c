#include <stdio.h>

typedef struct conf {
	int argc;
	char** argv;
	char** envp;
	char* rootfs;
	char* hostname;
} conf_t;

int
main()
{
	printf("hello\n");
	return 0;
}
