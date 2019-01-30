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

; MEGA memory

.ifndef __MEGA_INC__
.const __MEGA_INC__ 1

; MEGA memory configuration commands

.const MEGA_ALLOC_DONE	1\0		; allocation finished
.const MEGA_EPROM_HIDE	1\1 | 0\2	; hide EPROM segment @ 0xf000
.const MEGA_EPROM_SHOW	0\1 | 1\2	; show EPROM segment @ 0xf000
.const MEGA_FREE	1\5 | 1\6	; free page
.const MEGA_ALLOC	0\5 | 1\6	; allocate page

.endif

; vim: tabstop=8 shiftwidth=8 autoindent syntax=emas
