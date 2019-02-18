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

#include <stdio.h>

extern int yylineno;

#define STR_MAX 1024
#define INCLUDE_MAX 32

#define YY_USER_ACTION loc_update(yyleng);

struct loc {
	char *filename;
	int line, col;
	int oline, ocol;
	int yylineno;
};

struct loc loc_stack[INCLUDE_MAX+1];
int loc_pos;
struct st *filenames;
struct st *inc_paths;
extern char *cur_label;

extern char str_buf[STR_MAX+1];
extern int str_len;

void llerror(char *s, ...);
int unesc_char(char *c, int *esclen);
int flag2mask(char c);
int str2r40(char *str);
int lex_int(char *str, int offset, int base, int *val);
int lex_float(char *str, double *val);
int str_append(char c);
void loc_update(int len);
int loc_push(char *fname);
int loc_pop();
int loc_file(char *fname);
int inc_path_add(char *path);
FILE * inc_open(char *filename);

#endif

// vim: tabstop=4 autoindent
