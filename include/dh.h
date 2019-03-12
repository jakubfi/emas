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

#ifndef DH_H
#define DH_H

#include "st.h"

typedef int (*str_cmp_fun)(const char *s1, const char *s2);

struct dh_elem {
	char *name;
	int type;
	int value;
	struct st *t;
	struct dh_elem *next;
	int being_evaluated;
};

struct dh_table {
	int size;
	struct dh_elem **slots;
	int case_sens;
	str_cmp_fun str_cmp;
};

struct dh_table * dh_create(int size, int case_sens);
struct dh_elem * dh_get(struct dh_table *dh, char *name);
struct dh_elem * dh_add(struct dh_table *dh, char *name, int type, int value, struct st *t);
#define dh_addv(dh, name, type, value) dh_add(dh, name, type, value, NULL)
#define dh_addt(dh, name, type, t) dh_add(dh, name, type, 0, t)
int dh_delete(struct dh_table *dh, char *name);
void dh_destroy(struct dh_table *dh);
void dh_dump_stats(struct dh_table *dh);

#endif

// vim: tabstop=4 autoindent
