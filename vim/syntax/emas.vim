" Vim syntax file
" Language:	EMAS assembler (MERA-400)
" Orig Author:	Jakub Filipowicz <jakubf@gmail.com>
" Maintainer:	Jakub Filipowicz <jakubf@gmail.com>
" Last Change:	Date: 2014/03/27
" Revision:	1.0

" For version 5.x: Clear all syntax items
" For version 6.x: Quit when a syntax file was already loaded
if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

let s:cpo_save = &cpo
set cpo&vim

syn case ignore

" comments
syn match emasComment			";.*"

" registers
syn keyword emasRegister		r0 r1 r2 r3 r4 r5 r6 r7

" numbers
syn match emasDecimal			"[-+]\?[0-9]\+"
syn match emasBinary			"0b[0-1]\+"
syn match emasOctal				"0[0-7]\+"
syn match emasHexadecimal		"0x[0-9a-fA-F]\+"
syn match emasFloat				"[-+]\?\d\+\.\(\d*\(E[-+]\?\d\+\)\?\)\?"
syn match emasFlags				"?[ZMCVLEGXY]\+"

syn match emasIdentifier		"[a-z_][a-z0-9_]*"
syn match emasLabel				"[a-z_][a-z0-9_]*:"

" opcodes
syn keyword emasOpcode			LW TW LS RI RW PW RJ IS BB BM BS BC BN OU IN
syn keyword emasOpcode			AD SD MW DW AF SF MF DF
syn keyword emasOpcode			AW AC SW CW OR OM NR NM ER EM XR XM CL LB RB CB
syn keyword emasOpcode			AWT TRB IRB DRB CWT LWT LWS RWS
syn keyword emasOpcode			UJS JLS JES JGS JVS JXS JYS JCS
syn keyword emasOpcode			BLC EXL BRC NRF
syn keyword emasOpcode			RIC ZLB SXU NGA SLZ SLY SLX SRY NGL RPC SHC RKY ZRB SXL NGC SVZ SVY SVX SRX SRZ LPC
syn keyword emasOpcode			HLT MCL SIT SIL SIU CIT GIU LIP GIL
syn keyword emasOpcode			UJ JL JE JG JZ JM JN LJ
syn keyword emasOpcode			LD LF LA LL TD TF TA TL
syn keyword emasOpcode			RD RF RA RL PD PF PA PL
syn keyword emasOpcode			MB IM KI FI SP MD RZ IB
syn keyword emasOpcode			CRON SINT SIND

" directives
syn match emasDirective			"\.cpu\>"
syn match emasDirective			"\.equ\>"
syn match emasDirective			"\.const\>"
syn match emasDirective			"\.lbyte\>"
syn match emasDirective			"\.rbyte\>"
syn match emasDirective			"\.word\>"
syn match emasDirective			"\.dword\>"
syn match emasDirective			"\.float\>"
syn match emasDirective			"\.ascii\>"
syn match emasDirective			"\.asciiz\>"
syn match emasDirective			"\.r40\>"
syn match emasDirective			"\.res\>"
syn match emasDirective			"\.org\>"
syn match emasDirective			"\.entry\>"
syn match emasDirective			"\.global\>"

" preprocessor
syn match emasPreproc			"\.include\>"
syn match emasPreproc			"\.file\>"
syn match emasPreproc			"\.line\>"

" strings
syn region emasString			start=+'+ end=+'+ oneline
syn region emasString			start=+"+ end=+"+ oneline


" Define the default highlighting.
" For version 5.7 and earlier: only when not done already
" For version 5.8 and later: only when an item doesn't have highlighting yet
if version >= 508 || !exists("did_emas_syntax_inits")
  if version < 508
    let did_emas_syntax_inits = 1
    command -nargs=+ HiLink hi link <args>
  else
    command -nargs=+ HiLink hi def link <args>
  endif

	HiLink emasComment		Comment

	HiLink emasRegister		Special

	HiLink emasHexadecimal	Number
	HiLink emasDecimal		Number
	HiLink emasOctal		Number
	HiLink emasBinary		Number
	HiLink emasFloat		Number
	HiLink emasFlags		Number

	HiLink emasIdentifier	Identifier
	HiLink emasLabel		Type
	HiLink emasOpcode		Statement
	HiLink emasDirective	Statement
	HiLink emasPreproc		PreProc

	HiLink emasString		String

  syntax sync minlines=50

  delcommand HiLink
endif

let b:current_syntax = "emas"

let &cpo = s:cpo_save
unlet s:cpo_save

" vim: ts=4
