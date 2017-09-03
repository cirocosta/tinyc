#include "./cli.h"

int
tc_cli_parse(tc_cli_t* cli, int __attribute__((unused)) argc,
             char** __attribute__((unused)) argv)
{
	if (!cli) {
		return 1;
	}

	if (cli->argc < 1) {
		return 1;
	}

	return 0;
}

void
tc_cli_help()
{
	fprintf(stderr, tc_cli_msg_help);
}
