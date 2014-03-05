%{
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
#include <string.h>
#include <strings.h>

#include "parser.h"
#include "lexer_utils.h"
#include "keywords.h"

%}

%option nounput
%option noinput
%option noyywrap
%option yylineno
%option stack
%x str
%x p_include
%x p_line
%x p_file
%s comment
%s p_float

nl		(\n)|(\r\n)|(\f)|(\v)
ws		[ \t]+
cmt		;.*$

e_oct	\\0[0-3][0-7]{2,2}
e_hex	\\x[0-9a-fA-F]{2,2}
e_chr	\\.

bin		0b[01]+
oct		0[0-7]+
hex		0x[0-9a-fA-F]+
dec		[0-9]+
float	[0-9]+(\.[0-9]+)?([eE]-?[0-9]+)?
flags	\?[a-zA-Z0-9]+

reg		(?i:r)[0-9]+
pragma	\.[a-zA-Z]+
name	[a-zA-Z][a-zA-Z0-9_]*

%%

 /* ---- JUNK ------------------------------------------------------------ */

{nl}
<INITIAL,p_line,p_include,p_float>{ws}
<INITIAL,p_line,p_include,p_float>{cmt}
<INITIAL,p_line,p_include,p_float>"/*" { yy_push_state(comment); }
<comment>"*"
<comment>[^*]+
<comment>"*/" { yy_pop_state(); }

 /* ---- STRINGS --------------------------------------------------------- */

\" {
	yy_push_state(str);
	str_len = 0;
}

<str>\" {
	str_append('\0');
	yylval.s = strdup(str_buf);
	yy_pop_state();
	return STRING;
}

<str>{e_oct} {
	if (!str_append(strtol(yytext+2, NULL, 8))) {
		yy_pop_state();
		return INVALID_STRING;
	}
}

<str>{e_hex} {
	if (!str_append(strtol(yytext+2, NULL, 16))) {
		yy_pop_state();
		return INVALID_STRING;
	}
}

<str>{e_chr} {
	int c = esc2char(*(yytext+1));
	if (c < 0) {
		llerror("Invalid escape sequence: \"%s\"", yytext);
		yy_pop_state();
		return INVALID_STRING;
	}
	if (!str_append(c)) {
		yy_pop_state();
		return INVALID_STRING;
	}
}

<str>[^"][ \t]*$ {
	llerror("Unterminated string");
	yy_pop_state();
	return INVALID_STRING;
}

<str>. {
	if (!str_append(*yytext)) {
		yy_pop_state();
		return INVALID_STRING;
	}
}

 /* ---- CHARS ----------------------------------------------------------- */

'.' {
	yylval.v = yytext[1];
	return INT;
}

'..' {
	yylval.v = (yytext[1] << 8) + yytext[2];
	return INT;
}

'[a-zA-Z0-9\ _%#]{3,3}' {
	yylval.v = str2r40(yytext+1);
	return INT;
}

 /* ---- FLAGS ----------------------------------------------------------- */

{flags} {
	int v;
	char *c = yytext+1;
	yylval.v = 0;
	while (*c) {
		v = flag2mask(*c);
		if (v < 0) {
			llerror("Unknown flag: '%c'", *c);
			return INVALID_FLAGS;
		}
		if (yylval.v & v) {
			llerror("Duplicated flag: '%c'", *c);
			return INVALID_FLAGS;
		}
		yylval.v |= v;
		c++;
	}
	return INT;
}

 /* ---- NUMBERS --------------------------------------------------------- */

{oct}   {
	return lex_int(yytext, 1, 8, &(yylval.v));
}
<p_float>{dec} {
	return lex_float(yytext, &(yylval.f));
}
{dec}   {
	return lex_int(yytext, 0, 10, &(yylval.v));
}
{bin}   {
	return lex_int(yytext, 2, 2, &(yylval.v));
}
{hex}   {
	return lex_int(yytext, 2, 16, &(yylval.v));
}
{float} {
	return lex_float(yytext, &(yylval.f));
}

 /* ---- OPERATORS ------------------------------------------------------- */

[-+:,\[\]<>\*/%()|^&~] return *yytext;
"<<" return LSHIFT;
">>" return RSHIFT;
"\\" return '\\';
"." return CURLOC;

 /* ---- REGS ------------------------------------------------------- */

{reg} {
	yylval.v = strtol(yytext+1, NULL, 10);
	if ((yylval.v < 0) || (yylval.v > 7)) {
		return INVALID_REGISTER;
	} else {
		return REG;
	}
}

 /* ---- PRAGMAS --------------------------------------------------------- */

{pragma} {
	while (YY_START != INITIAL) yy_pop_state();
	struct dh_elem *p = pragma_get(yytext);
	if (!p) {
		llerror("Unknown assembler directive: \"%s\"", yytext);
		return INVALID_PRAGMA;
	}
	switch (p->type) {
		case P_INCLUDE:
			yy_push_state(p_include);
			break;
		case P_LINE:
			yy_push_state(p_line);
			break;
		case P_FILE:
			yy_push_state(p_file);
			break;
		case P_FLOAT:
			yy_push_state(p_float);
			return p->type;
		default:
			return p->type;
	}
}

<p_line>{nl} {
	yy_pop_state();
	llerror("Missing line number");
	return INVALID_PRAGMA;
}
<p_line>[^0-9\n\r\v\f \t;/*]+ {
	yy_pop_state();
	llerror("Not a line number: '%s'", yytext);
	return INVALID_PRAGMA;
}
<p_line>[0-9]+ {
	yy_pop_state();
	yylineno = strtol(yytext, NULL, 10) - 1;
}

<p_file>{nl} {
	yy_pop_state();
	llerror("Missing file name");
	return INVALID_PRAGMA;
}
<p_file>[a-zA-Z0-9_.-]+ {
	yy_pop_state();
	loc_file(yytext);
}

<p_include>{nl} {
	yy_pop_state();
	llerror("Missing include file name");
	return INVALID_PRAGMA;
}
<p_include>[a-zA-Z0-9_./-]+ {
	yy_pop_state();
	yyin = inc_open(yytext);
	if (!yyin) {
		llerror("Cannot find file: '%s' in any of include paths, or cannot open it", yytext);
		return INVALID_PRAGMA;
	}
	if (loc_push(yytext) < 0) {
		llerror("Cannot include file: '%s' (include too deep?)", yytext);
		fclose(yyin);
		return INVALID_PRAGMA;
	}
	yypush_buffer_state(yy_create_buffer(yyin, YY_BUF_SIZE));
}

 /* ---- LABEL ----------------------------------------------------------- */
{name}":" {
	while (YY_START != INITIAL) yy_pop_state();
	yylval.s = strdup(yytext);
	yylval.s[yyleng-1] = '\0';
	return LABEL;
}

 /* ---- NAMES and OPS --------------------------------------------------- */

{name} {
	struct dh_elem *p = mnemo_get(yytext);
	if (p) {
		while (YY_START != INITIAL) yy_pop_state();
		yylval.v = p->value;
		return p->type;
	} else {
		yylval.s = strdup(yytext);
		return NAME;
	}
}

 /* ---- EOF ------------------------------------------------------------- */

<<EOF>> {
	yy_top_state(); // just to suppress warning
	yypop_buffer_state();
	loc_pop();
	if (!YY_CURRENT_BUFFER) {
		yyterminate();
	}
}

 /* ---- ANYTHING ELSE --------------------------------------------------- */

. {
	llerror("Invalid character: '%c'", *yytext);
	return INVALID_TOKEN;
}

%%

// vim: tabstop=4 autoindent
