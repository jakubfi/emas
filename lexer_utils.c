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

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <errno.h>

#include "parser.h"
#include "lexer_utils.h"
#include "st.h"

char str_buf[STR_MAX+1];
int str_len;
struct st *filenames;

// -----------------------------------------------------------------------
void llerror(char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	printf("%s: Error at line %d column %d: ", loc_stack[loc_pos].yyfilename, yylineno, loc_stack[loc_pos].yycolumn);
	vprintf(s, ap);
	printf("\n");
	va_end(ap);
}


// -----------------------------------------------------------------------
int esc2char(char c)
{
	switch (c) {
		case 'n': return '\n';
		case 't': return '\t';
		case 'r': return '\r';
		case 'f': return '\f';
		case '\\':return '\\';
		case '"': return '\"';
		default:  return -1;
	}
}

// -----------------------------------------------------------------------
int flag2mask(char c)
{
	switch (c) {
		case 'Z': return 1 << 15;
		case 'M': return 1 << 14;
		case 'V': return 1 << 13;
		case 'C': return 1 << 12;
		case 'L': return 1 << 11;
		case 'E': return 1 << 10;
		case 'G': return 1 << 9;
		case 'Y': return 1 << 8;
		case 'X': return 1 << 7;
		case '1': return 1 << 6;
		case '2': return 1 << 5;
		case '3': return 1 << 4;
		case '4': return 1 << 3;
		case '5': return 1 << 2;
		case '6': return 1 << 1;
		case '7': return 1 << 0;
		default: return -1;
	}
}

// -----------------------------------------------------------------------
int lex_int(char *str, int offset, int base, int *val)
{
	long v = strtol(str+offset, NULL, base);
	if (v < INT_MIN) {
		llerror("Integer underflow: %s", str);
		return INVALID_INT;
	}
	if (v > INT_MAX) {
		llerror("Integer overflow: %s", str);
		return INVALID_INT;
	}
	*val = v;
	return INT;
}

// -----------------------------------------------------------------------
int lex_float(char *str, double *val)
{
	*val = strtod(str, NULL);
	return FLOAT;
}

// -----------------------------------------------------------------------
int str_append(char c)
{
	if (str_len >= STR_MAX) {
		llerror("string too long");
		return 0;
	}
	*(str_buf+str_len++) = c;
	return 1;
}

// -----------------------------------------------------------------------
int loc_push(char *fname)
{
	if (loc_pos > INCLUDE_MAX) {
		return -1;
	}

	struct st *cfname = st_str(0, fname);
	filenames = st_app(filenames, cfname);

	loc_pos++;
	loc_stack[loc_pos].yycolumn = 1;
	loc_stack[loc_pos].yylineno = yylineno;
	loc_stack[loc_pos].o_yylineno = 1;
	loc_stack[loc_pos].yyfilename = cfname->str;
	yylineno = 1;

	return 0;
}

// -----------------------------------------------------------------------
int loc_pop()
{
	if (loc_pos >= 0) {
		yylineno = loc_stack[loc_pos].yylineno;
		loc_pos--;
		return 0;
	} else {
		return -1;
	}
}

// -----------------------------------------------------------------------
int loc_file(char *fname)
{
	struct st *cfname = st_str(0, fname);
	filenames = st_app(filenames, cfname);
	loc_stack[loc_pos].yyfilename = cfname->str;
	return 0;
}

// vim: tabstop=4 autoindent
