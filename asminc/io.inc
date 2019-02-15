; Copyright (c) 2019 Jakub Filipowicz <jakubf@gmail.com>
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc.,
; 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

; MERA-400 I/O

.ifndef __IO_INC__
.const __IO_INC__ 1

; channel I/O

.const IO_CHAN	14	; bit position in N argument
.const IO_DEV	10	; bit position in N argument

; general channel I/O commands

.const IO_RESET		0b000\2 | 0b00\4	; IN, channel reset
.const IO_INTSPEC	0b000\2 | 0b01\4	; IN, get interrupt specification
.const IO_EXISTS	0b000\2 | 0b10\4	; IN, check if channel exists

; memory configuration field locations

.const MEM_CFG		1\15	; command is for memory module
.const MEM_PAGE		3	; page position (in r argument)
.const MEM_SEGMENT	15	; segment position (in r argument)
.const MEM_FRAME	10	; frame position (in N argument)
.const MEM_MODULE	14	; module position (in N argument)

.endif

; vim: tabstop=8 shiftwidth=8 autoindent syntax=emas
