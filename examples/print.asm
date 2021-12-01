; EMAS assembler example
; print a string on a terminal connected as device no. 0 in character channel 15

	.cpu	mera400

	.include cpu.inc
	.include io.inc

	uj	start

str:	.asciiz	"Hello world!\r\n"		; string to print
	.const	CH	15			; channel number
	.const	DEV	0			; device number
	.const	TERM	CH\IO_CHAN | DEV\IO_DEV ; terminal address

	.org	OS_START
start:

; ========================================================================

	lwt	r1, 0			; clear r1 register
	lw	r3, str<<1		; r3 stores current position in the string as a byte address (which has to be shifted left)
	lw	r2, TERM | KZ_CMD_DEV_WRITE	; load terminal address and 'write' command to r2

loop:
	lb	r1, r3			; load one character into r1
	cw	r1, 0			; is it the end of the string? (string is 0-terminated)
	jes	end			; end the program if so, otherwise...
	awt	r3, 1			; ...move to the next byte
retry:
	ou	r1, r2			; send the command stored in r2 and the byte stored in r1 to the device
	.word	.no, .en, .ok, .pe	; each I/O command may end in one of four possible states:
.no:	hlt	1			; no response from the I/O channel - stop the CPU
.en:	ujs	retry			; I/O channel busy - retry the transmission
.ok:	ujs	loop			; transmission OK, loop over and print the next character
.pe:	hlt	3			; parity error - stop the CPU

	; stop the CPU
end:	hlt
