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
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "parser.h"
#include "st.h"

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
int writer_debug(struct st *prog)
{
	struct st *t = prog->args;
	char *bin;

	while (t) {
		switch (t->type) {
			case INT:
				bin = int2binf("... ... . ... ... ...", t->val, 16);
				printf("@ 0x%04x : 0x%04x  /  %s  /  %i\n", t->ic, t->val, bin, t->val);
				free(bin);
				break;
			case BLOB:
				for (int i=0 ; i<t->val ; i++) {
					char *bin = int2binf("... ... . ... ... ...", t->data[i], 16);
					printf("@ 0x%04x : 0x%04x  /  %s  /  %i\n", t->ic+i, t->data[i], bin, t->data[i]);
					free(bin);
				}
				break;
			case NONE:
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
int writer_raw(struct st *prog)
{
	struct st *t = prog->args;
	char *bin;
	int icmax = 0;

	uint16_t image[64 * 2 * 1024];

	while (t) {
		switch (t->type) {
			case INT:
				if (t->ic > icmax) icmax = t->ic;
				image[t->ic] = htons(t->val);
				bin = int2binf("... ... . ... ... ...", t->val, 16);
				printf("@ 0x%04x (0x%04x) : 0x%04x  /  %s  /  %i\n", t->ic, t->ic*2, t->val, bin, t->val);
				free(bin);
				break;
			case BLOB:
				for (int i=0 ; i<t->val ; i++) {
					if (t->ic+i > icmax) icmax = t->ic+i;
					image[t->ic+i] = htons(t->data[i]);
					char *bin = int2binf("... ... . ... ... ...", t->data[i], 16);
					printf("@ 0x%04x (0x%04x) : 0x%04x  /  %s  /  %i\n", t->ic+i, (t->ic+i)*2, t->data[i], bin, t->data[i]);
					free(bin);
				}
				break;
			case NONE:
				//printf("@ 0x%04x : (none, type %d)\n", t->ic, t->type);
				break;
			default:
				//printf("@ 0x%04x : unresolved\n", t->ic);
				break;
		}
		t = t->next;
	}

	FILE *f = fopen("out.bin", "w");
	fwrite(image, icmax+1, 2, f);
	fclose(f);

	return 0;
}


// vim: tabstop=4 autoindent
