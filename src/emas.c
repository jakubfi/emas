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
	O_KEYS	= 4,
};

int yyparse();
int yylex_destroy();
extern FILE *yyin;

char *input_file;
char *output_file;
char *basename;
int otype = O_RAW;

// -----------------------------------------------------------------------
void usage()
{
	fprintf(stderr, "Usage: emas [options] [input]\n");
	fprintf(stderr, "Where options are one or more of:\n");
	fprintf(stderr, "   -o <output>    : set output file\n");
	fprintf(stderr, "   -c <cpu>       : set CPU type: mera400, mx16\n");
	fprintf(stderr, "   -O <otype>     : set output type: raw, debug, keys (defaults to raw)\n");
	fprintf(stderr, "   -I <dir>       : search for include files in <dir>\n");
	fprintf(stderr, "   -D <const>[=v] : define a constant and optionaly set its value (0 by default)\n");
	fprintf(stderr, "   -d             : print debug information to stderr (lots of)\n");
	fprintf(stderr, "   -v             : print version and exit\n");
	fprintf(stderr, "   -h             : print help and exit\n");
}

// -----------------------------------------------------------------------
int parse_args(int argc, char **argv)
{
	char *strval;
	int val = 0;

	int option;
	while ((option = getopt(argc, argv,"I:D:c:O:vhdo:")) != -1) {
		switch (option) {
			case 'c':
				if (prog_cpu(optarg, CPU_FORCED)) {
					fprintf(stderr, "Unknown cpu: '%s'.\n", optarg);
					return -1;
				}
				break;
			case 'I':
				inc_path_add(optarg);
				break;
			case 'D':
				strval = strchr(optarg, '=');
				if (strval) {
					*strval = '\0';
					val = atoi(strval+1);
				}
				add_const(optarg, val);
				break;
			case 'O':
				if (!strcmp(optarg, "raw")) {
					otype = O_RAW;
				} else if (!strcmp(optarg, "debug")) {
					otype = O_DEBUG;
				} else if (!strcmp(optarg, "keys")) {
					otype = O_KEYS;
				} else {
					fprintf(stderr, "Unknown output type: '%s'.\n", optarg);
					return -1;
				}
				break;
			case 'h':
				usage();
				exit(0);
				break;
			case 'v':
				fprintf(stderr, "EMAS v%s - modern assembler for MERA-400 minicomputer system\n", EMAS_VERSION);
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
		fprintf(stderr, "Wrong usage.\n");
		return -1;
	}

	return 0;
}

// -----------------------------------------------------------------------
int main(int argc, char **argv)
{
	int ret = 1;
	int res;
	FILE *outf = NULL;

	if (kw_init() < 0) {
		fprintf(stderr, "Internal dictionary initialization failed.\n");
		goto cleanup;
	}

	sym = dh_create(16000, 1);
	if (!sym) {
		fprintf(stderr, "Failed to create symbol table.\n");
		goto cleanup;
	}

	res = parse_args(argc, argv);

	if (res) {
		fprintf(stderr, "\n");
		usage();
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
		fprintf(stderr, "Cannot open source file: '%s'\n", input_file);
		goto cleanup;
	}

	inc_path_add(".");
	inc_path_add(EMAS_ASM_INCLUDES);
	inc_path_add("/usr/share/emas/include");
	inc_path_add("/usr/local/share/emas/include");

	AADEBUG("==== Include search dirs ==================");
	struct st *i = inc_paths;
	while (i) {
		AADEBUG("%s", i->str);
		i = i->next;
	}

	AADEBUG("==== Parse ================================");
	if (yyparse()) {
		if (yyin) fclose(yyin);
		goto cleanup;
	}

	if (yyin) fclose(yyin);

	if (!program) { // shouldn't happen - parser should always produce a program (even an empty one)
		fprintf(stderr, "Parse produced empty tree.\n");
		goto cleanup;
	}

	res = assemble(program, 1);

	if (res < 0) {
		fprintf(stderr, "%s\n", aerr);
		goto cleanup;
	} else if (res > 0) {
		if (assemble(program, 0)) {
			fprintf(stderr, "%s\n", aerr);
			goto cleanup;
		}
	}

	// set the output file name if no given
	if (!output_file) {
		if ((otype == O_DEBUG) || (otype == O_KEYS)) {
			output_file = strdup("(stdout)");
			outf = stdout;
		} else {
			if (!input_file) {
				output_file = strdup("a.out");
			} else {
				basename = strdup(input_file);
				char *of_dot = strrchr(basename, '.');
				if (of_dot) {
					*of_dot = '\0';
				}

				if (otype == O_RAW) {
					output_file = strdup(basename);
				} else {
					fprintf(stderr, "Unknown output file type.");
					goto cleanup;
				}

				if (!strcmp(input_file, output_file)) {
					fprintf(stderr, "Input and output file names cannot be the same: '%s'\n", output_file);
					goto cleanup;
				}
			}

		}
	}

	if (outf != stdout) {
		if ((otype == O_RAW) && !strcmp(output_file, "-")) {
			outf = stdout;
		} else {
			outf = fopen(output_file, "wb");
			if (!outf) {
				fprintf(stderr, "Cannot open output file '%s' for writing\n", output_file);
				goto cleanup;
			}
		}
	}

	switch (otype) {
		case O_RAW:
			res = writer_raw(program, outf);
			break;
		case O_DEBUG:
			res = writer_debug(program, outf);
			break;
		case O_KEYS:
			res = writer_keys(program, outf);
			break;
		default:
			fprintf(stderr, "Unknown output type.\n");
			fclose(outf);
			goto cleanup;
	}

	fclose(outf);

	if (res) {
		fprintf(stderr, "%s\n", aerr);
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
	free(basename);

	return ret;
}

// vim: tabstop=4 autoindent
