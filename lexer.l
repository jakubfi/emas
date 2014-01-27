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
#include "prog.h"

%}

%option nounput
%option noinput
%option noyywrap
%option yylineno
%x str
%x p_include
%x p_line
%s comment

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
flags	\?[ZMVCLEGYX1234567]{1,16}

reg		(?i:r)[0-7]
pragma	\.[a-zA-Z]+
name	[a-zA-Z][a-zA-Z0-9_]*

%%

 /* ---- JUNK ------------------------------------------------------------ */

{nl}
<INITIAL,p_line,p_include>{ws}
<INITIAL,p_line,p_include>{cmt}
<INITIAL,p_line,p_include>"/*" { BEGIN(comment); }
<comment>"*"
<comment>[^*]+
<comment>"*/" { BEGIN(INITIAL); }

 /* ---- STRINGS --------------------------------------------------------- */

\" {
	BEGIN(str);
	str_len = 0;
}

<str>\" {
	str_append('\0');
	yylval.s = strdup(str_buf);
	BEGIN(INITIAL);
	return STRING;
}

<str>{e_oct} {
	if (!str_append(strtol(yytext+2, NULL, 8))) {
		BEGIN(INITIAL);
		return INVALID_STRING;
	}
}

<str>{e_hex} {
	if (!str_append(strtol(yytext+2, NULL, 16))) {
		BEGIN(INITIAL);
		return INVALID_STRING;
	}
}

<str>{e_chr} {
	int c = esc2char(*(yytext+1));
	if (c < 0) {
		llerror("Invalid escape sequence: \"%s\"", yytext);
		BEGIN(INITIAL);
		return INVALID_STRING;
	}
	if (!str_append(c)) {
		BEGIN(INITIAL);
		return INVALID_STRING;
	}
}

<str>[^"][ \t]*$ {
	//llerror("Invalid character in string: #%d", *yytext);
	llerror("Unterminated string");
	BEGIN(INITIAL);
	return INVALID_STRING;
}

<str>. {
	if (!str_append(*yytext)) {
		BEGIN(INITIAL);
		return INVALID_STRING;
	}
}

 /* ---- CHARS ----------------------------------------------------------- */

'.' {
	yylval.v = *(yytext+1);
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
	return REG;
}

 /* ---- PRAGMAS --------------------------------------------------------- */

{pragma} {
	struct dh_elem *p = pragma_get(yytext);
	if (!p) {
		llerror("Unknown assembler directive: \"%s\"", yytext);
		return INVALID_PRAGMA;
	}
	switch (p->type) {
		case P_INCLUDE:
			BEGIN(p_include);
			break;
		case P_LINE:
			BEGIN(p_line);
			break;
		default:
			return p->type;
	}
}

<p_line>{nl} {
	BEGIN(INITIAL);
	llerror("Missing line number");
	return INVALID_PRAGMA;
}
<p_line>[^0-9\n\r\v\f \t;/*]+ {
	BEGIN(INITIAL);
	llerror("Not a line number: '%s'", yytext);
	return INVALID_PRAGMA;
}
<p_line>[0-9]+ {
	BEGIN(INITIAL);
	yylineno = strtol(yytext, NULL, 10) - 1;
}

<p_include>{nl} {
	BEGIN(INITIAL);
	llerror("Missing include file name");
	return INVALID_PRAGMA;
}
<p_include>[a-zA-Z0-9_.-]+ {
	BEGIN(INITIAL);
	yyin = fopen(yytext, "r");
	if (!yyin) {
		llerror("Cannot open include file: '%s'", yytext);
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
	yylval.s = strdup(yytext);
	yylval.s[yyleng-1] = '\0';
	return LABEL;
}

 /* ---- NAMES and OPS --------------------------------------------------- */

{name} {
	struct dh_elem *p = mnemo_get(yytext);
	if (p) {
		yylval.v = p->value;
		return p->type;
	} else {
		yylval.s = strdup(yytext);
		return NAME;
	}
}

 /* ---- EOF ------------------------------------------------------------- */

<<EOF>> {
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
