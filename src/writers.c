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
#include <string.h>
#include <arpa/inet.h>
#include <emelf.h>

#include "prog.h"
#include "st.h"

#define MEM_MAX 64 * 1024

uint16_t image[MEM_MAX];
int image_size;

// -----------------------------------------------------------------------
// convert an integer to formatted string with its binary representation
static char * int2binf(char *format, uint64_t value, int size)
{
	char *i = format;
	char *buf = malloc(strlen(format)+1);
	char *o = buf;

	size--;

	while (*i) {
		switch (*i) {
			case '.':
				if (size >= 0) {
					*o = (value >> size) & 1 ? '1' : '0';
					size--;
				} else {
					*o = '?';
				}
				break;
			default:
				*o = *i;
				break;
		}
		o++;
		i++;
	}
	*o = '\0';
	return buf;
}

// -----------------------------------------------------------------------
int writer_debug(struct st *prog, FILE *f)
{
	struct st *t = prog->args;
	char *bin;

	while (t) {
		switch (t->type) {
			case N_INT:
				bin = int2binf("... ... . ... ... ...", t->val, 16);
				fprintf(f, "@ 0x%04x : 0x%04x  /  %s  /  %i\n", t->ic, (uint16_t) t->val, bin, t->val);
				free(bin);
				break;
			case N_BLOB:
				for (int i=0 ; i<t->size ; i++) {
					char *bin = int2binf("... ... . ... ... ...", t->data[i], 16);
					fprintf(f, "@ 0x%04x : 0x%04x  /  %s  /  %i\n", t->ic+i, (uint16_t) t->data[i], bin, t->data[i]);
					free(bin);
				}
				break;
			case N_NONE:
				fprintf(f, "@ 0x%04x : (none, type %d)\n", t->ic, t->type);
				break;
			default:
				fprintf(f, "@ 0x%04x : unresolved\n", t->ic);
				break;
		}
		t = t->next;
	}
	return 0;
}

// -----------------------------------------------------------------------
static void img_put(struct st *t)
{
	switch (t->type) {
		case N_INT:
			if (t->ic+1 > image_size) image_size = t->ic+1;
			image[t->ic] = t->val;
			break;
		case N_BLOB:
			for (int i=0 ; i<t->size ; i++) {
				if (t->ic+1 > image_size) image_size = t->ic+1;
				image[t->ic+i] = t->data[i];
			}
			break;
	}
}

// -----------------------------------------------------------------------
int writer_raw(struct st *prog, FILE *f)
{
	int res;
	struct st *t;
	int pos;

	t = prog->args;
	while (t) {
		switch (t->type) {
			case N_INT:
			case N_BLOB:
				img_put(t);
				break;
			case N_NONE:
				break;
			default:
				aaerror(t, "Relocation not possible for raw output");
				return 1;
		}
		t = t->next;
	}

	pos = image_size - 1;
	while (pos >= 0) {
		image[pos] = htons(image[pos]);
		pos--;
	}

	res = fwrite(image, 2, image_size, f);
	if (res < 0) {
		aaerror(NULL, "Write failed");
		return 1;
	}

	return 0;
}

// -----------------------------------------------------------------------
static int try_reloc(struct st *t, struct emelf *e)
{
	int sym_idx;
	struct st *op = NULL, *arg1 = NULL, *arg2 = NULL;
	struct st *arg_sym = NULL;
	struct st *arg_int = NULL;

	int flags = 0;

	if (t->type != N_WORD) {
		return 1;
	}

	op = t->args;
	if (op) arg1 = op->args;
	if (arg1) arg2 = arg1->next;

	switch (op->type) {
		case N_NAME:
			arg_sym = op;
			break;
		case N_UMINUS:
			if (arg1->type == N_NAME) {
				arg_sym = arg1;
				flags = EMELF_RELOC_SYM_NEG;
			}
			break;
		case N_PLUS:
			if ((arg1->type == N_NAME) && (arg2->type == N_INT)) {
				arg_sym = arg1;
				arg_int = arg2;
			} else if ((arg1->type == N_INT) && (arg2->type == N_NAME)) {
				arg_int = arg1;
				arg_sym = arg2;
			}
			break;
		case N_MINUS:
			if ((arg1->type == N_NAME) && (arg2->type == N_INT)) {
				arg_sym = arg1;
				arg_int = arg2;
				arg_int->val *= -1;
			} else if ((arg1->type == N_INT) && (arg2->type == N_NAME)) {
				arg_int = arg1;
				arg_sym = arg2;
				flags = EMELF_RELOC_SYM_NEG;
			}
			break;
		default:
			return 1;
	}

	if (!arg_sym) {
		return 1;
	}

	if (arg_int) {
		image[t->ic] = arg_int->val;
		if (arg_int->relative) {
			flags |= EMELF_RELOC_BASE;
		}
	} else {
		image[t->ic] = 0;
	}
	if (t->ic+1 > image_size) image_size = t->ic+1;

	sym_idx = emelf_symbol_add(e, EMELF_SYM_NOFLAGS, arg_sym->str, 0);
	emelf_reloc_add(e, t->ic, EMELF_RELOC_SYM | flags, sym_idx);

	return 0;
}

// -----------------------------------------------------------------------
static int write_global_symbols(struct dh_table *dh, struct emelf *e)
{
	int i;
	int rel;
	struct dh_elem *elem;

	for(i=0 ; i<dh->size ; i++) {
		elem = dh->slots[i];
		while (elem) {
			if (elem->type & SYM_GLOBAL) {
				if (eval(elem->t)) {
					return 1;
				}
				rel = elem->t->relative ? EMELF_SYM_RELATIVE : 0;
				emelf_symbol_add(e, EMELF_SYM_GLOBAL | rel, elem->name, elem->t->val);
			}
			elem = elem->next;
		}
	}
	return 0;
}

// -----------------------------------------------------------------------
int writer_emelf(struct st *prog, struct dh_table *symbols, FILE *f)
{
	int res;
	int ret = 1;
	struct emelf *e = NULL;
	struct st *t;

	e = emelf_create(EMELF_RELOC, (cpu == CPU_MX16) ? EMELF_CPU_MX16 : EMELF_CPU_MERA400);
	if (!e) {
		aaerror(NULL, "Error creating EMELF structure");
		goto cleanup;
	}

	t = prog->args;
	while (t) {
		switch (t->type) {
			case N_INT:
				if (t->relative) {
					emelf_reloc_add(e, t->ic, EMELF_RELOC_BASE, -1);
				}
			case N_BLOB:
				img_put(t);
				break;
			case N_NONE:
				break;
			default:
				if (try_reloc(t, e)) {
					aaerror(t, "Unable to relocate");
					goto cleanup;
				}
				break;
		}
		t = t->next;
	}

	if (write_global_symbols(symbols, e)) {
		goto cleanup;
	}

	res = emelf_image_append(e, image, image_size);
	if (res != EMELF_E_OK) {
		aaerror(NULL, "Cannot append image to output emelf structure");
		goto cleanup;
	}

	if (entry) {
		eval(entry);
		if (entry->type != N_INT) {
			goto cleanup;
		}
		res = emelf_entry_set(e, entry->val);
		if (res != EMELF_E_OK) {
			aaerror(entry, "Cannot set program entry point to %i", entry->val);
			goto cleanup;
		}
	}

	res = emelf_write(e, f);
	if (res != EMELF_E_OK) {
		aaerror(NULL, "Error writing emelf output");
		goto cleanup;
	}

	ret = 0;

cleanup:
	emelf_destroy(e);

	return ret;
}


// vim: tabstop=4 autoindent
