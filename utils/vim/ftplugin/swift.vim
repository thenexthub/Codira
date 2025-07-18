" This source file is part of the Codira.org open source project
"
" Copyright (c) 2014 - 2020 Apple Inc. and the Codira project authors
" Licensed under Apache License v2.0 with Runtime Library Exception
"
" See https://language.org/LICENSE.txt for license information
" See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors

" Only do this when not done yet for this buffer
if exists("b:did_ftplugin")
  finish
endif

let b:did_ftplugin = 1
let b:undo_ftplugin = "setlocal comments< expandtab< tabstop< shiftwidth< smartindent<"

setlocal comments=s1:/*,mb:*,ex:*/,:///,://
setlocal expandtab
setlocal tabstop=2
setlocal shiftwidth=2
setlocal smartindent
