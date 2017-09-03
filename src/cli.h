#ifndef TC__CLI_H
#define TC__CLI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Final configuration retrieved from the
 * CLI arguments with the proper type casting
 * already done.
 */
typedef struct cli_t {
	char help;
	char rootfs[256];
	char** argv;
	int argc;
} tc_cli_t;

/**
 * Intermediate representation of a generic
 * flag that can be passed via CLI. It doesn't
 * perform any type casting.
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

/**
 * Available flags to retrieve values from the 'cli'.
 */
static const tc_cli_flag_t* tc_cli_flags[] = {
	&TC_FLAG_HELP,
};

/**
 * Number of flags that 'tc_cli_flags' contain.
 */
static const int tc_cli_flags_len =
  sizeof(tc_cli_flags) / sizeof(tc_cli_flag_t*);

static const char tc_cli_msg_help[] =
  "\n"
  "TINYC - 0.0.1\n"
  "Copyright 2017 - Ciro S. Costa <ciro.costa@liferay.com>\n"
  "\n"
  "Usage:  tinyc [opts] cmd\n"
  "        --memory MEM   sets maximum amount of memory\n"
  "        --help         shows this help message\n"
  "\n";

/**
 * Parses the command line arguments and
 * modifies the provided 'tc_cli_t' struct based
 * on the arguments specified. 'tc_cli_t*' can be
 * passed already prefilled so that arguments can
 * be uses as 'default values'.
 *
 * In case of errors, non-zero is returned.
 */
int tc_cli_parse(tc_cli_t*, int argc, char** argv);

/**
 * Prints to 'stderr' the help message.
 */
void tc_cli_help();

#endif
