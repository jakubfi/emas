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

struct st {
	int type;			// Node type (not token type!)
						// Node may hold:
	char *str;			//  * string
	int64_t val;		//  * integer value
	double flo;			//  * floating point value
	uint16_t *data;		//  * unsigned 16-bit blob (rendered only during evaluation)
						//  * list of arguments:
	struct st *args;	//     * list head
	struct st *next;	//     * next argument
	struct st *prev;	//     * previous argument
	struct st *last;	//     * list tail
						// Also, we store location of a node related token in source file (to reference errors)
	char *loc_file;		//  * source file (pointer to a file dictionary)
	int loc_line;		//  * source line
	int loc_col;		//  * source column
						// Upon evaluation, nodes get additional properties:
	int ic;				//  * IC at which node is to be rendered
	int size;			//  * size of data that node actually holds
	int flags;			//  * set if node holds object-relative value
};

enum st_flags {
	ST_NONE		= 0,
	ST_RELATIVE	= 1 << 0,
};

struct st * st_copy(struct st *t);
void st_drop(struct st *stx);
struct st * st_int(int type, int64_t val);
struct st * st_float(int type, double flo);
struct st * st_str(int type, char *str);
struct st * st_strval(int type, char *str, int val);
struct st * st_arg(int type, ...);
struct st * st_arg_app(struct st *stx, struct st *app_first);
struct st * st_app(struct st *t1, struct st *t2);

#endif

// vim: tabstop=4 autoindent
