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

%token END 0 "EOF"
%token INVALID_TOKEN "invalid token"
%token INVALID_FLAGS "invalid flags"
%token INVALID_STRING "invalid string"
%token INVALID_PRAGMA "invalid directive"
%token INVALID_INT "invalid integer"
%token INVALID_FLOAT "invalid float"
%token INVALID_REGISTER "invalid register"
%token <s> STRING "string"
%token <v> INT "integer"
%token <f> FLOAT "float"
%token <v> REG "register"
%token <s> NAME "symbol"
%token <s> LABEL "label"

%token PROG NORM NONE BLOB

%token P_CPU ".cpu"
%token P_FILE ".file"
%token P_LINE ".line"
%token P_INCLUDE ".include"
%token P_EQU ".equ"
%token P_CONST ".const"
%token P_LBYTE ".lbyte"
%token P_RBYTE ".rbyte"
%token P_WORD ".word"
%token P_DWORD ".dword"
%token P_FLOAT ".float"
%token P_ASCII ".ascii"
%token P_ASCIIZ ".asciiz"
%token P_RES ".res"
%token P_ORG ".org"
%token P_ENTRY ".entry"
%token P_GLOBAL ".global"

%token <v> OP_RN "reg-norm-arg op"
%token <v> OP_N "norm-arg op"
%token <v> OP_RT "reg-T-arg op"
%token <v> OP_T "T-arg op"
%token <v> OP_SHC "SHC"
%token <v> OP_R "reg-arg op"
%token <v> OP__ "no-arg op"
%token <v> OP_X "MX-16 op"
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

%type <t> line lines op pragma
%type <t> norm normval expr exprs
%type <t> float floats

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
	LABEL { $$ = st_str(N_LABEL, $1); free($1); }
	| op
	| pragma
	;

/* ---- OP --------------------------------------------------------------- */

op:
	OP_RN REG ',' norm	{ $$ = compose_norm(N_OP_RN, $1, $2<<6, $4); }
	| OP_N norm			{ $$ = compose_norm(N_OP_R, $1, 0, $2); }
	| OP_RT REG ',' expr{ $$ = st_int(N_OP_RT, $1|($2<<6)); st_arg_app($$, $4); }
	| OP_T expr			{ $$ = st_int(N_OP_T, $1); st_arg_app($$, $2); }
	| OP_SHC REG ','expr{ $$ = st_int(N_OP_SHC, $1|($2<<6)); st_arg_app($$, $4); }
	| OP_R REG			{ $$ = st_int(N_OP_R, $1|($2<<6)); }
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

norm:
	normval
	| '[' normval ']' { $$ = $2; $$->val |= 1<<9; }
	;

normval:
	REG { $$ = st_int(N_NORM, $1); }
	| expr { $$ = st_int(N_NORM, 0); st_arg_app($$, $1); }
	| REG '+' REG { $$ = st_int(N_NORM, $1|($3<<3)); }
	| REG '+' expr { $$ = st_int(N_NORM, $1<<3); st_arg_app($$, $3); }
	| expr '+' REG { $$ = st_int(N_NORM, $3<<3); st_arg_app($$, $1); }
	| REG '-' expr { $$ = st_int(N_NORM, $1<<3); st_arg_app($$, st_arg(N_UMINUS, $3, NULL)); }
	;

/* ---- PRAGMA ----------------------------------------------------------- */

pragma:
	P_CPU NAME {
		$$ = NULL;
		int res = prog_cpu($2, 0);
		if (res > 0) {
			yyerror("Unknown CPU type '%s'.", $2);
			YYABORT;
		} else if (res < 0) {
			yyerror("CPU type already set.");
			YYABORT;
		}
	}
	| P_EQU NAME expr { $$ = st_str(N_EQU, $2); st_arg_app($$, $3); }
	| P_CONST NAME expr { $$ = st_str(N_CONST, $2); st_arg_app($$, $3); }
	| P_LBYTE exprs { $$ = compose_list(N_LBYTE, $2); }
	| P_RBYTE exprs { $$ = compose_list(N_RBYTE, $2); }
	| P_WORD exprs { $$ = compose_list(N_WORD, $2); }
	| P_DWORD exprs { $$ = compose_list(N_DWORD, $2); }
	| P_FLOAT floats { $$ = compose_list(N_FLOAT, $2); }
	| P_ASCII STRING { $$ = st_str(N_ASCII, $2); }
	| P_ASCIIZ STRING { $$ = st_str(N_ASCIIZ, $2); }
	| P_RES expr { $$ = st_arg(N_RES, $2, NULL); }
	| P_RES expr ',' expr { $$ = st_arg(N_RES, $2, $4, NULL); }
	| P_ORG expr { $$ = st_arg(N_ORG, $2, NULL); }
	| P_ENTRY NAME { $$ = st_str(N_ENTRY, $2); }
	| P_GLOBAL NAME { $$ = st_str(N_GLOBAL, $2); }
	;

/* ---- EXPR ------------------------------------------------------------- */

expr:
	INT { $$ = st_int(N_INT, $1); }
	| NAME { $$ = st_str(N_NAME, $1); free($1); }
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

exprs:
	expr
	| expr ',' exprs { $$ = st_app($1, $3); }
	;

/* ---- STORAGE ---------------------------------------------------------- */

float:
	FLOAT { $$ = st_float(N_FLO, $1); }
	| '-' FLOAT { $$ = st_float(N_FLO, -$2); }
	;

floats:
	float
	| float ',' floats { $$ = st_app($1, $3); }
	;
%%


// vim: tabstop=4 autoindent
