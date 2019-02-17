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

; MERA-400 CPU

.ifndef __CPU_INC__
.const __CPU_INC__ 1

; general system addresses

.const INTV		0x40	; interrupt vectors
.const EXLV		0x60	; EXL procedure vector
.const STACKP		0x61	; stack pointer
.const SFPV		0x64	; software FP vectors
.const OS_START		0x70	; start of the useable OS memory space

.const INTV_SIZE	32	; there are 32 interrupts
.const SFPV_SIZE	12	; there are 12 FP commands

; SR field positions

.const SR_NB		15	; bit position of NB in SR register
.const SR_BS		11	; bit position of BS in SR register
.const SR_Q		10	; bit position of Q in SR register
.const SR_RM		9	; bit position of RM in SR register

; interrupt vectors

.const INTV_POWER	INTV + 0	; CPU power supply failure
.const INTV_PARITY	INTV + 1	; memory parity error
.const INTV_NOMEM	INTV + 2	; segmentation fault
.const INTV_CPU_H	INTV + 3	; interrupt from the other CPU, high
.const INTV_IFPOWER	INTV + 4	; interface power failure (from another module)
.const INTV_TIMER	INTV + 5	; timer interrupt
.const INTV_ILLEGAL	INTV + 6	; illegal instruction
.const INTV_DIV_OF	INTV + 7	; division overflow
.const INTV_FP_UF	INTV + 8	; floating point underflow
.const INTV_FP_OF	INTV + 9	; floating point overflow
.const INTV_FP_ERR	INTV + 10	; floating point error...
.const INTV_FP_DIV0	INTV_FP_ERR	; ...or floating point division by 0
.const INTV_UNUSED	INTV + 11	; unused interrupt
.const INTV_CH0		INTV + 12	; channel interrupts
.const INTV_CH1		INTV + 13
.const INTV_CH2		INTV + 14
.const INTV_CH3		INTV + 15
.const INTV_CH4		INTV + 16
.const INTV_CH5		INTV + 17
.const INTV_CH6		INTV + 18
.const INTV_CH7		INTV + 19
.const INTV_CH8		INTV + 20
.const INTV_CH9		INTV + 21
.const INTV_CH10	INTV + 22
.const INTV_CH11	INTV + 23
.const INTV_CH12	INTV + 24
.const INTV_CH13	INTV + 25
.const INTV_CH14	INTV + 26
.const INTV_CH15	INTV + 27
.const INTV_OPRQ	INTV + 28	; operator request
.const INTV_CPU_L	INTV + 29	; interrupt from the other CPU, low
.const INTV_SW_H	INTV + 30	; software interrupt, high
.const INTV_SW_L	INTV + 31	; software interrupt, low

; interrupt masks

.const IMASK_PARITY	1\0	; parity
.const IMASK_NOMEM	1\1	; no memory
.const IMASK_CPU_H	1\2	; cpu high
.const IMASK_IFPOWER	1\3	; interface power
.const IMASK_GROUP_H	1\4	; timer, illegal instruction, arithmetic, unused
.const IMASK_CH0_1	1\5	; channel 0-1
.const IMASK_CH2_3	1\6	; channel 2-3
.const IMASK_CH4_9	1\7	; channel 4-9
.const IMASK_CH10_15	1\8	; channel 10-15
.const IMASK_GROUP_L	1\9	; oprq, cpu low, software low, software low

.const IMASK_ALL_MEM	0xc000	; all memory interrupts
.const IMASK_ALL_CH	0x0780	; all channel interrupts
.const IMASK_ALL	0xffc0	; all interrupts
.const IMASK_NONE	0	; no interrupts

; 16-bit subset of RZ interrupt register accessible with 'ki' and 'fi' instructions

.const I_POWER		1\0
.const I_PARITY		1\1
.const I_NOMEM		1\2
.const I_CPU_H		1\3
.const I_IFPOWER	1\4
.const I_TIMER		1\5
.const I_ILLEGAL	1\6
.const I_DIV_OF		1\7
.const I_FP_UF		1\8
.const I_FP_OF		1\9
.const I_FP_ERR		1\10
.const I_DIV0		I_FP_ERR
.const I_EXTRA		1\11
.const I_OPRQ		1\12
.const I_CPU_L		1\13
.const I_SW_H		1\14
.const I_SW_L		1\15

.const I_ALL		0xffff
.const I_ALL_CPU	0x1004
.const I_ALL_ARITH	0x01e0
.const I_ALL_FP		0x00e0
.const I_ALL_SW		0x0003
.const I_MASKABLE	0x7fff
.const I_NONE		0

; software fp procedure vectors

.const SFPV_NRF0	SFPV + 0
.const SFPV_NRF1	SFPV + 1
.const SFPV_NRF2	SFPV + 2
.const SFPV_NRF3	SFPV + 3
.const SFPV_AD		SFPV + 4
.const SFPV_SD		SFPV + 5
.const SFPV_MW		SFPV + 6
.const SFPV_DW		SFPV + 7
.const SFPV_AF		SFPV + 8
.const SFPV_SF		SFPV + 9
.const SFPV_MF		SFPV + 10
.const SFPV_DF		SFPV + 11

.endif

; vim: tabstop=8 shiftwidth=8 autoindent syntax=emas
