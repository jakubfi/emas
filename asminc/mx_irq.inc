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

; MULTIX interrupts

.ifndef __MX_IRQ_INC__
.const __MX_IRQ_INC__ 1

.const MX_INIEA	0\7
.const MX_INSKA	1\7
.const MX_IWYZE	2\7
.const MX_IWYTE	3\7
.const MX_INKON	4\7
.const MX_IUKON	5\7
.const MX_INKOT	6\7
.const MX_ISTRE	7\7
.const MX_INSTR	8\7
.const MX_INKST	9\7
.const MX_IDOLI	10\7
.const MX_INDOL	11\7
.const MX_INKDO	12\7
.const MX_IETRA	13\7
.const MX_INTRA	14\7
.const MX_INKTR	15\7
.const MX_ITRER	16\7
.const MX_ITRAB	19\7
.const MX_IABTR	20\7
.const MX_INABT	21\7
.const MX_INKAB	22\7
.const MX_IODLI	23\7
.const MX_INODL	24\7
.const MX_INKOD	25\7
.const MX_INPAO	32\7
.const MX_IPARE	33\7
.const MX_IOPRU	34\7
.const MX_IEPS0	35\7
.const MX_IEPS6	36\7
.const MX_IEPS7	37\7
.const MX_IEPS8	38\7
.const MX_IEPSC	39\7
.const MX_IEPSD	40\7
.const MX_IEPSE	41\7
.const MX_IEPSF	42\7

.endif

; vim: tabstop=8 shiftwidth=8 autoindent syntax=emas
