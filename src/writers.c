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

#include "prog.h"
#include "st.h"

#define MEM_MAX 64 * 1024

uint16_t image[MEM_MAX];
uint16_t relocs[MEM_MAX];
int rel_count;
int icmax;

// -----------------------------------------------------------------------
// convert an integer to formatted string with its binary representation
char * int2binf(char *format, uint64_t value, int size)
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
int writer_debug(struct st *prog, char *ifile, char *ofile)
{
	struct st *t = prog->args;
	char *bin;

	printf("Input file: '%s'\n", ifile);

	while (t) {
		switch (t->type) {
			case N_INT:
				bin = int2binf("... ... . ... ... ...", t->val, 16);
				printf("@ 0x%04x : 0x%04x  /  %s  /  %i\n", t->ic, (uint16_t) t->val, bin, t->val);
				free(bin);
				break;
			case N_BLOB:
				for (int i=0 ; i<t->size ; i++) {
					char *bin = int2binf("... ... . ... ... ...", t->data[i], 16);
					printf("@ 0x%04x : 0x%04x  /  %s  /  %i\n", t->ic+i, (uint16_t) t->data[i], bin, t->data[i]);
					free(bin);
				}
				break;
			case N_NONE:
				printf("@ 0x%04x : (none, type %d)\n", t->ic, t->type);
				break;
			default:
				printf("@ 0x%04x : unresolved\n", t->ic);
				break;
		}
		t = t->next;
	}
	return 0;
}

// -----------------------------------------------------------------------
void img_put(struct st *t)
{
	switch (t->type) {
		case N_INT:
			if (t->ic > icmax) icmax = t->ic;
			image[t->ic] = htons(t->val);
			if (t->relative) {
				relocs[rel_count] = t->ic;
				rel_count++;
			}
			break;
		case N_BLOB:
			for (int i=0 ; i<t->size ; i++) {
				if (t->ic+i > icmax) icmax = t->ic+i;
				image[t->ic+i] = htons(t->data[i]);
			}
			break;
	}
}

// -----------------------------------------------------------------------
int img_fill(struct st *prog)
{
	struct st *t;

	assert(prog);

	t = prog->args;
	while (t) {
		switch (t->type) {
			case N_INT:
			case N_BLOB:
				img_put(t);
				break;
			case N_NONE:
				break;
			case N_PLUS:
			case N_MINUS:
			case N_UMINUS:
			default:
				return 1;
		}
		t = t->next;
	}

	return 0;
}

// -----------------------------------------------------------------------
int writer_raw(struct st *prog, char *ifile, char *ofile)
{
	int res;
	FILE *f = NULL;
	char *output;

	if (img_fill(prog)) {
		return 1;
	}

	if (ofile) {
		output = strdup(ofile);
	} else {
		output = strdup("out.bin");
	}

	f = fopen(output, "w");
	if (!f) {
		return 1;
	}

	res = fwrite(image, icmax+1, 2, f);
	if (res < 0) {
		fclose(f);
		return 1;
	}

	fclose(f);
	return 0;
}

// -----------------------------------------------------------------------
int writer_emelf(struct st *prog, struct dh_table *symbols, char *ifile, char *ofile)
{
	int ret;

ret = 1; goto cleanup;

	if (img_fill(prog)) {
		ret = 1;
		goto cleanup;
	}

	ret = 0;

cleanup:
	printf("Output type 'emelf' is yet to be implemented.\n");
	return ret;
}


// vim: tabstop=4 autoindent
