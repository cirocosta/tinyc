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

int
main()
{
	test_fails_if_no_args();
}
