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

#include "dh.h"
#include "st.h"
#include "parser.h"
#include "prog.h"

struct dh_table *sym;
struct st *program;

char aerr[MAX_ERRLEN+1];

int ic;
int cpu = CPU_DEFAULT;
int ic_max = 32767;

// -----------------------------------------------------------------------
int prog_cpu(char *cpu_name)
{
	assert(cpu_name);

	int ret = 1;
	if (cpu != CPU_DEFAULT) {
		yyerror("CPU type already set");
		ret = 0;
	} else {
		if (!strcasecmp(cpu_name, "mera400")) {
			cpu = CPU_MERA400;
		} else if (!strcasecmp(cpu_name, "mx16")) {
			cpu = CPU_MX16;
			ic_max = 65535;
		} else {
			yyerror("Unknown CPU type '%s'", cpu_name);
			ret = 0;
		}
	}

	return ret;
}

// -----------------------------------------------------------------------
struct st * compose_norm(int type, int opcode, int reg, struct st *norm)
{
	struct st *op = st_int(type, opcode | reg | norm->val);
	struct st *data = NULL;
	if (norm->args) {
		data = st_arg(P_WORD, norm->args, NULL);
		norm->args = NULL;
	}
	st_drop(norm);
	return st_app(op, data);
}

// -----------------------------------------------------------------------
void aaerror(struct st *t, char *format, ...)
{
	assert(t && format);

	va_list ap;

	int len = snprintf(aerr, MAX_ERRLEN, "%s: Error at line %d column %d: ", t->loc_file, t->loc_line, t->loc_col);

	if (len && (len<MAX_ERRLEN)) {
		va_start(ap, format);
		vsnprintf(aerr+len, MAX_ERRLEN-len, format, ap);
		va_end(ap);
	}
}

// -----------------------------------------------------------------------
int eval_1arg(struct st *t)
{
	int u;
	struct st *arg = t->args;

	u = eval(arg);
	if (u) return u;

	if ((t->type == '~') && (arg->relative)) {
		aaerror(t, "Invalid argument type for operator: '%c' (%s)", t->type, arg->relative ? "relative" : "absolute");
		return -1;
	}

	switch (t->type) {
		case UMINUS:
			t->val = -arg->val;
			break;
		case '~':
			t->val = ~arg->val;
			break;
		default:
			aaerror(t, "Unknown operator: #%d ('%c')", t->type, t->type);
			return -1;
	}

	t->type = INT;
	t->relative = arg->relative;
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

	if ((t->type != '+') && (t->type != '-') && ((arg1->relative) || (arg2->relative))) {
		char oper[3];
		if (t->type == LSHIFT) sprintf(oper, "<<");
		else if (t->type == RSHIFT) sprintf(oper, ">>");
		else sprintf(oper, "%c", t->type);
		aaerror(t, "Invalid argument types for operator '%s': (%s, %s)", oper,
			arg1->relative ? "relative" : "absolute",
			arg2->relative ? "relative" : "absolute");
		return -1;
	} else if ((t->type == '-') && (arg1->relative) && (arg2->relative)) {
		t->relative = 0;
	} else {
		t->relative = arg1->relative | arg2->relative;
	}

	switch (t->type) {
		case '+':
			t->val = arg1->val + arg2->val;
			break;
		case '-':
			t->val = arg1->val - arg2->val;
			break;
		case '*':
			t->val = arg1->val * arg2->val;
			break;
		case '/':
			t->val = arg1->val / arg2->val;
			break;
		case '%':
			t->val = arg1->val % arg2->val;
			break;
		case '&':
			t->val = arg1->val & arg2->val;
			break;
		case '^':
			t->val = arg1->val ^ arg2->val;
			break;
		case '|':
			t->val = arg1->val | arg2->val;
			break;
		case LSHIFT:
			t->val = arg1->val << arg2->val;
			break;
		case RSHIFT:
			t->val = arg1->val >> arg2->val;
			break;
		case '\\':
			t->val = arg1->val << (15-arg2->val);
			break;
		default:
			aaerror(t, "Unknown operator: #%s ('%c')", t->type, t->type);
			return -1;
	}

	t->type = INT;
	st_drop(t->args);
	t->args = t->last = NULL;

	return 0;
}

#define FP_BITS 64
#define FP_BIT(n, z) ((z) & (1ULL << (FP_BITS-1-n)))
#define FP_M_MAX 0b0111111111111111111111111111111111111111000000000000000000000000
#define FP_CORRECTION (1ULL << (FP_BITS-1-39))

