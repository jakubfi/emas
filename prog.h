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

extern struct dh_table *sym;
extern struct st *program;
extern int cpu;
extern char aerr[MAX_ERRLEN+1];

enum cpu_types {
	CPU_DEFAULT = 0,
	CPU_MERA400 = 1,
	CPU_MX16 = 2,
};

enum sym_types {
	SYM_UNDEFINED	= 0b00000001,
	SYM_RELATIVE	= 0b00000010,
	SYM_CONST		= 0b00000100,
	SYM_LOCAL		= 0b00001000,
};

int prog_cpu(char *cpu_name);
struct st * compose_norm(int type, int opcode, int reg, struct st *norm);
struct st * compose_list(int type, struct st *list);
int eval(struct st *t);
int assemble(struct st *prog, int pass);

#endif

// vim: tabstop=4 autoindent
