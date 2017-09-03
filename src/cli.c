#include "./cli.h"

tc_cli_t*
tc_cli_parse(int __attribute__((unused)) argc,
             char** __attribute__((unused)) argv)
{
	return NULL;
}

void
tc_cli_help()
{
	fprintf(stderr, tc_cli_msg_help);
}
