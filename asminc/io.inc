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

.const IO_CHAN	14	; channel number position in N argument of I/O instructions
.const IO_DEV	10	; device number position in N argument of I/O instructions

; general channel commands

.const IO_EXISTS		0b00000\4	; IN or OU, check if channel exists
.const IO_INT_MASK		0b00001\4	; OU, suspend interrupts to this CPU
.const IO_INT_MASK_OTHER	0b00010\4	; OU, suspend interrupts to the other CPU
.const IO_INT_ASSIGN		0b00011\4	; OU, assign device to this CPU
.const IO_INTSPEC		0b00001\4	; IN, get interrupt specification
.const IO_INT_GET_ALLOC		0b00011\4	; IN, get device interrupt allocation

; device-specific I/O commands

.const IO_CHAR_EXISTS	0b100\2	; IN, check if device exists
.const IO_CHAR_READ	0b101\2	; IN, read byte
.const IO_CHAR_RESET	0b100\2	; OU, reset device
.const IO_CHAR_DETACH	0b101\2	; OU, detach device (soft reset)
.const IO_CHAR_WRITE	0b110\2	; OU, write byte

; memory configuration field locations

.const MEM_CFG		1\15	; command is for memory module
.const MEM_PAGE		3	; page position in r argument of OU instruction
.const MEM_SEGMENT	15	; segment position in r argument of OU instruction
.const MEM_FRAME	10	; frame position in N argument of OU instruction
.const MEM_MODULE	14	; module position in N argument of OU instruction

.endif

; vim: tabstop=8 shiftwidth=8 autoindent syntax=emas
