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
#include <assert.h>
#include <strings.h>
#include <stdarg.h>

#include "prog.h"
#include "parser.h"

extern int lexer_err_reported;

// -----------------------------------------------------------------------
void yyerror(char *s, ...)
{
	if (lexer_err_reported) return;
	va_list ap;
	va_start(ap, s);
	printf("%s:%d:%d: ", yylloc.filename, yylloc.first_line, yylloc.first_column);
	vprintf(s, ap);
	printf("\n");
	va_end(ap);
}

// -----------------------------------------------------------------------
struct st * compose_norm(int type, int opcode, int reg, struct st *norm)
{
	struct st *op = st_int(type, opcode | reg | norm->val);
	struct st *data = NULL;
	if (norm->args) {
		data = st_arg(N_WORD, norm->args, NULL);
		norm->args = NULL;
	}
	st_drop(norm);
	return st_app(op, data);
}

// -----------------------------------------------------------------------
struct st * compose_list(int type, struct st *list)
{
	struct st *out = NULL;
	struct st *l = list;
	struct st *next;

	while (l) {
		next = l->next;
		out = st_app(out, st_arg(type, l, NULL));
		l->next = NULL;
		l = next;
	}

	return out;
}

// vim: tabstop=4 autoindent
