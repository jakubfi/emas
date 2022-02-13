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
#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include "prog.h"
#include "st.h"

#define MEM_MAX 64 * 1024

uint16_t image[MEM_MAX];
int rel_count;
int icmax = -1;

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

	AADEBUG("==== DEBUG writer ================================");
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
static void keys_print(FILE *f, uint16_t addr, uint16_t data)
{
	char *bin;
	bin = int2binf(".... .... .... ....", data, 16);
	fprintf(f, "%4i: %06o   %s   ", addr, data, bin);
	free(bin);
	int first = 1;
	for (int i=0; i<16 ; i++) {
		if ((data & (1<<(15-i)))) {
			if (!first) {
				fprintf(f, ", %i", i);
			} else {
				fprintf(f, "%i", i);
			}
			first = 0;
		}
	}
	fprintf(f, "\n");
}

// -----------------------------------------------------------------------
int writer_keys(struct st *prog, FILE *f)
{
	struct st *t = prog->args;

	AADEBUG("==== KEYS writer ================================");
	fprintf(f, "addr: oct      bin                   keys\n");
	fprintf(f, "-------------------------------------------------------------------\n");
	while (t) {
		switch (t->type) {
			case N_INT:
				keys_print(f, t->ic, t->val);
				break;
			case N_BLOB:
				for (int i=0 ; i<t->size ; i++) {
					keys_print(f, t->ic+i, t->data[i]);
				}
				break;
			default:
				//fprintf(f, "@ 0x%04x : unresolved\n", t->ic);
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
			if (t->ic > icmax) icmax = t->ic;
			image[t->ic] = t->val;
			break;
		case N_BLOB:
			for (int i=0 ; i<t->size ; i++) {
				if (t->ic+i > icmax) icmax = t->ic+i;
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

	AADEBUG("==== RAW writer ================================");
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

	pos = icmax;
	while (pos >= 0) {
		image[pos] = htons(image[pos]);
		pos--;
	}

	if (icmax >= 0) {
		res = fwrite(image, 2, icmax+1, f);
		if (res < 0) {
			aaerror(NULL, "Write failed");
			return 1;
		}
	}

	return 0;
}

// vim: tabstop=4 autoindent
