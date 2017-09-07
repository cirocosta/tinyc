#include "./cli.h"
#include "./common.h"

int
tc_cli_parse(tc_cli_t* cli, int argc, char** argv)
{
	int ndx;
	char* current_arg;
	size_t current_arg_len;

	char* env[16] = { 0 };
	int envc = 0;

	if (!cli) {
		return 1;
	}

	if (argc < 2) {
		return 1;
	}

	for (ndx = 1; ndx < argc; ndx++) {
		current_arg = argv[ndx];
		current_arg_len = strlen(current_arg);

		if (current_arg_len < 3) {
			break;
		}

		if (current_arg[0] != '-' || current_arg[1] != '-') {
			break;
		}

		if (!strcmp(current_arg, TC_FLAG_HELP.name)) {
			cli->help = true;
			continue;
		}

		if (strstr(current_arg, TC_FLAG_ROOTFS.name)) {
			cli->rootfs = current_arg + TC_FLAG_ROOTFS.name_len + 1;
			continue;
		}

		if (strstr(current_arg, TC_FLAG_ENV.name)) {
			env[envc++] = current_arg + TC_FLAG_ENV.name_len + 1;
			continue;
		}
	}

	if (envc > 0) {
		cli->envp = malloc(sizeof(cli->envp) * envc);
		if (cli->envp == NULL) {
			return 1;
		}

		cli->envc = envc;
		for (int i = 0; i < envc; i++) {
			cli->envp[i] = env[i];
		}
	}

	cli->argc = argc - ndx;
	cli->argv = argv + ndx;

	return 0;
}

void
tc_cli_help()
{
	tc_cli_flag_t const* flag;
	fprintf(stderr, tc_cli_msg_help_header);

	for (int i = 0; i < tc_cli_flags_len; i++) {
		flag = tc_cli_flags[i];
		fprintf(stderr, "    %s\t\t%s\n", flag->name,
		        flag->description);
	}
	fprintf(stderr, "\n");
}

void
tc_cli_cleanup(tc_cli_t* cli)
{
	if (cli == NULL) {
		return;
	}

	if (cli->envp != NULL) {
		free(cli->envp);
	}
}
