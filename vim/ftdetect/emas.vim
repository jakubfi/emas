au! BufNewFile,BufRead *.[Aa][Ss][Mm] call s:FTemas()
	function! s:FTemas()

		let n = 1
		let level = 0

		while n <= 100 && n <= line("$")
			let ln = getline(n)

			" if cpu is set - no doubts
			if 1 == 2
			elseif (ln =~? '^[ \t]*\.cpu[ \t]\+\(mx16\|mera400\)')
				let level = 100
			" em400 tests - no doubts
			elseif (ln =~ '^;[ \t]\+XPCT')
				let level = 100
			" strong allocations
			elseif (ln =~? '^\([a-z0-9_]\+:\)\?[ \t]*\(\.dword\|\.lbyte\|\.rbyte\)')
				let level = level + 2
			" strong mnemonics
			elseif (ln =~? '^\([a-z0-9_]\+:\)\?[ \t]*\(mcl\|lip\|siu\|sil\|sit\|cit\|giu\|gil\)[ \t]*;\?')
				let level = level + 3
			" strong mnemonics with arg
			elseif (ln =~? '^\([a-z0-9_]\+:\)\?[ \t]*\(exl\|ric\|rky\|rpc\|lpc\)[ \t]')
				let level = level + 3
			" loads
			elseif (ln =~? '^\([a-z0-9_]\+:\)\?[ \t]*\(lw\|lwt\|lws\|ld\|lf\|la\|ll\|ls\)[ \t]')
				let level = level + 1
			" takes
			elseif (ln =~? '^\([a-z0-9_]\+:\)\?[ \t]*\(tw\|td\|tf\|ta\|tl\)[ \t]')
				let level = level + 1
			" remembers
			elseif (ln =~? '^\([a-z0-9_]\+:\)\?[ \t]*\(rw\|rws\|rd\|rf\|ra\|rl\|rz\|ri\)[ \t]')
				let level = level + 1
			" puts
			elseif (ln =~? '^\([a-z0-9_]\+:\)\?[ \t]*\(pw\|pd\|pf\|pa\|pl\)[ \t]')
				let level = level + 1
			" arithmetic
			elseif (ln =~? '^\([a-z0-9_]\+:\)\?[ \t]*\(aw\|awt\|ac\|sw\|ad\|sd\|mw\|dw\|af\|sf\|df\|mf\|cw\|cwt\|nga\|ngc\)[ \t]')
				let level = level + 1
			" logic
			elseif (ln =~? '^\([a-z0-9_]\+:\)\?[ \t]*\(or\|om\|nr\|nm\|xr\|xm\|em\|er\|cl\|ngl\|slz\|sly\|slx\)[ \t]')
				let level = level + 1
			" shifts
			elseif (ln =~? '^\([a-z0-9_]\+:\)\?[ \t]*\(svz\|svy\|svx\|srz\|sry\|srx\|shc\)[ \t]')
				let level = level + 1
			" jumps
			elseif (ln =~? '^\([a-z0-9_]\+:\)\?[ \t]*\(lj\|uj\|ujs\|rj\|jgs\|jvs\|jys\|jxs\)[ \t]')
				let level = level + 1
			" loops
			elseif (ln =~? '^\([a-z0-9_]\+:\)\?[ \t]*\(trb\|irb\|drb\)[ \t]')
				let level = level + 1
			endif

			if (level > 10)
				break
			endif

			let n = n + 1
		endwhile

		if (level > 10)
			set filetype=emas
		else
			set filetype=asm
		endif

	endfunction

" vim: tabstop=4
