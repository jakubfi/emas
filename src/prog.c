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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <inttypes.h>

#include <emawp.h>

#include "dh.h"
#include "st.h"
#include "prog.h"
#include "lexer_utils.h"

struct dh_table *sym;
struct st *program;
struct st *entry;

char aerr[MAX_ERRLEN+1];
int aadebug;

int ic;
int cpu = CPU_DEFAULT;
int ic_max = 32767;

// table indexed by node type, order here must match enum node_types
struct eval_t eval_tab[] = {
	{ "NONE",	eval_none },
	{ "INT",	eval_none },
	{ "BLOB",	eval_none },
	{ "FLOAT",	eval_float },
	{ "+",		eval_2arg },
	{ "-",		eval_2arg },
	{ "*",		eval_2arg },
	{ "/",		eval_2arg },
	{ "%",		eval_2arg },
	{ "&",		eval_2arg },
	{ "^",		eval_2arg },
	{ "|",		eval_2arg },
	{ "<<",		eval_2arg },
	{ ">>",		eval_2arg },
	{ "\\",		eval_2arg },
	{ "- (unary)",eval_1arg },
	{ "~",		eval_1arg },
	{ ".word",	eval_word },
	{ ".dword",	eval_multiword },
	{ ".float",	eval_multiword },
	{ ".res",	eval_res },
	{ ".org",	eval_org },
	{ ".ascii",	eval_string },
	{ ".asciiz",eval_string },
	{ ".entry",	eval_entry },
	{ ".global",eval_global },
	{ ".ifdef",	eval_ifdef },
	{ ".struct",eval_struct },
	{ "SFIELD",	eval_struct_field },
	{ "LABEL",	eval_label },
	{ ".equ",	eval_equ },
	{ ".const",	eval_const },
	{ "NAME",	eval_name },
	{ ".",		eval_curloc },
	{ "OP MX16",eval_op_mx16 },
	{ "OP (RN)",eval_op_noarg },
	{ "OP (N)",	eval_op_noarg },
	{ "OP (R)",	eval_op_noarg },
	{ "OP noarg",eval_op_noarg },
	{ "OP (R,T)",eval_op_short },
	{ "OP (T)",	eval_op_short },
	{ "SHC",	eval_op_short },
	{ "BLC",	eval_op_short },
	{ "BRC",	eval_op_short },
	{ "EXL",	eval_op_short },
	{ "NRF",	eval_op_short },
	{ "HLT",	eval_op_short },
	{ "PROG",	eval_err },
	{ "NORM",	eval_err },
	{ "(max)",	eval_err }
};

