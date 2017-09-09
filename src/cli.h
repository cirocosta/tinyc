#ifndef TC__CLI_H
#define TC__CLI_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 *      Final configuration retrieved from the
 *      CLI arguments with the proper type casting
 *      already done.
 */
typedef struct cli_t {
	bool help;
	bool privileged;
	char* rootfs;
	char** argv;
	int argc;
	char** envp;
	int envc;
} tc_cli_t;

/**
 *      Intermediate representation of a generic
 *      flag that can be passed via CLI. It doesn't
 *      perform any type casting.
 *
 *      -       'name', 'name_len', 'description'
 *              and 'description_len' are compile-time
 *              constants.
 *
 *      -       'value' and ' value_len' are filled
 *              during runtime, being 'value' just
 *              a reference to a value in 'argv'.
 */
typedef struct cli_flag_t {
	char name[256];
	int name_len;
	char description[256];
	char* value;
	int value_len;
} tc_cli_flag_t;

static const tc_cli_flag_t TC_FLAG_HELP = {
	.name = "--help",
	.name_len = strlen("--help"),
	.description = "shows this help message",
};

static const tc_cli_flag_t TC_FLAG_PRIVILEGED = {
	.name = "--privileged",
	.name_len = strlen("--privileged"),
	.description = "disables capabilities and seccomp filtering",
};

static const tc_cli_flag_t TC_FLAG_ROOTFS = {
	.name = "--rootfs",
	.name_len = strlen("--rootfs"),
	.description = "absolute path to a rootfs",
};

static const tc_cli_flag_t TC_FLAG_ENV = {
	.name = "--env",
	.name_len = strlen("--env"),
	.description = "additional environment variable (list)",
};

/**
 *      Available flags to retrieve values from the 'cli'.
 */
static const tc_cli_flag_t* tc_cli_flags[] = { &TC_FLAG_HELP, &TC_FLAG_ROOTFS,
	                                       &TC_FLAG_ENV,
	                                       &TC_FLAG_PRIVILEGED };

/**
 *      Number of flags that 'tc_cli_flags' contain.
 */
static const int tc_cli_flags_len =
  sizeof(tc_cli_flags) / sizeof(tc_cli_flag_t*);

/**
 *      Top of the help message that is issued in
 *      case of either CLI parsing failures or ask
 *      for help'.
 */
static const char tc_cli_msg_help_header[] =
  "\n"
  "TINYC - 0.0.1\n"
  "Copyright 2017 - Ciro S. Costa <ciro.costa@liferay.com>\n"
  "\n"
  "Usage:  tinyc [opts] cmd\n";

static const char tc_cli_msg_help_footer[] =
  "Examples:\n"
  "\n"
  "    sudo tinyc --privileged --rootfs=/tmp/busybox /bin/sh\n"
  "\n";

/**
 *      Parses the command line arguments and
 *      modifies the provided 'tc_cli_t' struct based
 *      on the arguments specified. 'tc_cli_t*' can be
 *      passed already prefilled so that arguments can
 *      be uses as 'default values'.
 *
 *      In case of errors, non-zero is returned.
 *
 *      It expectes to reveice 'argc' and 'argv' right from
 *      the CLI, which means that it expects 'argv[0]' to
 *      container the 'tinyc' executable and then the rest
 *      be the actual set of flags and command to execute.
 */
int tc_cli_parse(tc_cli_t*, int argc, char** argv);

/**
 *      Prints to 'stderr' the help message.
 */
void tc_cli_help();

/**
 *      Cleans any resources allocated by 'tc_cli_parse'.
 */
void tc_cli_cleanup(tc_cli_t* cli);

#endif
