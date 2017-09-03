#include "./cli.h"

typedef enum _cli_state {
	_TC_CLI_STATE_LOOKUP_FLAGS = 1,
	_TC_CLI_STATE_LOOKUP_POSITIONALS
} _tc_cli_state;

int
tc_cli_parse(tc_cli_t* cli, int argc, __attribute__((unused)) char** argv)
{
	_tc_cli_state state = _TC_CLI_STATE_LOOKUP_FLAGS;
	char* current_arg;

	size_t current_arg_len;
	size_t number_of_flags = 0;

	if (!cli) {
		return 1;
	}

	if (argc < 1) {
		return 1;
	}

	for (int ndx = 0; ndx < argc - 1; ndx++) {
		current_arg = argv[ndx];
		current_arg_len = strlen(current_arg);

		if (current_arg_len < 3) {
			state = _TC_CLI_STATE_LOOKUP_POSITIONALS;
			break;
		}

		if (current_arg[0] != '-' || current_arg[1] != '-') {
			state = _TC_CLI_STATE_LOOKUP_POSITIONALS;
			break;
		}

		number_of_flags++;
	}

	return 0;
}

void
tc_cli_help()
{
	fprintf(stderr, tc_cli_msg_help);
}
