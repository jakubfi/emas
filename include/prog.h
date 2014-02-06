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

#ifndef PROG_H
#define PROG_H

#include <inttypes.h>

#include "dh.h"
#include "st.h"

#define MAX_ERRLEN 1024

typedef int (*eval_fun)(struct st *t);

extern struct dh_table *sym;
extern struct st *program;
extern int cpu;
extern char aerr[MAX_ERRLEN+1];
extern int ic_max;

enum cpu_types {
	CPU_DEFAULT			= 0,
	CPU_FORCED			= 1 << 0,
	CPU_MERA400			= 1 << 1,
	CPU_MX16			= 1 << 2,
};

enum sym_types {
	SYM_UNDEFINED	= 0b00000001,
	SYM_RELATIVE	= 0b00000010,
	SYM_CONST		= 0b00000100,
	SYM_GLOBAL		= 0b00001000,
	SYM_ENTRY		= 0b00010000,
};

enum node_types {
	N_NONE = 0,
	N_INT,
	N_BLOB,
	N_FLO,
	N_PLUS,
	N_MINUS,
	N_MUL,
	N_DIV,
	N_REM,
	N_AND,
	N_XOR,
	N_OR,
	N_LSHIFT,
	N_RSHIFT,
	N_SCALE,
	N_UMINUS,
	N_NEG,
	N_WORD,
	N_LBYTE,
	N_RBYTE,
	N_DWORD,
	N_FLOAT,
	N_RES,
	N_ORG,
	N_ASCII,
	N_ASCIIZ,
	N_ENTRY,
	N_GLOBAL,
	N_LABEL,
	N_EQU,
	N_CONST,
	N_NAME,
	N_CURLOC,
	N_OP_X,
	N_OP_RN,
	N_OP_N,
	N_OP_R,
	N_OP__,
	N_OP_RT,
	N_OP_T,
	N_OP_SHC,
	N_OP_BLC,
	N_OP_BRC,
	N_OP_EXL,
	N_OP_NRF,
	N_OP_HLT,
	N_PROG,
	N_NORM,
	N_MAX,
};

struct eval_t {
	char *name;
	eval_fun fun;
};

int prog_cpu(char *cpu_name, int force);

int eval_1arg(struct st *t);
int eval_2arg(struct st *t);
int eval_float(struct st *t);
int eval_word(struct st *t);
int eval_multiword(struct st *t);
int eval_res(struct st *t);
int eval_org(struct st *t);
int eval_string(struct st *t);
int eval_label(struct st *t);
int eval_equ(struct st *t);
int eval_const(struct st *t);
int eval_global(struct st *t);
int eval_name(struct st *t);
int eval_curloc(struct st *t);
int eval_as_short(struct st *t, int type, int op);
int eval_op_short(struct st *t);
int eval_op_mx16(struct st *t);
int eval_op_noarg(struct st *t);
int eval_none(struct st *t);
int eval_err(struct st *t);

int eval(struct st *t);
int assemble(struct st *prog, int pass);

#endif

// vim: tabstop=4 autoindent
