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
#include <strings.h>
#include <ctype.h>

#include "dh.h"
#include "st.h"

// -----------------------------------------------------------------------
struct dh_table * dh_create(int size, int case_sens)
{
	struct dh_table *dh = malloc(sizeof(struct dh_table));
	if (!dh) {
		return NULL;
	}

	dh->slots = calloc(size, sizeof(struct dh_elem));
	if (!dh->slots) {
		free(dh);
		return NULL;
	}
	dh->size = size;
	if (case_sens) {
		dh->str_cmp = strcmp;
		dh->case_sens = 1;
	} else {
		dh->str_cmp = strcasecmp;
		dh->case_sens = 0;
	}

	return dh;
}

// -----------------------------------------------------------------------
unsigned dh_hash(struct dh_table *dh, char *str)
{
	unsigned v = 0;
	char *c = str;

	while (c && *c) {
		v = (dh->case_sens ? *c : tolower(*c)) + (v<<5) - v;
		c++;
	}

	return v % dh->size;
}

// -----------------------------------------------------------------------
struct dh_elem * dh_get(struct dh_table *dh, char *name)
{
	unsigned hash = dh_hash(dh, name);
	struct dh_elem *elem = dh->slots[hash];

	while (elem) {
		if (!(dh->str_cmp(name, elem->name))) {
			return elem;
		}
		elem = elem->next;
	}

	return NULL;
}

// -----------------------------------------------------------------------
struct dh_elem * dh_add(struct dh_table *dh, char *name, int type, int value, struct st *t)
{
	unsigned hash = dh_hash(dh, name);
	struct dh_elem *elem = dh->slots[hash];

	while (elem) {
		if (!dh->str_cmp(name, elem->name)) {
			return NULL;
		}
		elem = elem->next;
	}

	struct dh_elem *new_elem = malloc(sizeof(struct dh_elem));
	new_elem->name = strdup(name);
	new_elem->type = type;
	new_elem->value = value;
	new_elem->t = t;
	new_elem->next = dh->slots[hash];
	dh->slots[hash] = new_elem;

	return elem;
}

// -----------------------------------------------------------------------
int dh_delete(struct dh_table *dh, char *name)
{
	unsigned hash = dh_hash(dh, name);
	struct dh_elem *prev = NULL;
	struct dh_elem *elem = dh->slots[hash];

    while (elem) {
        if (!(dh->str_cmp(name, elem->name))) {
			if (prev) {
				prev->next = elem->next;
			} else {
				dh->slots[hash] = elem->next;
			}
			free(elem->name);
			st_drop(elem->t);
			free(elem);
            return 0;
        }
		prev = elem;
        elem = elem->next;
    }
	return -1;
}

// -----------------------------------------------------------------------
void dh_destroy(struct dh_table *dh)
{
	int i;
	struct dh_elem *elem;
	struct dh_elem *tmp;

	if (!dh) return;

	for(i=0 ; i<dh->size ; i++) {
		elem = dh->slots[i];
		while (elem) {
			tmp = elem->next;
			free(elem->name);
			if (elem->t) st_drop(elem->t);
			free(elem);
			elem = tmp;
		}
	}

	free(dh->slots);
	free(dh);
}

// -----------------------------------------------------------------------
void dh_dump_stats(struct dh_table *dh)
{
	int i;
	int elem_total = 0, collisions = 0, max_depth = 0;

	struct dh_elem *elem;

	if (!dh) return;

	unsigned *elem_slot = calloc(dh->size, sizeof(int));

	for(i=0 ; i<dh->size ; i++) {
		elem = dh->slots[i];
		int depth = 0;
		while (elem) {
			depth++;
			elem_total++;
			elem_slot[i]++;
			if (elem_slot[i] > 1) {
				collisions++;
			}
			elem = elem->next;
		}
		if (depth > max_depth) max_depth = depth;
	}

	printf("-----------------------------------\n");
	printf("      Slots: %d\n", dh->size);
	printf("   Elements: %d\n", elem_total);
	printf("  Max depth: %d\n", max_depth);
	printf(" Collisions: %d\n", collisions);
	printf("   Collided: \n");
	for(i=0 ; i<dh->size ; i++) {
		if (elem_slot[i] > 1) {
			printf(" %10d: %d\n", i, elem_slot[i]-1);
		}
	}

	free(elem_slot);
}

// -----------------------------------------------------------------------
int dh_sanity_check(struct dh_table *dh)
{
	int ret = -1;
	int i;
	struct dh_elem *elem;
	char *lcname, *ucname, *c;

	if (!dh) return -1;

	for(i=0 ; i<dh->size ; i++) {
		elem = dh->slots[i];
		while (elem) {
			if (dh->case_sens) {
				if (!dh_get(dh, elem->name)) {
					ret--;
					printf("Could not find: %s\n", elem->name);
				}
			} else {
				lcname = strdup(elem->name);
				ucname = strdup(elem->name);
				c = lcname;
				while (c && *c) {
					*c = tolower(*c);
					c++;
				}
				c = ucname;
				while (c && *c) {
					*c = toupper(*c);
					c++;
				}
				if (!dh_get(dh, ucname)) {
					ret--;
					printf("Could not find (uower case): %s\n", ucname);
				}
				if (!dh_get(dh, lcname)) {
					ret--;
					printf("Could not find (lower case): %s\n", lcname);
				}
				free(lcname);
				free(ucname);
			}
			elem = elem->next;
		}
	}
	return ret;
}

// vim: tabstop=4 autoindent
