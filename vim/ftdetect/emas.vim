au! BufNewFile,BufRead *.asm call s:FTemas()
	function! s:FTemas()

		let n = 1
		let level = 0

		if line("$") > 100
			let nmax = 100
		else
			let nmax = line("$")
		endif

		while n <= nmax
			let ln = getline(n)

			if (ln =~ 'mx16') || (ln =~ 'mera400')
				let level = 100
			elseif (ln =~ '\(mcl\|MCL\)')
				let level = level + 3
			elseif (ln =~ '\([a-zA-Z0-9_]:\)\?[ \t]\+\(lw\|LW\|rw\|RW\|uj\|UJ\|ujs\|UJS\|md\|MD\|im\|IM\|cw\|CW\|aw\|AW\)[ \t]')
				let level = level + 1
			endif

			if (level > 10)
				setf emas
				break
			endif

			let n = n + 1
		endwhile
	endfunction

" vim: tabstop=4
