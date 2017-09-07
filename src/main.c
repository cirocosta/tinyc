#include <stdlib.h>
#include <sys/time.h>

#include "./child.h"
#include "./cli.h"
#include "./common.h"
#include "./names.h"
#include "./proc.h"

int
main(int argc, char** argv)
{
	struct timeval time;
	tc_cli_t cli = { 0 };
	tc_proc_t proc = { 0 };
	int err = 0;

	if (tc_cli_parse(&cli, argc, argv)) {
		tc_cli_help();
		fprintf(stderr,
		        "ERROR: Couldn't properly parse CLI arguments.\n"
		        "Make sure you're passing the right arguments.\n"
		        "Aborting.");
		exit(1);
	}

	if (cli.help == true) {
		tc_cli_help();
		exit(0);
	}

	gettimeofday(&time, NULL);
	srand((time.tv_sec * 1000) + (time.tv_usec / 1000));

	proc.rootfs = cli.rootfs;
	proc.envp = cli.envp;
	proc.envpc = cli.envc;
	proc.argv = cli.argv;
	proc.argc = cli.argc;

	tc_names_fill(proc.hostname, 255);
	tc_proc_show(&proc);

	err = tc_proc_init(&proc);
	if (err) {
		fprintf(stderr,
		        "ERROR: Couldn't properly run the application.\n"
		        "Aborting.");
		goto abort;
	}

	err = tc_proc_run(&proc, tc_child);
	if (err) {
		fprintf(stderr,
		        "ERROR: Couldn't properly run the application.\n"
		        "Aborting.");
		goto abort;
	}

	return 0;

abort:
	tc_cli_cleanup(&cli);
	tc_proc_cleanup(&proc);
	return 1;
}
