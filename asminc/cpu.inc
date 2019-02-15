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

; interrupt numbers

.const INT_POWER	0	; power loss
.const INT_PARITY	1	; memory parity error
.const INT_NOMEM	2	; no memory (segmentation fault)
.const INT_CPU_H	3	; other CPU high interupt
.const INT_IFPOWER	4	; interface power loss
.const INT_TIMER	5	; timer
.const INT_ILLEGAL	6	; illegal instruction
.const INT_DIV_OF	7	; division overflow
.const INT_FP_UF	8	; floating point underflow
.const INT_FP_OF	9	; floating point overflow
.const INT_FP_ERR	10	; floating point error...
.const INT_DIV0		INT_FP_ERR ; ...or division by zero
.const INT_UNUSED	11	; unused interrupt
.const INT_CH0		12	; channel 0
.const INT_CH1		13	; channel 1
.const INT_CH2		14	; channel 2
.const INT_CH3		15	; channel 3
.const INT_CH4		16	; channel 4
.const INT_CH5		17	; channel 5
.const INT_CH6		18	; channel 6
.const INT_CH7		19	; channel 7
.const INT_CH8		20	; channel 8
.const INT_CH9		21	; channel 9
.const INT_CH10		22	; channel 10
.const INT_CH11		23	; channel 11
.const INT_CH12		24	; channel 12
.const INT_CH13		25	; channel 13
.const INT_CH14		26	; channel 14
.const INT_CH15		27	; channel 15
.const INT_OPRQ		28	; operator request
.const INT_CPU_L	29	; other CPU low interrupt
.const INT_SW_H		30	; software high interrupt
.const INT_SW_L		31	; software low interrupt

; interrupt vectors

.const INTV_POWER	INTV + INT_POWER
.const INTV_PARITY	INTV + INT_PARITY
.const INTV_NOMEM	INTV + INT_NOMEM
.const INTV_CPU_H	INTV + INT_CPU_H
.const INTV_IFPOWER	INTV + INT_IFPOWER
.const INTV_TIMER	INTV + INT_TIMER
.const INTV_ILLEGAL	INTV + INT_ILLEGAL
.const INTV_DIV_OF	INTV + INT_DIV_OF
.const INTV_FP_UF	INTV + INT_FP_UF
.const INTV_FP_OF	INTV + INT_FP_OF
.const INTV_FP_ERR	INTV + INT_FP_ERR
.const INTV_FP_DIV0	INTV_FP_ERR
.const INTV_UNUSED	INTV + INT_UNUSED
.const INTV_CH0		INTV + INT_CH0
.const INTV_CH1		INTV + INT_CH1
.const INTV_CH2		INTV + INT_CH2
.const INTV_CH3		INTV + INT_CH3
.const INTV_CH4		INTV + INT_CH4
.const INTV_CH5		INTV + INT_CH5
.const INTV_CH6		INTV + INT_CH6
.const INTV_CH7		INTV + INT_CH7
.const INTV_CH8		INTV + INT_CH8
.const INTV_CH9		INTV + INT_CH9
.const INTV_CH10	INTV + INT_CH10
.const INTV_CH11	INTV + INT_CH11
.const INTV_CH12	INTV + INT_CH12
.const INTV_CH13	INTV + INT_CH13
.const INTV_CH14	INTV + INT_CH14
.const INTV_CH15	INTV + INT_CH15
.const INTV_OPRQ	INTV + INT_OPRQ
.const INTV_CPU_L	INTV + INT_CPU_L
.const INTV_SW_H	INTV + INT_SW_H
.const INTV_SW_L	INTV + INT_SW_L

; interrupt masks

.const IMASK_PARITY	0b1000000000000000	; parity
.const IMASK_NOMEM	0b0100000000000000	; no memory
.const IMASK_CPU_H	0b0010000000000000	; cpu high
.const IMASK_IFPOWER	0b0001000000000000	; interface power
.const IMASK_GROUP_H	0b0000100000000000	; timer, illegal instruction, arithmetic, unused
.const IMASK_CH0_1	0b0000010000000000	; channel 0-1
.const IMASK_CH2_3	0b0000001000000000	; channel 2-3
.const IMASK_CH4_9	0b0000000100000000	; channel 4-9
.const IMASK_CH10_15	0b0000000010000000	; channel 10-15
.const IMASK_GROUP_L	0b0000000001000000	; oprq, cpu low, software low, software low

.const IMASK_ALL_MEM	0b1100000000000000	; all memory interrupts
.const IMASK_ALL_CH	0b0000011110000000	; all channel interrupts
.const IMASK_ALL	0b1111111111000000	; all interrupts
.const IMASK_NONE	0b0000000000000000	; no interrupts

; 16-bit subset of interrupts accessible by instructions 'ki' and 'fi'

.const	I16_POWER	0b1000000000000000
.const	I16_PARITY	0b0100000000000000
.const	I16_NOMEM	0b0010000000000000
.const	I16_CPU_H	0b0001000000000000
.const	I16_IFPOWER	0b0000100000000000
.const	I16_TIMER	0b0000010000000000
.const	I16_ILLEGAL	0b0000001000000000
.const	I16_DIV_OF	0b0000000100000000
.const	I16_FP_UF	0b0000000010000000
.const	I16_FP_OF	0b0000000001000000
.const	I16_FP_ERR	0b0000000000100000
.const	I16_DIV0	I_FP_ERR
.const	I16_EXTRA	0b0000000000010000
.const	I16_OPRQ	0b0000000000001000
.const	I16_CPU_L	0b0000000000000100
.const	I16_SW_H	0b0000000000000010
.const	I16_SW_L	0b0000000000000001
.const	I16_ALL		0b1111111111111111
.const	I16_NONE	0b0000000000000000

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