// -----------------------------------------------------------------------
int eval_float(struct st *t)
{
	if (!t->data) {
		t->data = malloc(3 * sizeof(uint16_t));
	}

	int exp;
	double m = frexp(t->flo, &exp);
	int64_t m_int = ldexp(m, FP_BITS-1);

	if ((m_int != 0) && ((FP_BIT(0, m_int) >> 1) == FP_BIT(1, m_int))) {
		m_int <<= 1;
		exp--;
	}

	if (FP_BIT(40, m_int)) {
		if ((m_int & FP_M_MAX) == FP_M_MAX) {
			m_int >>= 1;
			exp++;
			m_int += FP_CORRECTION >> 1;
		} else {
			m_int += FP_CORRECTION;
		}
	}
	// check for overflow/underflow
	if (exp > 127) {
		aaerror(t, "Floating point overflow");
		return -1;
	} else if (((m != 0.0f) && exp < -128)) {
		aaerror(t, "Floating point underflow");
		return -1;
	}

	t->data[0] = m_int >> 48;
	t->data[1] = m_int >> 32;
	t->data[2] = (m_int >> 16) & 0b1111111100000000;
	t->data[2] |= exp & 255;

	return 0;
}

// -----------------------------------------------------------------------
int eval_multiword(struct st *t)
{
	int u;
	int uret = 0;
	struct st *arg = t->args;
	int count = 0;
	int chunk_size;

	switch (t->type) {
		case P_DWORD: chunk_size = 2; break;
		case P_FLOAT: chunk_size = 3; break;
		default: chunk_size = 1; break;
	}

	if (!t->data) {
		t->data = malloc(DATA_MAX * chunk_size * sizeof(uint16_t));
	}

	while (arg) {
		if (count >= DATA_MAX * chunk_size) {
			aaerror(t, "Argument list too long (%d max)", DATA_MAX);
			return -1;
		}

		u = eval(arg);

		if (u < 0) {
			return u;
		} else if (u > 0) {
			uret += u;
		} else {
			switch (t->type) {
				case P_WORD:
				case P_RBYTE:
					t->data[count] = arg->val;
					break;
				case P_LBYTE:
					t->data[count] <<= 8;
					break;
				case P_DWORD:
					t->data[count] = arg->val >> 16;
					t->data[count+1] = arg->val & 65535;
					break;
				case P_FLOAT:
					t->data[count] = arg->data[0];
					t->data[count+1] = arg->data[1];
					t->data[count+2] = arg->data[2];
					break;
				default:
					assert(0);
					break;
			}
		}

		ic += chunk_size;
		count += chunk_size;
		arg = arg->next;
	}

	if (uret) {
		return uret;
	}

	t->type = BLOB;
	t->val = count;
	st_drop(t->args);
	t->args = t->last = NULL;

	return 0;
}

// -----------------------------------------------------------------------
int eval_res(struct st *t)
{
	int u;
	int count;
	int value = 0;

	// first, we need element count
	u = eval(t->args);
	if (u) {
		return -1;
	}

	count = t->args->val;
	ic += count;

	// then, check if user specified a value to fill with
	if (t->args->next) {
		u = eval(t->args->next);
		if (u) {
			return u;
		}
		value = t->args->next->val;
	}

	t->data = malloc(count * sizeof(uint16_t));
	for (int i=0 ; i<count ; i++) t->data[i] = value;

	t->type = BLOB;
	t->val = count;
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

	ic = t->args->val;
	t->type = NONE;
	st_drop(t->args);
	t->args = t->last = NULL;

	return 0;
}

// -----------------------------------------------------------------------
int eval_string(struct st *t)
{
	char *s = t->str;
	int len = strlen(s);
	int pos = 1; // start with left byte
	int count = 0;

	if (t->type == P_ASCIIZ) len++;

	if (!t->data) {
		t->data = malloc((len+1)/2);
	}

	// append each double-character as value
	while (s && (len > 0)) {
		if (pos == 1) {
			t->data[count] = (*s) << 8;
		} else {
			t->data[count] += *s;
		}

		pos *= -1;
		s++;
		len--;

		// flush
		if ((pos == 1) || (len <= 0)) {
			ic++;
			count++;
		}
	}

	t->type = BLOB;
	t->val = count;

	return 0;
}

// -----------------------------------------------------------------------
int eval_label(struct st *t)
{
	if (dh_get(sym, t->str)) {
		aaerror(t, "Symbol '%s' already defined", t->str);
		return -1;
	}

	dh_addv(sym, t->str, SYM_RELATIVE | SYM_CONST | SYM_LOCAL, ic);

	t->type = NONE;

	return 0;
}

// -----------------------------------------------------------------------
int eval_equ(struct st *t)
{
	int u;
	struct dh_elem *s = dh_get(sym, t->str);

	if (s) {
		if (s->type & SYM_CONST) { // defined, but constant
			aaerror(t, "Symbol '%s' cannot be redefined", t->str);
			return -1;
		} else { // defined, variable - just redefine
			dh_delete(sym, t->str);
		}
	}

	u = eval(t->args);
	if (u < 0) {
		return u;
	}

	dh_addt(sym, t->str, SYM_LOCAL, t->args);

	t->type = NONE;
	t->args = NULL;

	return 0;
}

// -----------------------------------------------------------------------
int eval_const(struct st *t)
{
	int u;

	if (dh_get(sym, t->str)) {
		aaerror(t, "Symbol '%s' already defined", t->str);
		return -1;
	}

	u = eval(t->args);
	if (u < 0) {
		return u;
	}

	dh_addt(sym, t->str, SYM_LOCAL | SYM_CONST, t->args);

	t->type = NONE;
	t->args = NULL;

	return 0;
}

