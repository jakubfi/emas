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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "parser.h"
#include "lexer_utils.h"
#include "st.h"

int lexer_err_reported;
char str_buf[STR_MAX+1];
int str_len;
struct st *filenames;
struct st *inc_paths;
char *cur_label;
struct loc loc_stack[INCLUDE_MAX+1];
int loc_pos;

// -----------------------------------------------------------------------
void llerror(char *s, ...)
{
	lexer_err_reported = 1;
	va_list ap;
	va_start(ap, s);
	fprintf(stderr, "%s:%d:%d: ", loc_stack[loc_pos].filename, loc_stack[loc_pos].oline, loc_stack[loc_pos].ocol);
	vfprintf(stderr, s, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}


// -----------------------------------------------------------------------
int unesc_char(char *c, int *esclen)
{
	int ret;
	int el;
	char buf[4];

	if (*c != '\\') {
		el = 1;
		ret = *c;
	} else {
		switch (c[1]) {
			case 'a': // alert (bell)
				el = 2;
				ret = '\a';
				break;
			case 'b': // backspace
				el = 2;
				ret = '\b';
				break;
			case 'f': // form feed
				el = 2;
				ret = '\f';
				break;
			case 'n': // new line
				el = 2;
				ret = '\n';
				break;
			case 'r': // carriage return
				el = 2;
				ret = '\r';
				break;
			case 't': // tab
				el = 2;
				ret = '\t';
				break;
			case 'v': // vertical tab
				el = 2;
				ret = '\v';
				break;
			case '\\': // literal backslash
				el = 2;
				ret = '\\';
				break;
			case '"': // literal double quote
				el = 2;
				ret = '\"';
				break;
			case '\'': // literal single quote
				el = 2;
				ret = '\'';
				break;
			case 'x': // hex character value
				el = 4;
				strncpy(buf, c+2, 2);
				buf[2] = '\0';
				ret = strtol(buf, NULL, 16);
				break;
			case '0': // oct character value
				el = 5;
				strncpy(buf, c+2, 3);
				buf[3] = '\0';
				ret = strtol(buf, NULL, 8);
				break;
			default: // unknown escape sequence
				el = 0;
				ret = -1;
				break;
		}
	}

	if (esclen) {
		*esclen = el;
	}

	return ret;
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
void delchar(char *str, char c)
{
	char *r = str;
	char *w = str;

	while (*r) {
		*w = *r;
		if (*w != c) w++;
		r++;
	}
	*w = '\0';
}

// -----------------------------------------------------------------------
int lex_int(char *str, int offset, int base, int *val)
{
	delchar(str+offset, '_');
	*val = strtol(str+offset, NULL, base);
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
	str_buf[str_len++] = c;
	return 1;
}

// -----------------------------------------------------------------------
void loc_update(int len)
{
	loc_stack[loc_pos].oline = loc_stack[loc_pos].line;
	loc_stack[loc_pos].line = yylineno;
	loc_stack[loc_pos].ocol = loc_stack[loc_pos].col;
	if (loc_stack[loc_pos].oline != loc_stack[loc_pos].line) {
		loc_stack[loc_pos].col = 1;
	} else {
		loc_stack[loc_pos].col += len;
	}

	yylloc.filename = loc_stack[loc_pos].filename;
	yylloc.first_line = loc_stack[loc_pos].oline;
	yylloc.last_line = loc_stack[loc_pos].line;
	yylloc.first_column = loc_stack[loc_pos].ocol;
	yylloc.last_column = loc_stack[loc_pos].col;
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

	loc_stack[loc_pos].filename = cfname->str;
	loc_stack[loc_pos].col = 1;
	loc_stack[loc_pos].line = 1;
	loc_stack[loc_pos].ocol = 1;
	loc_stack[loc_pos].oline = 1;

	loc_stack[loc_pos].yylineno = yylineno;
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
	loc_stack[loc_pos].filename = cfname->str;
	return 0;
}

// -----------------------------------------------------------------------
int inc_path_add(char *path)
{
	struct st *incpath = st_str(0, path);
	inc_paths = st_app(inc_paths, incpath);
	return 0;
}

// -----------------------------------------------------------------------
FILE * inc_open(char *filename)
{
	struct st *path = inc_paths;
	char pbuf[STR_MAX+1];

	while (path) {
		int i = snprintf(pbuf, STR_MAX, "%s/%s", path->str, filename);
		if (i > 0) {
			FILE *f = fopen(pbuf, "r");
			if (f) {
				return f;
			}
		}
		path = path->next;
	}

	return NULL;
}

// vim: tabstop=4 autoindent
