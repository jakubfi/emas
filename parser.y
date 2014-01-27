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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "st.h"
#include "prog.h"
#include "parser_utils.h"

int yylex(void);

%}

%code requires {

void yyerror(char *s, ...);

typedef struct YYLTYPE {
  int first_line;
  int first_column;
  int last_line;
  int last_column;
  char *filename;
} YYLTYPE;
# define YYLTYPE_IS_DECLARED 1

# define YYLLOC_DEFAULT(Current, Rhs, N) \
	if (N) { \
		(Current).first_line = YYRHSLOC (Rhs, 1).first_line; \
		(Current).first_column = YYRHSLOC (Rhs, 1).first_column; \
		(Current).last_line = YYRHSLOC (Rhs, N).last_line; \
		(Current).last_column = YYRHSLOC (Rhs, N).last_column; \
		(Current).filename = YYRHSLOC (Rhs, 1).filename; \
	} else { \
		(Current).first_line = (Current).last_line = YYRHSLOC (Rhs, 0).last_line; \
		(Current).first_column = (Current).last_column = YYRHSLOC (Rhs, 0).last_column; \
		(Current).filename = NULL; \
	} \
}

%error-verbose
%locations

%union {
	int v;
	char *s;
	double f;
	struct st *t;
};

%token END 0 "end of file"
%token INVALID_TOKEN "invalid token"
%token INVALID_FLAGS "invalid flags"
%token INVALID_STRING "invalid string"
%token INVALID_PRAGMA "invalid directive"
%token INVALID_INT "invalid integer"
%token INVALID_FLOAT "invalid float"
%token <s> STRING "string"
%token <v> INT "integer"
%token <f> FLOAT "floating point number"
%token <v> REG "register name"
%token <s> NAME "symbol"
%token <s> LABEL "label"

%token PROG NORM NONE BLOB

%token P_CPU ".cpu"
%token P_FILE ".file"
%token P_LINE ".line"
%token P_INCLUDE ".include"
%token P_EQU ".equ"
%token P_CONST ".const"
%token P_LBYTE P_RBYTE P_WORD P_DWORD P_FLOAT P_ASCII P_ASCIIZ P_RES
%token P_ORG ".org"
%token P_ENTRY ".entry"
%token P_GLOBAL ".global"

%token <v> OP_RN "(reg, norm) argument op"
%token <v> OP_N "(norm) argument op"
%token <v> OP_RT "(reg, T) argument op"
%token <v> OP_T "(T) argument op"
%token <v> OP_SHC "SHC"
%token <v> OP_R "(reg) argument op"
%token <v> OP__ "argumentless op"
%token <v> OP_X "argumentless extra op"
%token <v> OP_BLC "BLC"
%token <v> OP_BRC "BRC"
%token <v> OP_EXL "EXL"
%token <v> OP_NRF "NRF"
%token <v> OP_HLT "HLT"

%token '[' ']' ',' '(' ')'

%token CURLOC "."

%left '|'
%left '^'
%left '&'
%left RSHIFT "<<"
%left LSHIFT ">>"
%left '+' '-'
%left '*' '/' '%'
%left '\\'
%right '~'
%nonassoc UMINUS "unary minus"

%type <v> reg
%type <t> line lines label op pragma
%type <t> norm normval expr exprs name
%type <t> int float floats
%type <s> string

%destructor { st_drop($$); } <t>
%destructor { free($$); } <s>

%%

/* ---- PROGRAM ---------------------------------------------------------- */

program:
	lines { program = $1; }
	;

lines:
	/* empty */ { $$ = st_int(N_PROG, 0); }
	| lines line { $$ = st_arg_app($1, $2); }
	;

line:
	label
	| op
	| pragma
	| INVALID_TOKEN { $$ = NULL; YYABORT; }
	;

/* ---- LABEL ------------------------------------------------------------ */

label:
	LABEL { $$ = st_str(N_LABEL, $1); free($1); }
	;

/* ---- OP --------------------------------------------------------------- */

op:
	OP_RN reg ',' norm	{ $$ = compose_norm(N_OP_RN, $1, $2<<6, $4); }
	| OP_N norm			{ $$ = compose_norm(N_OP_R, $1, 0, $2); }
	| OP_RT reg ',' expr{ $$ = st_int(N_OP_RT, $1|($2<<6)); st_arg_app($$, $4); }
	| OP_T expr			{ $$ = st_int(N_OP_T, $1); st_arg_app($$, $2); }
	| OP_SHC reg ','expr{ $$ = st_int(N_OP_SHC, $1|($2<<6)); st_arg_app($$, $4); }
	| OP_R reg			{ $$ = st_int(N_OP_R, $1|($2<<6)); }
	| OP__				{ $$ = st_int(N_OP__, $1); }
	| OP_X				{ $$ = st_int(N_OP_X, $1); }
	| OP_BLC expr		{ $$ = st_int(N_OP_BLC, $1); st_arg_app($$, $2); }
	| OP_BRC expr		{ $$ = st_int(N_OP_BRC, $1); st_arg_app($$, $2); }
	| OP_EXL expr		{ $$ = st_int(N_OP_EXL, $1); st_arg_app($$, $2); }
	| OP_NRF			{ $$ = st_int(N_OP_NRF, $1); st_arg_app($$, st_int(N_INT, 255)); }
	| OP_NRF expr		{ $$ = st_int(N_OP_NRF, $1); st_arg_app($$, $2); }
	| OP_HLT			{ $$ = st_int(N_OP_HLT, $1); st_arg_app($$, st_int(N_INT, 0)); }
	| OP_HLT expr		{ $$ = st_int(N_OP_HLT, $1); st_arg_app($$, $2); }
	;

