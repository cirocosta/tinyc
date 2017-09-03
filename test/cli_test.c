#include <string.h>

#include "../src/cli.h"
#include "../src/common.h"

void
test_fails_if_no_args()
{
	tc_cli_t cli = { 0 };
	int argc = 0;
	char* argv[] = {};
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
test_retrieves_retrieves_nothing_if_just_argv0()
{
	tc_cli_t cli = { 0 };
	int argc = 1;
	char* argv[] = {
		"tinyc",
	};
	int res = tc_cli_parse(&cli, argc, argv);

	_TC_MUST(!res, "should have succeeded");
	_TC_MUST(cli.argc == 0, "should've captured 0 arguments");
	_TC_INFO("OK");
}

void
test_retrieves_positional_wout_flags()
{
	tc_cli_t cli = { 0 };
	int argc = 2;
	char* argv[] = {
		"tinyc", "haha",
	};
	int res = tc_cli_parse(&cli, argc, argv);
	char* expected = argv[1];
	char* actual = cli.argv[0];

	_TC_MUST(!res, "should have succeeded");
	_TC_MUST(cli.argc == 1, "should've captured 1 argument");

	_TC_MUST(!strcmp(expected, actual), "strings [%s] and [%s] don't match",
	         expected, actual);
	_TC_INFO("OK");
}

int
main()
{
	test_fails_if_no_args();
	test_fails_if_nil_pointer();
	test_retrieves_retrieves_nothing_if_just_argv0();
	test_retrieves_positional_wout_flags();
}
