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
#include "keywords.h"
#include "parser.h"
#include "prog.h"
#include "lexer_utils.h"
#include "writers.h"

int yyparse();
int yylex_destroy();

// -----------------------------------------------------------------------
int main(int argc, char **argv)
{
	int ret = 0;
	int res;

	if (kw_init() < 0) {
		printf("Fatal: Internal dictionary initialization failed.\n");
		ret = 1;
		goto cleanup;
	}

	sym = dh_create(16000, 1);
	if (!sym) {
		printf("Fatal: failed to create symbol table.\n");
		ret = 1;
		goto cleanup;
	}

	loc_push("(stdin)");

	if (yyparse()) {
		printf("Parse failed.\n"); // parser should print appropriate error message
		ret = 1;
		goto cleanup;
	}

	if (!program) { // shouldn't happen - parser should always produce program (even empty one)
		printf("Fatal: Parse produced empty tree.\n");
		ret = 1;
		goto cleanup;
	}

	res = assemble(program);

	if (res < 0) {
		printf("Fatal: Assemble failed.\n"); // assemble should print appropriate message
		ret = 1;
		goto cleanup;
	}

	if (res > 0) {
		printf("Second pass (%i unresolved)\n", res);
		res = assemble(program);
		if (res) {
			printf("Fatal: Second assemble pass failed (%i unresolved).\n", res); // assemble should print appropriate message
			ret = 1;
			goto cleanup;
		}
	}

	writer_raw(program);

cleanup:
	yylex_destroy();
	st_drop(filenames);
	st_drop(program);
	dh_destroy(sym);
	kw_destroy();

	return ret;
}

// vim: tabstop=4 autoindent
