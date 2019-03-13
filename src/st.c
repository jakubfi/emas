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
#include <stdarg.h>
#include <string.h>

#include "st.h"
#include "parser.h"

extern int ic;

// -----------------------------------------------------------------------
struct st * st_new(int type, int val, double flo, char *str, struct st *args)
{
	struct st *sx;

	sx = calloc(1, sizeof(struct st));
	if (!sx) {
		return NULL;
	}

	sx->type = type;
	sx->val = val;
	sx->flo = flo;
	sx->args = args;
	if (str) {
		sx->str = strdup(str);
		if (!sx->str) {
			free(sx);
			return NULL;
		}
	}

	sx->ic = -1;
	sx->size = 0;
	sx->flags = ST_NONE;

	if (yylloc.filename) {
		sx->loc_file = yylloc.filename;
	} else {
		sx->loc_file = NULL;
	}
	sx->loc_line = yylloc.first_line;
	sx->loc_col = yylloc.first_column;

	return sx;
}

// -----------------------------------------------------------------------
struct st * st_copy(struct st *t)
{
	if (!t) return NULL;

	struct st *sx = st_new(t->type, t->val, t->flo, t->str, NULL);

	return sx;
}

// -----------------------------------------------------------------------
void st_drop(struct st *stx)
{
	if (!stx) return;
	free(stx->str);
	free(stx->data);
	st_drop(stx->next);
	st_drop(stx->args);
	free(stx);
}

// -----------------------------------------------------------------------
struct st * st_int(int type, int val)
{
	return st_new(type, val, 0, NULL, NULL);
}

// -----------------------------------------------------------------------
struct st * st_float(int type, double flo)
{
	return st_new(type, 0, flo, NULL, NULL);
}

// -----------------------------------------------------------------------
struct st * st_str(int type, char *str)
{
	return st_new(type, 0, 0, str, NULL);
}

// -----------------------------------------------------------------------
struct st * st_arg(int type, ...)
{
	va_list ap;
	struct st *stx, *s;

	stx = st_new(type, 0, 0, NULL, NULL);
	if (!stx) return NULL;

	va_start(ap, type);

	s = va_arg(ap, struct st*);
	while (s) {
		st_arg_app(stx, s);
		s = va_arg(ap, struct st*);
	}
	va_end(ap);

	return stx;
}

// -----------------------------------------------------------------------
struct st * st_arg_app(struct st *t1, struct st *arg)
{
	struct st *newlast;
	if (t1 && arg) {
		newlast = arg;
		while (newlast->next) {
			newlast = newlast->next;
		}
		if (t1->last) {
			t1->last->next = arg;
		} else {
			t1->args = arg;
		}
		t1->last = newlast;
	}
	return t1;

}

// -----------------------------------------------------------------------
struct st * st_app(struct st *t1, struct st *t2)
{
	struct st *t;

	if (!t1) return t2;
	if (!t2) return t1;

	t = t1;
	while (t->next) {
		t = t->next;
	}
	t->next = t2;
	t2->prev = t;

	return t1;
}

// vim: tabstop=4 autoindent
