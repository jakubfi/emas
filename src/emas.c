//  Copyright (c) 2014 Jakub Filipowicz <jakubf@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc.,
//  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "keywords.h"
#include "parser.h"
#include "prog.h"
#include "lexer_utils.h"
#include "writers.h"

enum output_types {
	O_DEBUG	= 1,
	O_RAW	= 2,
	O_EMELF	= 3,
	O_KEYS	= 4,
};

int yyparse();
int yylex_destroy();
extern FILE *yyin;

char *input_file;
char *output_file = NULL;
int otype = O_RAW;

// -----------------------------------------------------------------------
void usage()
{
	printf("Usage: emas [options] [input]\n");
	printf("Where options are one or more of:\n");
	printf("   -o <output> : set output file (a.out otherwise)\n");
	printf("   -c <cpu>    : set CPU type: mera400, mx16\n");
	printf("   -O <otype>  : set output type: raw, debug, emelf, keys (defaults to raw)\n");
	printf("   -I <dir>    : search for include files in <dir>\n");
	printf("   -d          : print debug information to stderr (lots of)\n");
	printf("   -v          : print version and exit\n");
	printf("   -h          : print help and exit\n");
}

// -----------------------------------------------------------------------
int parse_args(int argc, char **argv)
{
	int option;
	while ((option = getopt(argc, argv,"I:c:O:vhdo:")) != -1) {
		switch (option) {
			case 'c':
				if (prog_cpu(optarg, CPU_FORCED)) {
					printf("Unknown cpu: '%s'.\n", optarg);
					return -1;
				}
				break;
			case 'I':
				inc_path_add(optarg);
				break;
			case 'O':
				if (!strcmp(optarg, "raw")) {
					otype = O_RAW;
				} else if (!strcmp(optarg, "debug")) {
					otype = O_DEBUG;
				} else if (!strcmp(optarg, "emelf")) {
					otype = O_EMELF;
				} else if (!strcmp(optarg, "keys")) {
					otype = O_KEYS;
				} else {
					printf("Unknown output type: '%s'.\n", optarg);
					return -1;
				}
				break;
			case 'h':
				usage();
				exit(0);
				break;
			case 'v':
				printf("EMAS v%s - modern MERA 400 assembler\n", EMAS_VERSION);
				exit(0);
				break;
			case 'd':
				aadebug = 1;
				break;
			case 'o':
				output_file = strdup(optarg);
				break;
			default:
				return -1;
		}
	}

	if (optind == argc) {
		input_file = NULL;
	} else if (optind == argc-1) {
		input_file = argv[optind];
	} else {
		printf("Wrong usage.\n");
		return -1;
	}

	return 0;
}

// -----------------------------------------------------------------------
int main(int argc, char **argv)
{
	int ret = 1;
	int res;
	FILE *outf;

	res = parse_args(argc, argv);

	if (res) {
		printf("\n");
		usage();
		goto cleanup;
	}

	if (kw_init() < 0) {
		printf("Internal dictionary initialization failed.\n");
		goto cleanup;
	}

	sym = dh_create(16000, 1);
	if (!sym) {
		printf("Failed to create symbol table.\n");
		goto cleanup;
	}

	if (input_file) {
		yyin = fopen(input_file, "r");
		loc_push(input_file);
	} else {
		yyin = stdin;
		loc_push("(stdin)");
	}

	if (!yyin) {
		printf("Cannot open source file: '%s'\n", input_file);
		goto cleanup;
	}

	inc_path_add(".");
	inc_path_add("/usr/share/emas/include");

	AADEBUG("==== Parse ================================");
	if (yyparse()) {
		if (yyin) fclose(yyin);
		goto cleanup;
	}

	if (yyin) fclose(yyin);

	if (!program) { // shouldn't happen - parser should always produce a program (even an empty one)
		printf("Parse produced empty tree.\n");
		goto cleanup;
	}

	res = assemble(program, 1);

	if (res < 0) {
		printf("%s\n", aerr);
		goto cleanup;
	} else if (res > 0) {
		if (otype == O_EMELF) {
			if (assemble(program, 1) < 0) {
				printf("%s\n", aerr);
				goto cleanup;
			}
		} else {
			if (assemble(program, 0)) {
				printf("%s\n", aerr);
				goto cleanup;
			}
		}
	}

	if ((otype != O_DEBUG) && (otype != O_KEYS)) {
		if (!output_file) {
			output_file = strdup("a.out");
		}
		outf = fopen(output_file, "w");
	} else {
		if (!output_file) {
			output_file = strdup("(stdout)");
			outf = stdout;
		} else {
			outf = fopen(output_file, "w");
		}
	}

	if (!outf) {
		printf("Cannot open output file '%s' for writing\n", output_file);
		goto cleanup;
	}

	switch (otype) {
		case O_RAW:
			res = writer_raw(program, outf);
			break;
		case O_DEBUG:
			res = writer_debug(program, outf);
			break;
		case O_EMELF:
			res = writer_emelf(program, sym, outf);
			break;
		case O_KEYS:
			res = writer_keys(program, outf);
			break;
		default:
			printf("Unknown output type.\n");
			fclose(outf);
			goto cleanup;
	}

	fclose(outf);

	if (res) {
		printf("%s\n", aerr);
		goto cleanup;
	}
	ret = 0;

cleanup:
	yylex_destroy();
	st_drop(inc_paths);
	st_drop(filenames);
	st_drop(program);
	dh_destroy(sym);
	st_drop(entry);
	kw_destroy();
	free(output_file);

	return ret;
}

// vim: tabstop=4 autoindent
