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

	_TC_MUST(res == 1, "should have failed succeeded");
	_TC_MUST(cli.argc == 0, "should've captured 0 arguments");
	_TC_INFO("OK");
}

void
test_retrieves_positional_wout_flags()
{
	tc_cli_t cli = { 0 };
	int argc = 2;
	char* argv[] = {
		"tinyc",
		"haha",
	};
	int res = tc_cli_parse(&cli, argc, argv);
	char* expected = argv[1];
	char* actual = cli.argv[0];

	_TC_MUST(!res, "should have succeeded");
	_TC_MUST(cli.argc == 1, "should've captured 1 positional argument");

	_TC_MUST(!strcmp(expected, actual),
	         "strings [%s] and [%s] don't match",
	         expected,
	         actual);
	_TC_INFO("OK");
}

void
test_retrieves_help_flag()
{
	tc_cli_t cli = { 0 };
	int argc = 2;
	char* argv[] = {
		"tinyc",
		"--help",
	};
	int res = tc_cli_parse(&cli, argc, argv);

	_TC_MUST(!res, "should have succeeded");
	_TC_MUST(cli.argc == 0, "should've captured 0 positional arguments");
	_TC_MUST(cli.help == true, "should have set help to true");

	_TC_INFO("OK");
}

void
test_retrieves_rootfs_flag()
{
	tc_cli_t cli = { 0 };
	int argc = 2;
	char* argv[] = {
		"tinyc",
		"--rootfs=/tmp/rootfs",
	};
	int res = tc_cli_parse(&cli, argc, argv);

	char* expected = "/tmp/rootfs";
	char* actual = cli.rootfs;

	_TC_MUST(!res, "should have succeeded");
	_TC_MUST(cli.argc == 0, "should've captured 0 positional arguments");
	_TC_MUST(cli.rootfs != NULL, "should've captured a rootfs arg");
	_TC_MUST(!strcmp(expected, actual),
	         "strings [%s] and [%s] don't match",
	         expected,
	         actual);

	_TC_INFO("OK");
}

void
test_retrieves_env_flags()
{
	tc_cli_t cli = { 0 };
	int argc = 4;
	char* argv[] = {
		"tinyc",
		"--env=foo=bar",
		"--env=caz=baz",
		"abc",
	};
	int res = tc_cli_parse(&cli, argc, argv);

	char* expected_env1 = "foo=bar";
	char* expected_env2 = "caz=baz";
	char* expected_pos_arg = "abc";

	_TC_MUST(!res, "should have succeeded");
	_TC_MUST(cli.argc == 1, "should've captured 0 positional arguments");
	_TC_MUST(cli.envc == 2, "should've captured 1 env vars");
	_TC_MUST(cli.rootfs == NULL, "should've captured no rootfs flag");

	_TC_MUST(!strcmp(expected_env1, cli.envp[0]),
	         "strings [%s] and [%s] don't match",
	         expected_env1,
	         cli.envp[0]);
	_TC_MUST(!strcmp(expected_env2, cli.envp[1]),
	         "strings [%s] and [%s] don't match",
	         expected_env2,
	         cli.envp[1]);
	_TC_MUST(!strcmp(expected_pos_arg, cli.argv[0]),
	         "strings [%s] and [%s] don't match",
	         expected_pos_arg,
	         cli.argv[0]);

	_TC_INFO("OK");
	tc_cli_cleanup(&cli);
}

int
main()
{
	test_fails_if_no_args();
	test_fails_if_nil_pointer();
	test_retrieves_retrieves_nothing_if_just_argv0();
	test_retrieves_positional_wout_flags();
	test_retrieves_help_flag();
	test_retrieves_rootfs_flag();
	test_retrieves_env_flags();
}
