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

; MULTIX 'setcfg' command constants

.ifndef __MX_SETCFG_INC__
.const __MX_SETCFG_INC__ 1

; 'set configuration' error codes

.const MX_SCERR_CONFSET		0\7	; configuration already set
.const MX_SCERR_NUM		1\7	; wrong number of logical or physical lines
.const MX_SCERR_TYPE		2\7	; wrong device type in physical line description
.const MX_SCERR_DIR		3\7	; wrong transmission direction
.const MX_SCERR_INCOMPLETE	4\7	; incomplete physical line descruption
.const MX_SCERR_PROTO		5\7	; missing protocol
.const MX_SCERR_UNUSED		6\7	; physical line unused
.const MX_SCERR_DIR_MISMATCH	7\7	; device vs. protocol transmission direction mismatch
.const MX_SCERR_USED		8\7	; physical line already used
.const MX_SCERR_MEM		9\7	; memory exhausted
.const MX_SCERR_TYPE_MISMATCH	10\7	; protocol type vs. physical line type mismatch
.const MX_SCERR_PARAMS		11\7	; wrong protocol configuration

; physical line direction

.const MX_LDIR_NONE		0b000\2
.const MX_LDIR_ERR1		0b001\2
.const MX_LDIR_IN		0b010\2	; input
.const MX_LDIR_ERR3		0b011\2
.const MX_LDIR_OUT		0b100\2	; output
.const MX_LDIR_ERR5		0b101\2
.const MX_LDIR_HD		0b110\2	; half-duplex
.const MX_LDIR_FD		0b111\2	; full-duplex

; device type

.const MX_LTYPE_USARTA		0\7	; async serial
.const MX_LTYPE_8255		1\7	; parallel
.const MX_LTYPE_USARTS		2\7	; sync serial
.const MX_LTYPE_WINCH		3\7	; winchester hard disk drive
.const MX_LTYPE_TAPE		4\7	; magnetic tape drive
.const MX_LTYPE_FLOPPY		5\7	; floppy disk drive
.const MX_LTYPE_ERR		6\7

; transmission protocols

.const MX_LPROTO_PUNCHRD	0\7	; punched paper tape reader
.const MX_LPROTO_PUNCHER	1\7	; paper tape puncher
.const MX_LPROTO_MONITOR	2\7	; terminal
.const MX_LPROTO_SOM_PUNCHRD	3\7	; punched paper tape reader (SOM-3)
.const MX_LPROTO_SOM_PUNCHER	4\7	; paper tape puncher (SOM-3)
.const MX_LPROTO_SOM_MONITOR	5\7	; terminal (SOM-3)
.const MX_LPROTO_WINCH		6\7	; winchester hard disk drive
.const MX_LPROTO_TAPE		7\7	; magnetic tape drive
.const MX_LPROTO_FLOPPY		8\7	; floppy disk drive
.const MX_LPROTO_TTY_ITWL	9\7	; ITWL tty
.const MX_LPROTO_ERR		10\7

; physical line usage

.const MX_LINE_USED		1\3
.const MX_LINE_UNUSED		0\3

; floppy drive types

.const MX_FLOPPY_SD		0\7	; 40 cylinders, SD
.const MX_FLOPPY_DD		1\7	; 80 cylinders, DD
.const MX_FLOPPY_HD		2\7	; 80 cylinders, HD
.const MX_FLOPPY_ERR		3\7

; format protection (floppy, winchester)

.const MX_NO_FORMAT_PROTECT	0\15
.const MX_FORMAT_PROTECT	1\15

; winchester address format

.const MX_SHORT_DISK_ADDR	0\4
.const MX_LONG_DISK_ADDR	1\4

.endif

; vim: tabstop=8 shiftwidth=8 autoindent syntax=emas
