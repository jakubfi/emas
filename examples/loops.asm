; EMAS assembler example
; simple loops

	.cpu	mera400

	.include cpu.inc

	uj	start

count:	.res	1

	.org	OS_START
start:

; ========================================================================
; loop over values in r1 using comparison and jump
loop1:
	lwt	r1, 0	; load 0 into r1
.l:	
	awt	r1, 1	; increase value in r1
	cw	r1, 10	; is it 10?
	jn	.l	; not 10, loop over

; ========================================================================
; loop over values in r1 using irb
loop2:
	lwt	r1, -10	; load -10 into r1
.l:
	irb	r1, .l	; increase r1 by 1, loop over if r1 !=0, otherwise exit the loop

; ========================================================================
; loop over values in r1 using drb
loop3:
	lwt	r1, 10	; load 10 into r1
.l:
	drb	r1, .l	; decrease r1 by 1, loop over if r1 !=0, otherwise exit the loop

; ========================================================================
; loop over values in memory location
loop4:
	lw	r1, -10		; load -10 into r1
	rw	r1, count	; store -10 in memory location
.l:
	ib	count		; increase cout by 1, skip next instruction if count == 0
	ujs	.l		; loop over

; ========================================================================
; stop the CPU
	hlt
