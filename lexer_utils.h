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

#ifndef LEXER_UTILS_H
#define LEXER_UTILS_H

extern int yylineno;

#define STR_MAX 1024
#define INCLUDE_MAX 32

#define YY_USER_ACTION \
	yylloc.filename = loc_stack[loc_pos].yyfilename; \
	yylloc.first_line = yylloc.last_line = yylineno; \
	yylloc.first_column = loc_stack[loc_pos].yycolumn; \
	if (yylineno != loc_stack[loc_pos].o_yylineno) loc_stack[loc_pos].yycolumn = 1; \
	else loc_stack[loc_pos].yycolumn += yyleng; \
	yylloc.last_column = loc_stack[loc_pos].yycolumn; \

struct loc {
	int yycolumn;
	int yylineno;
	int o_yylineno;
	char *yyfilename;
};

struct loc loc_stack[INCLUDE_MAX+1];
int loc_pos;
struct st *filenames;

extern char str_buf[STR_MAX+1];
extern int str_len;

void llerror(char *s, ...);
int esc2char(char c);
int flag2mask(char c);
int lex_int(char *str, int offset, int base, int *val);
int lex_float(char *str, double *val);
int str_append(char c);
int loc_push(char *fname);
int loc_pop();
int loc_file(char *fname);

#endif

// vim: tabstop=4 autoindent
