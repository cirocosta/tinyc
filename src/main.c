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

	// CC:  parses the command line arguments
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

	// CC:  initialize the seed for our random choice of
	//      container name
	gettimeofday(&time, NULL);
	srand((time.tv_sec * 1000) + (time.tv_usec / 1000));

	proc.envpc = cli.envc;
	proc.argv = cli.argv;
	proc.argc = cli.argc;
	proc.disable_userns_remap = cli.userns_remap == false;

	// CC:  fill the hostname of the process that we'll
	//      start.
	tc_names_fill(proc.hostname, 255);
	tc_proc_show(&proc);

	err = tc_proc_init(&proc);
	if (err) {
		fprintf(stderr,
		        "ERROR: Couldn't properly run the application.\n"
		        "Aborting.");
		goto abort;
	}

	err = tc_proc_run(&proc, tc_child_main);
	if (err) {
		fprintf(stderr,
		        "ERROR: Couldn't properly run the application.\n"
		        "Aborting.");
		goto abort;
	}

	tc_cli_cleanup(&cli);
	tc_proc_cleanup(&proc);

	return 0;

abort:
	tc_cli_cleanup(&cli);
	tc_proc_cleanup(&proc);
	return 1;
}
