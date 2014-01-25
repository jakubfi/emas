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

#ifndef SXTREE_H
#define SXTREE_H

#include <inttypes.h>

#define DATA_MAX 256

struct st {
	int type;

	char *str;
	int val;
	double flo;
	uint16_t *data;

	int loc_line;
	int loc_col;
	char *loc_file;

	int ic;
	int relative;

	struct st *args;
	struct st *next;
	struct st *prev;
	struct st *last;
};

struct st * st_copy(struct st *t);
void st_drop(struct st *stx);
struct st * st_int(int type, int val);
struct st * st_float(int type, double flo);
struct st * st_str(int type, char *str);
struct st * st_arg(int type, ...);
struct st * st_arg_app(struct st *stx, struct st *app_first);
struct st * st_app(struct st *t1, struct st *t2);

#endif

// vim: tabstop=4 autoindent
