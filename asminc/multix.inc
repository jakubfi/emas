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

; MULTIX peripherial processor

.ifndef __MX_IO__
.const __MX_IO__ 1

.include io.inc

; I/O commands

.const MX_IO_RESET	IO_RESET	; IN, MULTIX reset
.const MX_IO_INTSPEC	IO_INTSPEC	; IN, get interrupt specification
.const MX_IO_EXISTS	IO_EXISTS	; IN, check if MULTIX exists
.const MX_IO_REQUEUE	0b001\2		; IN, requeue interrupt
.const MX_IO_TEST	0b001\2		; OU, run MULTIX test
.const MX_IO_SETCFG	0b101\2		; OU, set configuration
.const MX_IO_ATTACH	0b010\2		; OU, attach line
.const MX_IO_DETACH	0b010\2		; IN, detach line
.const MX_IO_STATUS	0b011\2		; OU, get line status
.const MX_IO_TRANSMIT	0b100\2		; OU, start transmission
.const MX_IO_ABORT	0b011\2		; IN, abort transmission

; interrupt specifications

.const MX_INIEA	0\7	; interrupt no longer valid
.const MX_INSKA	1\7	; MULTIX initialization failed
.const MX_IWYZE	2\7	; MULTIX initialization OK
.const MX_IWYTE	3\7	; MULTIX test finished
.const MX_INKON	4\7	; 'set configuration' rejected
.const MX_IUKON	5\7	; 'set configuration' finished successfully
.const MX_INKOT	6\7	; 'set configuration' unsuccessfull
.const MX_ISTRE	7\7	; 'report status' OK
.const MX_INSTR	8\7	; 'report status' rejected
.const MX_INKST	9\7	; 'report status' for anon-existent line
.const MX_IDOLI	10\7	; 'attach line' OK
.const MX_INDOL	11\7	; 'attach line' rejected
.const MX_INKDO	12\7	; 'attach line' for a non-existent line
.const MX_IETRA	13\7	; 'transmit' OK
.const MX_INTRA	14\7	; 'transmit' rejected
.const MX_INKTR	15\7	; 'transmit' for a non-existent line
.const MX_ITRER	16\7	; 'transmit' finished with error
.const MX_IRQ17 17\7	; (unused)
.const MX_IRQ18 18\7	; (unused)
.const MX_ITRAB	19\7	; 'transmit' cancelled as requested by 'cancel transmission'
.const MX_IABTR	20\7	; 'cancel transmission' OK
.const MX_INABT	21\7	; 'cancel transmission' while no transmission
.const MX_INKAB	22\7	; 'cancel transmission' for a non-existent line
.const MX_IODLI	23\7	; 'detach line' OK
.const MX_INODL	24\7	; 'detach line' for a line with ongoing transmission
.const MX_INKOD	25\7	; 'detach line' for non existent line
.const MX_IRQ26 26\7	; (unused)
.const MX_IRQ27 27\7	; (unused)
.const MX_IRQ28 28\7	; (unused)
.const MX_IRQ29 29\7	; (unused)
.const MX_IRQ30 30\7	; (unused)
.const MX_IRQ31 31\7	; (unused)
.const MX_INPAO	32\7	; memory access error
.const MX_IPARE	33\7	; memory parity error
.const MX_IOPRU	34\7	; operator request
.const MX_IEPS0	35\7	; unknown control command, code=0
.const MX_IEPS6	36\7	; unknown control command, code=6
.const MX_IEPS7	37\7	; unknown control command, code=7
.const MX_IEPS8	38\7	; unknown control command, code=8
.const MX_IEPSC	39\7	; unknown control command, code=C
.const MX_IEPSD	40\7	; unknown control command, code=D
.const MX_IEPSE	41\7	; unknown control command, code=E
.const MX_IEPSF	42\7	; unknown control command, code=F

; line state constants

.const MX_LSTATE_NONE		0
.const MX_LSTATE_SEND_START	1\15	; send started
.const MX_LSTATE_SEND_RUN	1\14	; send running
.const MX_LSTATE_RECV_START	1\13	; receive started
.const MX_LSTATE_RECV_RUN	1\12	; receive running
.const MX_LSTATE_CAN_STOP	1\11	; stop after CAN (protocol 5)
.const MX_LSTATE_STOP_CHAR	1\10	; stop character received
.const MX_LSTATE_PARITY_ERR	1\9	; parity error
.const MX_LSTATE_OPRQ		1\8	; OPRQ
.const MX_LSTATE_ATTACHED	1\7	; line attached
.const MX_LSTATE_TRANS		1\6	; transmission active
.const MX_LSTATE_5		1\5	; (unused)
.const MX_LSTATE_4		1\4	; (unused)
.const MX_LSTATE_TASK_XOFF	1\3	; task suspended due to XOFF
.const MX_LSTATE_TRANS_XOFF	1\2	; transmission suspended due to XOFF
.const MX_LSTATE_TRANS_LAST	1\1	; transmission of a last fragment
.const MX_LSTATE_0		1\0	; (unused)

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

.const MX_LDIR_NONE		0\2
.const MX_LDIR_ERR1		1\2
.const MX_LDIR_IN		2\2	; input
.const MX_LDIR_ERR3		3\2
.const MX_LDIR_OUT		4\2	; output
.const MX_LDIR_ERR5		5\2
.const MX_LDIR_HD		6\2	; half-duplex
.const MX_LDIR_FD		7\2	; full-duplex

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
