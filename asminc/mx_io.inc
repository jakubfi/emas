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

; MULTIX I/O commands

.ifndef __MX_IO_INC__
.const __MX_IO_INC__ 1

.include io.inc

; default MULTIX I/O channel

.const MX_DEFAULT_CH	1\IO_CHAN
.const MX_DEFAULT_IV	IV_CH1

; MULTIX I/O commands

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

.endif

; vim: tabstop=8 shiftwidth=8 autoindent syntax=emas
