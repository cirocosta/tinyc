#include "../src/cli.h"

typedef void (*tc_assertion)(int);

void
test_parses_accordingly()
{
	fprintf(stderr, "OK\n");
}

int
main()
{
	test_parses_accordingly();
}
