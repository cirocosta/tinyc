#include <string.h>

#include "../src/cli.h"
#include "../src/common.h"

void
test_fails_if_no_args()
{
	int argc = 0;
	char* argv[] = {};
	tc_cli_t cli = { 0 };
	int res = tc_cli_parse(&cli, argc, argv);

	_TC_MUST(res, "should have failed");
	_TC_INFO("OK");
}

void
test_fails_if_nil_pointer()
{
	int argc = 0;
	char* argv[] = {};
	int res = tc_cli_parse(NULL, argc, argv);

	_TC_MUST(res, "should have failed");
	_TC_INFO("OK");
}

void
test_retrieves_argv_if_no_flag()
{
	int argc = 1;
	char* argv[] = {
		"haha",
	};
	tc_cli_t cli = { 0 };
	int res = tc_cli_parse(&cli, argc, argv);

	_TC_MUST(!res, "should have succeeded");
	_TC_MUST(cli.argc == 1, "should have updated argc");

	_TC_MUST(!strncmp(argv[0], cli.argv[0], 5),
	         "strings [%s] and [%s] don't match", argv[0], cli.argv[0]);

	_TC_INFO("OK");
}

int
main()
{
	test_fails_if_no_args();
	test_fails_if_nil_pointer();
	test_retrieves_argv_if_no_flag();
}
