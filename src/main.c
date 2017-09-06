#include <fcntl.h>
#include <grp.h>
#include <linux/capability.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/capability.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>

// TODO this could come from a '='-separated
//      configuration file.
#define USERNS_OFFSET 10000
#define USERNS_COUNT 2000

#include "./cli.h"
#include "./common.h"
#include "./names-generator.h"
#include "./proc.h"

int
main(int argc, char** argv)
{
	struct timeval time;
	tc_cli_t cli = { 0 };
	tc_proc_t proc = { 0 };
	tc_tc_t program = {
		.sockets = { 0 }, .err = 0,
	};

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

	proc.argv = cli.argv;
	proc.argc = cli.argc;

	tc_fill_with_name(proc.hostname, 255);
	tc_proc_show(&proc);
	tc_proc_run(&proc);

	// TODO wait for the child process.
	return 1;
}
