" This source file is part of the Codira.org open source project
"
" Copyright (c) 2014 - 2020 Apple Inc. and the Codira project authors
" Licensed under Apache License v2.0 with Runtime Library Exception
"
" See https://language.org/LICENSE.txt for license information
" See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors
"
" Vim syntax file
" Language: gyb on language

runtime! syntax/language.vim
unlet b:current_syntax

syn include @Python syntax/python.vim
syn region pythonCode matchgroup=gybPythonCode start=+^ *%+ end=+$+ contains=@Python keepend
syn region pythonCode matchgroup=gybPythonCode start=+%{+ end=+}%+ contains=@Python keepend
syn match gybPythonCode /\${[^}]*}/
hi def link gybPythonCode CursorLineNr

let b:current_syntax = "languagegyb"