reg:
	REG
	| INVALID_TOKEN { YYABORT; }
	;

norm:
	normval
	| '[' normval ']' { $$ = $2; $$->val |= 1<<9; }
	;

normval:
	reg { $$ = st_int(N_NORM, $1); }
	| expr { $$ = st_int(N_NORM, 0); st_arg_app($$, $1); }
	| reg '+' reg { $$ = st_int(N_NORM, $1|($3<<3)); }
	| reg '+' expr { $$ = st_int(N_NORM, $1<<3); st_arg_app($$, $3); }
	| expr '+' reg { $$ = st_int(N_NORM, $3<<3); st_arg_app($$, $1); }
	| reg '-' expr { $$ = st_int(N_NORM, $1<<3); st_arg_app($$, st_arg(N_UMINUS, $3, NULL)); }
	;

/* ---- PRAGMA ----------------------------------------------------------- */

pragma:
	P_CPU NAME { $$ = NULL; if (!prog_cpu($2)) YYABORT; }
	| P_EQU name expr { $$ = $2; $$->type = N_EQU; st_arg_app($$, $3); }
	| P_CONST NAME expr { $$ = st_arg(N_CONST, $2, $3); }
	| P_LBYTE exprs { $$ = compose_list(N_LBYTE, $2); }
	| P_RBYTE exprs { $$ = compose_list(N_RBYTE, $2); }
	| P_WORD exprs { $$ = compose_list(N_WORD, $2); }
	| P_DWORD exprs { $$ = compose_list(N_DWORD, $2); }
	| P_FLOAT floats { $$ = compose_list(N_FLOAT, $2); }
	| P_ASCII string { $$ = st_str(N_ASCII, $2); }
	| P_ASCIIZ string { $$ = st_str(N_ASCIIZ, $2); }
	| P_RES expr { $$ = st_arg(N_RES, $2, NULL); }
	| P_RES expr ',' expr { $$ = st_arg(N_RES, $2, $4, NULL); }
	| P_ORG expr { $$ = st_arg(N_ORG, $2, NULL); }
	| P_ENTRY NAME { $$ = st_arg(N_ENTRY, $2, NULL); }
	| P_GLOBAL NAME { $$ = st_arg(N_GLOBAL, $2, NULL); }
	| INVALID_PRAGMA { $$ = NULL; YYABORT; }
	;

/* ---- EXPR ------------------------------------------------------------- */

expr:
	int
	| name
	| CURLOC { $$ = st_int(N_CURLOC, 0); }
	| '(' expr ')' { $$ = $2; }
	| expr '+' expr { $$ = st_arg(N_PLUS, $1, $3, NULL); }
	| expr '-' expr { $$ = st_arg(N_MINUS, $1, $3, NULL); }
	| expr '*' expr { $$ = st_arg(N_MUL, $1, $3, NULL); }
	| expr '/' expr { $$ = st_arg(N_DIV, $1, $3, NULL); }
	| expr '%' expr { $$ = st_arg(N_REM, $1, $3, NULL); }
	| '-' expr %prec UMINUS { $$ = st_arg(N_UMINUS, $2, NULL); }
	| expr '\\' expr { $$ = st_arg(N_SCALE, $1, $3, NULL); }
	| expr LSHIFT expr { $$ = st_arg(N_LSHIFT, $1, $3, NULL); }
	| expr RSHIFT expr { $$ = st_arg(N_RSHIFT, $1, $3, NULL); }
	| expr '&' expr { $$ = st_arg(N_AND, $1, $3, NULL); }
	| expr '|' expr { $$ = st_arg(N_OR, $1, $3, NULL); }
	| expr '^' expr { $$ = st_arg(N_XOR, $1, $3, NULL); }
	| '~' expr { $$ = st_arg(N_NEG, $2, NULL); }
	;

name:
	NAME { $$ = st_str(N_NAME, $1); free($1); }
	;

exprs:
	expr
	| expr ',' exprs { $$ = st_app($1, $3); }
	;

/* ---- STORAGE ---------------------------------------------------------- */

int:
	INT { $$ = st_int(N_INT, $1); }
	| INVALID_INT { $$ = NULL; YYABORT; }
	| INVALID_FLAGS { $$ = NULL; YYABORT; }
	;

string:
	STRING
	| INVALID_TOKEN { $$ = NULL; YYABORT; }
	;

float:
	FLOAT { $$ = st_float(N_FLO, $1); }
	| '-' FLOAT { $$ = st_float(N_FLO, -$2); }
	| INT { $$ = st_float(N_FLO, $1); }
	| '-'INT { $$ = st_float(N_FLO, -$2); }
	| INVALID_TOKEN { $$ = NULL; YYABORT; }
	;

floats:
	float
	| float ',' floats { $$ = st_app($1, $3); }
	;
%%


// -----------------------------------------------------------------------
void yyerror(char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	printf("%s:%d:%d: ", yylloc.filename, yylloc.first_line, yylloc.first_column);
	vprintf(s, ap);
	printf("\n");
	va_end(ap);
}

// vim: tabstop=4 autoindent
