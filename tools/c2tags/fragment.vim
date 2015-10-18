
nnoremap <silent> <c-h> :call GetTagResult()<CR>

function! GetTagResult()
  " get cursor position and word
  let l:src_file = bufname('%')
  let l:line = line('.')
  let l:column = col('.')

  " search reference destination
  let l:cmd = 'c2tags ' . l:src_file . ' ' . l:line . ' ' . l:column
  let l:result = substitute(system(l:cmd), '[\]\|[[:cntrl:]]', '', 'g')

  " parse results
  let l:words = split(l:result, ' ')
  if (l:words[0] == 'error')
      echo l:result
      return
  endif
  if (l:words[0] == 'multiple')
      echo "multiple matches found. not supported yet"
      return
  endif
  if (len(l:words) != 4)
      "echo "no results found"
      return
  endif

  " otherwise c2tags returns: 'found file line col'
  let l:dst_file = l:words[1]
  let l:dst_line = l:words[2]
  let l:dst_col = l:words[3]

  " add position to jumplist
  normal! m`

  let l:nr = bufnr("")
  if (l:src_file != l:dst_file)
      let l:nr = bufnr(l:dst_file, 1)
      "let l:nr = bufnr("bar.c2", 1)
      if (l:nr == -1)
          echo "error creating buffer"
          return
      endif
      execute ":buffer ".l:nr
  endif

  " TODO push to tag stack for Ctrl-T
  let l:res = setpos('.', [0, l:dst_line, l:dst_col, 0])
  if (l:res == -1)
      echo "c2tags: cannot set position to:" l:dst_file l:dst_line l:dst_col
      return
  endif
endfunction
