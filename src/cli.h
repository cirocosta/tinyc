#ifndef TC__CLI_H
#define TC__CLI_H

#include <stdio.h>
#include <stdlib.h>

typedef struct cli_t {
	char help;
	char rootfs[256];
	char** argv;
	int argc;
} tc_cli_t;

static const char tc_cli_msg_help[] =
  "\n"
  "TINYC - 0.0.1\n"
  "Copyright 2017 - Ciro S. Costa <ciro.costa@liferay.com>\n"
  "\n"
  "Usage:  tinyc [opts] cmd\n"
  "        --memory MEM   sets maximum amount of memory\n"
  "        --help         shows this help message\n"
  "\n";

tc_cli_t* tc_cli_parse(int argc, char** argv);
void tc_cli_help();

#endif
