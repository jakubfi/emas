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

e_oct	\\0[0-7]{3,3}
e_hex	\\x[0-9a-fA-F]{2,2}
e_chr	\\.

bin		0b_*[01][01_]*
oct		0_*[0-7][0-7_]*
hex		0x_*[0-9a-fA-F][0-9a-fA-F_]*
dec		_*[0-9][0-9_]*
float	[0-9]+(\.[0-9]+)?([eE]-?[0-9]+)?
flags	\?[a-zA-Z0-9]+

reg		(?i:r)[0-9]+
pragma	\.[a-zA-Z0-9]+
name	[a-zA-Z_][a-zA-Z0-9_]*

achar	[^\a\b\f\n\r\t\v\\\'\"]

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
	yylval.s = malloc(str_len);
	yylval.s = memcpy(yylval.s, str_buf, str_len);
	yy_pop_state();
	return STRING;
}

<str>{e_chr}|{e_hex}|{e_oct} {
	int c = unesc_char(yytext, NULL);
	if (c < 0) {
		llerror("Invalid escape sequence: \"%s\"", yytext);
		yy_pop_state();
		return INVALID_STRING;
	} else if (c > 255) {
		llerror("Invalid escape sequence (value too big): \"%s\"", yytext);
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

'({achar}|{e_chr}|{e_hex}|{e_oct})' {
	int c = unesc_char(yytext+1, NULL);
	if (c < 0) {
		llerror("Invalid escape sequence: \"%s\"", yytext);
		return INVALID_STRING;
	} else if (c > 255) {
		llerror("Invalid escape sequence (value too big): \"%s\"", yytext);
		return INVALID_STRING;
	} else {
		yylval.v = c;
	}
	return INT;
}

'({achar}|{e_chr}|{e_hex}|{e_oct}){2}' {
	int c;
	int esclen = 0;
	yylval.v = 0;
	for (int mul=256 ; mul>0 ; mul-=255) {
		c = unesc_char(yytext+1+esclen, &esclen);
		if (c < 0) {
			llerror("Invalid escape sequence: \"%s\"", yytext);
			return INVALID_STRING;
		} else if (c > 255) {
			llerror("Invalid escape sequence (value too big): \"%s\"", yytext);
			return INVALID_STRING;
		} else {
			yylval.v += c * mul;
		}
	}
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

{oct} {
	return lex_int(yytext, 1, 8, &(yylval.v));
}
<p_float>{dec} {
	return lex_float(yytext, &(yylval.f));
}
{dec} {
	return lex_int(yytext, 0, 10, &(yylval.v));
}
{bin} {
	return lex_int(yytext, 2, 2, &(yylval.v));
}
{hex} {
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
		if (!cur_label) {
			llerror("Cannot use local label \"%s\" outside a global label context" , yytext);
			return INVALID_LABEL;
		}
		yylval.s = malloc(strlen(cur_label)+strlen(yytext)+1);
		sprintf(yylval.s, "%s%s", cur_label, yytext);
		return NAME;
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

 /* ---- LABELS ---------------------------------------------------------- */
{name}":" {
	while (YY_START != INITIAL) yy_pop_state();
	yylval.s = strdup(yytext);
	yylval.s[yyleng-1] = '\0';
	free(cur_label);
	cur_label = strdup(yylval.s);
	return LABEL;
}
"."{name}":" {
	yytext[yyleng-1] = '\0';
	struct dh_elem *p = pragma_get(yytext);
	if (p) {
		llerror("Cannot use assembler directive \"%s\" as a label" , yytext);
		return INVALID_LABEL;
	}
	if (!cur_label) {
		llerror("Cannot define local label \"%s\" outside a global label context" , yytext);
		return INVALID_LABEL;
	}
	while (YY_START != INITIAL) yy_pop_state();
	yylval.s = malloc(strlen(cur_label)+strlen(yytext)+1);
	sprintf(yylval.s, "%s%s", cur_label, yytext);
	return LABEL;
}


 /* ---- NAMES and OPS --------------------------------------------------- */
"."{name} {
	if (!cur_label) {
		llerror("Cannot use local label \"%s\" outside a global label context" , yytext);
		return INVALID_LABEL;
	}
	yylval.s = malloc(strlen(cur_label)+strlen(yytext)+1);
	sprintf(yylval.s, "%s%s", cur_label, yytext);
	return NAME;
}

{name}|{name}"."{name} {
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
	if (yy_start_stack_ptr) yy_top_state(); // just to suppress warning
	yypop_buffer_state();
	loc_pop();
	free(cur_label);
	cur_label = NULL;
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
