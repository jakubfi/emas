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

; channel numbers for I/O commands

.const IO_CH0	0\14
.const IO_CH1	1\14
.const IO_CH2	2\14
.const IO_CH3	3\14
.const IO_CH4	4\14
.const IO_CH5	5\14
.const IO_CH6	6\14
.const IO_CH7	7\14
.const IO_CH8	8\14
.const IO_CH9	9\14
.const IO_CH10	10\14
.const IO_CH11	11\14
.const IO_CH12	12\14
.const IO_CH13	13\14
.const IO_CH14	14\14
.const IO_CH15	15\14

; general channel I/O commands

.const IO_RESET		0b000\2 | 0b00\4	; IN, channel reset
.const IO_INTSPEC	0b000\2 | 0b01\4	; IN, get interrupt specification
.const IO_EXISTS	0b000\2 | 0b10\4	; IN, check if channel exists

; memory configuration bit

.const MEM_CFG		1\15	; command for memory module

.endif

; vim: tabstop=8 shiftwidth=8 autoindent syntax=emas