// -----------------------------------------------------------------------
int eval_name(struct st *t)
{
	int u;
	struct dh_elem *s = dh_get(sym, t->str);

	if (!s) {
		aaerror(t, "Symbol '%s' not defined", t->str);
		return 1;
	}

	if (s->t) { // entry with tree
		u = eval(s->t);
		if (u) {
			return u;
		}
		t->val = s->t->val;
	} else { // entry with just a value
		t->val = s->value;
	}

	if ((s->type & SYM_RELATIVE) || (s->t->relative)) {
		t->relative = 1;
	}

	t->type = INT;

	return 0;
}

// -----------------------------------------------------------------------
int eval_curloc(struct st *t)
{
	t->type = INT;
	t->val = ic;
	t->relative = 1;
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
		case OP_SHC:
			min = 0; max = 15;
			break;
		case OP_T:
			min = -63; max = 63;
			rel_op = 1;
			break;
		case OP_RT:
			min = -63; max = 63;
			// relative argument with instruction that use relative addresses?
			// IRB, DRB, LWS, RWS use adresses relative to IC
			opl = (op >> 10) & 0b111;
			rel_op = ((opl == 0b010) || (opl == 0b011) || (opl == 0b110) || (opl == 0b111)) ? 1 : 0;
			break;
		case OP_HLT:
			min = -63; max = 63;
			break;
		case OP_BRC:
		case OP_EXL:
		case OP_NRF:
			min = 0; max = 255;
		default:
			min = -32768; max = 65535;
			break;
	}

	if (rel_op && t->relative) {
		t->val -= ic;
	}

	if ((t->val < min) || (t->val > max)) {
		aaerror(t, "Short argument value %i out of range (%i..%i)", t->val, min, max);
		return -1;
	}

	return 0;
}

// -----------------------------------------------------------------------
int eval_op_short(struct st *t)
{
	int u;
	struct st *arg = t->args;

	ic++;

	u = eval_as_short(arg, t->type, t->val);
	if (u) {
		return u;
	}

	switch (t->type) {
		case OP_SHC:
			arg->val = (arg->val & 0b111) | ((arg->val & 0b1000) << 6);
			break;
		case OP_T:
		case OP_RT:
		case OP_HLT:
			if (arg->val < 0) {
				arg->val = -arg->val;
				arg->val |= 0b0000001000000000;
			}
			break;
		case OP_BLC:
			if (arg->val & 255) {
				aaerror(t, "Lower byte for BLC argument is not 0");
				return -1;
			}
			arg->val = arg->val >> 8;
			break;
		default:
			break;
	}

	t->type = INT;
	t->val |= arg->val;
	st_drop(arg);
	t->args = t->last = NULL;

	return 0;
}

// -----------------------------------------------------------------------
int eval_op_noarg(struct st *t)
{
	t->type = INT;
	ic++;

	return 0;
}

// -----------------------------------------------------------------------
int eval(struct st *t)
{
	assert(t);

	switch (t->type) {
		case NONE:
		case INT:
		case BLOB:
			return 0;
		case FLOAT:
			return eval_float(t);
		case '+':
		case '-':
		case '*':
		case '/':
		case '%':
		case '&':
		case '^':
		case '|':
		case LSHIFT:
		case RSHIFT:
		case '\\':
			return eval_2arg(t);
		case UMINUS:
		case '~':
			return eval_1arg(t);
		case P_WORD:
		case P_DWORD:
		case P_LBYTE:
		case P_RBYTE:
		case P_FLOAT:
			return eval_multiword(t);
		case P_RES:
			return eval_res(t);
		case P_ORG:
			return eval_org(t);
		case P_ASCII:
		case P_ASCIIZ:
			return eval_string(t);
		case P_ENTRY:
		case P_GLOBAL:
			return 0;
		case LABEL:
			return eval_label(t);
		case P_EQU:
			return eval_equ(t);
		case P_CONST:
			return eval_const(t);
		case NAME:
			return eval_name(t);
		case CURLOC:
			return eval_curloc(t);
		case OP_X:
			if (cpu != CPU_MX16) {
				aaerror(t, "Instruction valid only for MX-16");
				return -1;
			}
		case OP_RN:
		case OP_N:
		case OP_R:
		case OP__:
			return eval_op_noarg(t);
		case OP_RT:
		case OP_T:
		case OP_SHC:
		case OP_BLC:
		case OP_BRC:
		case OP_EXL:
		case OP_NRF:
		case OP_HLT:
			return eval_op_short(t);
	}

	aaerror(t, "Unknown syntax element: %i", t->type);
	return -1;
}

// -----------------------------------------------------------------------
int assemble(struct st *prog, int pass)
{
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
		u = eval(t);
		if (u < 0) {
			return u;
		}
		if ((pass > 0) && (u > 0)) {
			return -1;
		}
		uret += u;
		t = t->next;
	}

	return uret;
}

// vim: tabstop=4 autoindent