// -----------------------------------------------------------------------
void AADEBUG(char *format, ...)
{
	if (!aadebug) return;
	fprintf(stderr, "DEBUG: ");
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

// -----------------------------------------------------------------------
void aaerror(struct st *t, char *format, ...)
{
	va_list ap;
	int len = 0;

	if (t) {
		len = snprintf(aerr, MAX_ERRLEN, "%s:%d:%d: ", t->loc_file, t->loc_line, t->loc_col);
	}

	if (len<MAX_ERRLEN) {
		va_start(ap, format);
		vsnprintf(aerr+len, MAX_ERRLEN-len, format, ap);
		va_end(ap);
	}
	AADEBUG("Error logged: %s", aerr);
}
// -----------------------------------------------------------------------
int prog_cpu(char *cpu_name, int force)
{
	assert(cpu_name);

	// if cpu type was set in commandline, silently ignore .cpu directive
	if (cpu & CPU_FORCED) {
		return 0;
	}

	// if cpu type was already set using .cpu directive, fail
	if (cpu != CPU_DEFAULT) {
		return -1;
	}

	// first time setting cpu type
	if (!strcasecmp(cpu_name, "mera400")) {
		cpu = CPU_MERA400 | force;
		ic_max = 32767;
	} else if (!strcasecmp(cpu_name, "mx16")) {
		cpu = CPU_MX16 | force;
		ic_max = 65535;
	} else { // unknown CPU type
		return 1;
	}

	return 0;
}

// -----------------------------------------------------------------------
int eval_1arg(struct st *t)
{
	int u;
	struct st *arg = t->args;

	u = eval(arg);
	if (u) return u;

/*	if ((t->type == N_NEG) && (arg->relative)) {
		aaerror(t, "Invalid argument type for operator '%s': (%s)", eval_tab[t->type].name, arg->relative ? "relative" : "absolute");
		return -1;
	}
*/
	switch (t->type) {
		case N_UMINUS:
			t->val = -arg->val;
			break;
		case N_NEG:
			t->val = ~arg->val;
			break;
		default:
			assert(!"unknown 1-arg operator node");
			return -1;
	}

	t->type = N_INT;
	t->flags |= arg->flags & ST_RELATIVE;
	st_drop(arg);
	t->args = t->last = NULL;

	return 0;
}

// -----------------------------------------------------------------------
int eval_2arg(struct st *t)
{
	int u1, u2;
	struct st *arg1 = t->args;
	struct st *arg2 = t->args->next;

	u1 = eval(arg1);
	if (u1 < 0) return u1;

	u2 = eval(arg2);
	if (u2 < 0) return u2;

	if (u1 || u2) return 1;

/*	if ((t->type != N_PLUS) && (t->type != N_MINUS) && ((arg1->relative) || (arg2->relative))) {
		aaerror(t, "Invalid argument types for operator '%s': (%s, %s)",
			eval_tab[t->type].name,
			arg1->relative ? "relative" : "absolute",
			arg2->relative ? "relative" : "absolute");
		return -1;
	} else */if ((t->type == N_MINUS) && (arg1->flags & ST_RELATIVE) && (arg2->flags & ST_RELATIVE)) {
		t->flags &= ~ST_RELATIVE;
	} else {
		t->flags |= (arg1->flags | arg2->flags) & ST_RELATIVE;
	}

	switch (t->type) {
		case N_PLUS:
			t->val = arg1->val + arg2->val;
			break;
		case N_MINUS:
			t->val = arg1->val - arg2->val;
			break;
		case N_MUL:
			t->val = arg1->val * arg2->val;
			break;
		case N_DIV:
			if (arg2->val == 0.0) {
				aaerror(t, "Division by 0");
				return -1;
			}
			t->val = arg1->val / arg2->val;
			break;
		case N_REM:
			if (arg2->val == 0.0) {
				aaerror(t, "Division by 0");
				return -1;
			}
			t->val = arg1->val % arg2->val;
			break;
		case N_AND:
			t->val = arg1->val & arg2->val;
			break;
		case N_XOR:
			t->val = arg1->val ^ arg2->val;
			break;
		case N_OR:
			t->val = arg1->val | arg2->val;
			break;
		case N_LSHIFT:
			t->val = arg1->val << arg2->val;
			break;
		case N_RSHIFT:
			t->val = arg1->val >> arg2->val;
			break;
		case N_SCALE:
			t->val = arg1->val << (15-arg2->val);
			break;
		default:
			assert(!"unknown 2-arg operator");
			return -1;
	}

	t->type = N_INT;
	st_drop(t->args);
	t->args = t->last = NULL;

	return 0;
}

// -----------------------------------------------------------------------
int eval_float(struct st *t)
{
	uint16_t regs[4]; // r0...r3, flags stored in r0
	int res = awp_from_double(regs, t->flo);
	if (!t->data) {
		t->data = malloc(3 * sizeof(uint16_t));
	}
	memcpy(t->data, regs+1, 3 * sizeof(uint16_t));

	// check for overflow/underflow
	switch (res) {
		case AWP_FP_OF:
			aaerror(t, "Floating point overflow");
			return -1;
		case AWP_FP_UF:
			aaerror(t, "Floating point underflow");
			return -1;
	}

	return 0;
}

// -----------------------------------------------------------------------
int eval_word(struct st *t)
{
	int u;

	t->size = 1;

	u = eval(t->args);
	if (u) {
		return u;
	}

	switch (t->type) {
		case N_WORD:
			if ((t->args->val < -32768) || (t->args->val > 65535)) {
				aaerror(t, "Value %i is not an 16-bit signed/unsigned integer", t->args->val);
				return -1;
			}
			t->val = t->args->val;
			break;
	}

	t->type = N_INT;
	t->flags |= t->args->flags & ST_RELATIVE;
	st_drop(t->args);
	t->args = t->last = NULL;

	return 0;
}

// -----------------------------------------------------------------------
int eval_multiword(struct st *t)
{
	int u;
	struct st *arg = t->args;

	switch (t->type) {
		case N_DWORD: t->size = 2; break;
		case N_FLOAT: t->size = 3; break;
		default: assert(!"not a multiword"); break;
	}

	if ((t->size < 0) || (t->size > 65536)) {
		aaerror(t, "Cannot fit the array in a process address space (%i words needed)", t->size);
		return -1;
	}

	if (!t->data) {
		t->data = malloc(t->size * sizeof(uint16_t));
	}

	u = eval(arg);
	if (u) {
		return u;
	}

	switch (t->type) {
		case N_DWORD:
			t->data[0] = arg->val >> 16;
			t->data[1] = arg->val & 65535;
			break;
		case N_FLOAT:
			t->data[0] = arg->data[0];
			t->data[1] = arg->data[1];
			t->data[2] = arg->data[2];
			break;
		default:
			assert(!"not a multiword");
			break;
	}

	t->type = N_BLOB;
	st_drop(t->args);
	t->args = t->last = NULL;

	return 0;
}

// -----------------------------------------------------------------------
int eval_res(struct st *t)
{
	int u;
	int value = 0;

	// first, we need element count
	u = eval(t->args);
	if (u) {
		return -1;
	}

	if ((t->args->val < 0) || (t->args->val > 65536)) {
		aaerror(t, "Cannot reserve memory outside the process address space (requested %i words)", t->args->val);
		return -1;
	}

	t->size = t->args->val;

	// then, check if user specified a value to fill with
	if (t->args->next) {
		u = eval(t->args->next);
		if (u) {
			return u;
		}
		value = t->args->next->val;
	}

	if (!t->data) {
		t->data = malloc(t->size * sizeof(uint16_t));
		for (int i=0 ; i<t->size ; i++) t->data[i] = value;
	}

	t->type = N_BLOB;
	st_drop(t->args);
	t->args = t->last = NULL;

	return 0;
}

// -----------------------------------------------------------------------
int eval_org(struct st *t)
{
	int u = eval(t->args);
	if (u) {
		return -1;
	}

	if (t->args->val < ic) {
		aaerror(t, "Cannot move location pointer backwards by %i words", t->args->val - ic);
		return -1;
	}
	ic = t->args->val;
	t->type = N_NONE;
	st_drop(t->args);
	t->args = t->last = NULL;

	return 0;
}

// -----------------------------------------------------------------------
int eval_string(struct st *t)
{
	char *s = t->str;
	int chars, words;
	int pos = 1; // start with left byte

	chars = t->val;
	if (t->type == N_ASCII) chars--;
	words = (chars+1) / 2;

	if ((words < 0) || (words > 65536)) {
		aaerror(t, "Cannot fit the string in a process address space (%i words needed)", words);
		return -1;
	}

	t->data = malloc(words * sizeof(uint16_t));
	t->size = 0;
	t->type = N_BLOB;

	// append each double-character as value
	while (chars > 0) {
		if (pos == 1) {
			t->data[t->size] = *s << 8;
		} else {
			t->data[t->size] |= *s & 0xff;
		}

		pos *= -1;
		s++;
		chars--;

		// flush
		if ((pos == 1) || (chars <= 0)) {
			t->size++;
		}
	}

	return 0;
}

// -----------------------------------------------------------------------
int eval_label(struct st *t)
{
	struct dh_elem *s;
	struct st *tic;

	s = dh_get(sym, t->str);

	if (!s) {
		tic = st_int(N_INT, ic);
		tic->flags |= ST_RELATIVE;
		dh_addt(sym, t->str, SYM_CONST, tic);
	} else if (s->type & SYM_UNDEFINED) {
		// this is when .global label appears before label
		s->type &= ~SYM_UNDEFINED;
		s->type |= SYM_CONST;
		tic = st_int(N_INT, ic);
		tic->flags |= ST_RELATIVE;
		s->t = tic;
	} else {
		aaerror(t, "Symbol '%s' already defined", t->str);
		return -1;
	}

	t->type = N_NONE;

	return 0;
}

// -----------------------------------------------------------------------
int eval_equ(struct st *t)
{
	int u;
	struct dh_elem *s;

	u = eval(t->args);
	if (u < 0) {
		return u;
	}

	s = dh_get(sym, t->str);

	if (!s) {
		dh_addt(sym, t->str, 0, t->args);
	} else if (s->type & SYM_CONST) { // defined, but constant
		aaerror(t, "Const symbol '%s' cannot be redefined", t->str);
		return -1;
	} else if (s->type & SYM_UNDEFINED) { // is there, but undefined
		s->type &= ~SYM_UNDEFINED;
		s->t = t->args;
	} else { // defined, variable - just redefine
		st_drop(s->t);
		s->t = t->args;
	}

	t->type = N_NONE;
	t->args = NULL;

	return 0;
}

// -----------------------------------------------------------------------
int eval_const(struct st *t)
{
	int u;
	struct dh_elem *s;

	u = eval(t->args);
	if (u < 0) {
		return u;
	}

	s = dh_get(sym, t->str);

	if (!s) {
		dh_addt(sym, t->str, SYM_CONST, t->args);
	} else if (s->type & SYM_UNDEFINED) { // is there, but undefined
		s->type &= ~SYM_UNDEFINED;
		s->t = t->args;
	} else {
		aaerror(t, "Symbol '%s' already defined", t->str);
		return -1;
	}

	t->type = N_NONE;
	t->args = NULL;

	return 0;}

// -----------------------------------------------------------------------
int eval_entry(struct st *t)
{
	if (entry) {
		aaerror(t, "Program entry already defined");
		return -1;
	}

	entry = t->args;

	t->type = N_NONE;
	t->args = NULL;

	return 0;
}

// -----------------------------------------------------------------------
int eval_global(struct st *t)
{
	struct dh_elem *s;

	s = dh_get(sym, t->str);

	if (s) {
		s->type |= SYM_GLOBAL;
	} else {
		dh_addv(sym, t->str, SYM_UNDEFINED | SYM_GLOBAL, 0);
	}

	t->type = N_NONE;

	return 0;
}

// -----------------------------------------------------------------------
int eval_ifdef(struct st *t)
{
	struct st *prog;
	struct dh_elem *s = dh_get(sym, t->str);

	if (s && !(s->type & SYM_UNDEFINED)) {
		// first argument holds the program block for 'symbol defined' case
		prog = t->args;
	} else {
		// second argument holds the program block for 'symbol undefined' case
		prog = t->args->next;
	}

	assert(prog);

	// if there is any code in this program block
	if (prog->args) {
		// link the code that follows .endif after the last instruction in the block
		prog->last->next = t->next;
		// link the conditional code just after this node
		t->next = prog->args;
		// unlink the injected code from the conditional block (no double-frees)
		prog->args = NULL;
	}

	// ignore the node from now on
	t->type = N_NONE;

	return 0;
}

// -----------------------------------------------------------------------
int eval_struct(struct st *t)
{
	struct dh_elem *s;

	s = dh_get(sym, t->str);
	if (!s) {
		s = dh_addt(sym, t->str, SYM_CONST | SYM_UNDEFINED, st_int(N_INT, 0));
	}

	// evaluate all arguments (struct fields)
	struct st *args = t->args;
	while (args) {
		int u = eval(args);
		if (u) {
			return u;
		}
		args = args->next;
	}

	// update structure size only when all field sizes are known
	s->t->val = t->last->val + t->last->args->val;
	s->type &= ~SYM_UNDEFINED;

	// drop the arguments subtree
	t->type = N_NONE;
	st_drop(t->args);
	t->args = t->last = NULL;

	return 0;
}

// -----------------------------------------------------------------------
int eval_struct_field(struct st *t)
{
	int u;
	struct dh_elem *s;

	s = dh_get(sym, t->str);
	if (!s) {
		s = dh_addt(sym, t->str, SYM_CONST | SYM_UNDEFINED, st_int(N_INT, 0));
	}

	if (!t->prev) { // offset for the first element is always known = 0
		t->val = 0;
		s->t->val = t->val;
		s->type &= ~SYM_UNDEFINED;
	} else { // if this is not the first element
		if (t->prev->args->type == N_INT) { // size of the previous field is known
			struct dh_elem *ps = dh_get(sym, t->prev->str); // get the offset
			if (!(ps->type & SYM_UNDEFINED)) { // offset of the previous field is known
				t->val = t->prev->args->val + t->prev->val; // update this element offset
				s->t->val = t->val; // update element in the dictionary
				s->type &= ~SYM_UNDEFINED;
			}
		}
	}

	// evaluate size of this struct element
	u = eval(t->args);

	return u;
}

// -----------------------------------------------------------------------
int eval_name(struct st *t)
{
	int u;
	struct dh_elem *s = dh_get(sym, t->str);

	if (!s || (s->type & SYM_UNDEFINED)) {
		aaerror(t, "Symbol '%s' not defined", t->str);
		return 1;
	}

	assert(s->t);

	if (s->being_evaluated > 0) {
		aaerror(t, "Symbol '%s' is defined recursively", t->str);
		return 1;
	}

	s->being_evaluated++;
	u = eval(s->t);
	if (u) {
		s->being_evaluated--;
		return u;
	}
	s->being_evaluated--;

	t->val = s->t->val;
	t->flags |= s->t->flags & ST_RELATIVE;
	t->type = N_INT;

	return 0;
}

// -----------------------------------------------------------------------
int eval_curloc(struct st *t)
{
	t->type = N_INT;
	t->val = ic;
	t->flags |= ST_RELATIVE;
	return 0;
}

// -----------------------------------------------------------------------
int eval_as_short(struct st *t, int type, int op)
{
	int min, max;
	int opl, rel_op = 0;

	int u = eval(t);
	if (u) {
		return u;
	}

	switch (type) {
		case N_OP_SHC:
			min = -15; max = 15;
			break;
		case N_OP_T:
			min = -63; max = 63;
			rel_op = 1;
			break;
		case N_OP_RT:
			min = -63; max = 63;
			// relative argument with instruction that use relative addresses?
			// IRB, DRB, LWS, RWS use adresses relative to IC
			opl = (op >> 10) & 0b111;
			rel_op = ((opl == 0b010) || (opl == 0b011) || (opl == 0b110) || (opl == 0b111)) ? 1 : 0;
			break;
		case N_OP_HLT:
			min = 0; max = 127;
			break;
		case N_OP_BRC:
		case N_OP_EXL:
		case N_OP_NRF:
			min = 0; max = 255;
		default:
			min = -32768; max = 65535;
			break;
	}

	if (rel_op && (t->flags & ST_RELATIVE)) {
		int diff = t->val - (ic+1);
		if (diff >= 65535 - 63) {
			t->val = diff - 65536;
		} else if (diff <= -65535 + 63) {
			t->val = diff + 65536;
		} else {
			t->val = diff;
		}
	}

	if ((t->val < min) || (t->val > max)) {
		aaerror(t, "Argument value %i for %s is out of range (%i..%i)", t->val, eval_tab[t->type].name, min, max);
		return -1;
	}

	return 0;
}

// -----------------------------------------------------------------------
int eval_op_short(struct st *t)
{
	int u;
	struct st *arg = t->args;

	t->size = 1;

	u = eval_as_short(arg, t->type, t->val);
	if (u) {
		return u;
	}

	switch (t->type) {
		case N_OP_SHC:
			if (arg->val < 0) {
				arg->val = 16 + arg->val;
			}
			arg->val = (arg->val & 0b111) | ((arg->val & 0b1000) << 6);
			break;
		case N_OP_T:
		case N_OP_RT:
			if (arg->val < 0) {
				arg->val = -arg->val;
				arg->val |= 0b0000001000000000;
			}
			break;
		case N_OP_HLT:
			arg->val |= (arg->val & 0b0000000001000000) << 3;
			arg->val &= 0b1111111110111111;
			break;
		case N_OP_BRC:
			if (arg->val > 255) {
				aaerror(t, "Upper byte for BRC argument is not 0");
				return -1;
			}
			break;
		case N_OP_BLC:
			if (arg->val & 255) {
				aaerror(t, "Lower byte for BLC argument is not 0");
				return -1;
			}
			arg->val = arg->val >> 8;
			break;
		default:
			break;
	}

	t->type = N_INT;
	t->val |= arg->val;
	st_drop(arg);
	t->args = t->last = NULL;

	return 0;
}

// -----------------------------------------------------------------------
int eval_op_mx16(struct st *t)
{
	if (!(cpu & CPU_MX16)) {
		aaerror(t, "Instruction valid only for MX-16");
		return -1;
	} else {
		return eval_op_noarg(t);
	}
}

// -----------------------------------------------------------------------
int eval_op_noarg(struct st *t)
{
	t->type = N_INT;
	t->size = 1;

	return 0;
}

// -----------------------------------------------------------------------
int eval_none(struct st *t)
{
	return 0;
}

// -----------------------------------------------------------------------
int eval_err(struct st *t)
{
	aaerror(t, "Cannot eval node type %i", t->type);
	return -1;
}

// -----------------------------------------------------------------------
int eval(struct st *t)
{
	if ((t->type >= N_MAX) || (t->type < 0)) {
		return eval_err(t);
	}

	AADEBUG(" eval: %s", eval_tab[t->type].name);

	return eval_tab[t->type].fun(t);
}

// -----------------------------------------------------------------------
int assemble(struct st *prog, int keep_going)
{
	AADEBUG("==== Assemble ================================");
	struct st *t = prog->args;
	int u = 0;
	int uret = 0;

	ic = 0;

	while (t) {
		if (ic > ic_max) {
			aaerror(t, "Program too large (>%i words)", ic_max+1);
			return -1;
		}
		if (t->ic < 0) {
			t->ic = ic;
		} else {
			ic = t->ic;
		}
		AADEBUG("---- IC=%i, Top node: %s ----", ic, eval_tab[t->type].name);
		u = eval(t);
		AADEBUG("---- eval ret: %i", u);
		ic += t->size;
		if ((u < 0) || ((u > 0) && !keep_going)) {
			return u;
		} else {
			uret += u;
		}
		t = t->next;
	}

	return uret;
}

// -----------------------------------------------------------------------
int add_const(char *name, int val)
{
	struct dh_elem *s;

	s = dh_get(sym, name);

	if (!s) {
		struct st *t = st_int(N_INT, val);
		dh_addt(sym, name, SYM_CONST, t);
	} else {
		s->value = val;
	}

	return 0;
}

// vim: tabstop=4 autoindent
