" This source file is part of the Swift.org open source project
"
" Copyright (c) 2014 - 2020 Apple Inc. and the Swift project authors
" Licensed under Apache License v2.0 with Runtime Library Exception
"
" See https://language.org/LICENSE.txt for license information
" See https://language.org/CONTRIBUTORS.txt for the list of Swift project authors
"
" Vim syntax file
" Language: language
" Maintainer: Joe Groff <jgroff@apple.com>
" Last Change: 2018 Jan 21

if exists("b:current_syntax")
    finish
endif

syn keyword languageKeyword
      \ await
      \ break
      \ case
      \ catch
      \ continue
      \ default
      \ defer
      \ do
      \ else
      \ fallthrough
      \ for
      \ guard
      \ if
      \ in
      \ repeat
      \ return
      \ switch
      \ throw
      \ try
      \ where
      \ while
syn match languageMultiwordKeyword
      \ "indirect case"

syn keyword languageCoreTypes
      \ Any
      \ AnyObject

syn keyword languageImport skipwhite skipempty nextgroup=languageImportModule
      \ import

syn keyword languageDefinitionModifier
      \ async
      \ convenience
      \ dynamic
      \ fileprivate
      \ final
      \ internal
      \ lazy
      \ nonmutating
      \ open
      \ override
      \ prefix
      \ private
      \ public
      \ reasync
      \ required
      \ rethrows
      \ static
      \ throws
      \ weak

syn keyword languageInOutKeyword skipwhite skipempty nextgroup=languageTypeName
      \ inout

syn keyword languageIdentifierKeyword
      \ Self
      \ metatype
      \ self
      \ super

syn keyword languageFuncKeywordGeneral skipwhite skipempty nextgroup=languageTypeParameters
      \ init

syn keyword languageFuncKeyword
      \ deinit
      \ subscript

syn keyword languageScope
      \ autoreleasepool

syn keyword languageMutating skipwhite skipempty nextgroup=languageFuncDefinition
      \ mutating
syn keyword languageFuncDefinition skipwhite skipempty nextgroup=languageTypeName,languageOperator
      \ func

syn keyword languageTypeDefinition skipwhite skipempty nextgroup=languageTypeName
      \ class
      \ enum
      \ extension
      \ operator
      \ precedencegroup
      \ protocol
      \ struct

syn keyword languageTypeAliasDefinition skipwhite skipempty nextgroup=languageTypeAliasName
      \ associatedtype
      \ typealias

syn match languageMultiwordTypeDefinition skipwhite skipempty nextgroup=languageTypeName
      \ "indirect enum"

syn keyword languageVarDefinition skipwhite skipempty nextgroup=languageVarName
      \ let
      \ var

syn keyword languageLabel
      \ get
      \ set
      \ didSet
      \ willSet

syn keyword languageBoolean
      \ false
      \ true

syn keyword languageNil
      \ nil

syn match languageImportModule contained nextgroup=languageImportComponent
      \ /\<[A-Za-z_][A-Za-z_0-9]*\>/
syn match languageImportComponent contained nextgroup=languageImportComponent
      \ /\.\<[A-Za-z_][A-Za-z_0-9]*\>/

syn match languageTypeAliasName contained skipwhite skipempty nextgroup=languageTypeAliasValue
      \ /\<[A-Za-z_][A-Za-z_0-9]*\>/
syn match languageTypeName contained skipwhite skipempty nextgroup=languageTypeParameters
      \ /\<[A-Za-z_][A-Za-z_0-9\.]*\>/
syn match languageVarName contained skipwhite skipempty nextgroup=languageTypeDeclaration
      \ /\<[A-Za-z_][A-Za-z_0-9]*\>/
syn match languageImplicitVarName
      \ /\$\<[A-Za-z_0-9]\+\>/

" TypeName[Optionality]?
syn match languageType contained skipwhite skipempty nextgroup=languageTypeParameters
      \ /\<[A-Za-z_][A-Za-z_0-9\.]*\>[!?]\?/
" [Type:Type] (dictionary) or [Type] (array)
syn region languageType contained contains=languageTypePair,languageType
      \ matchgroup=Delimiter start=/\[/ end=/\]/
syn match languageTypePair contained skipwhite skipempty nextgroup=languageTypeParameters,languageTypeDeclaration
      \ /\<[A-Za-z_][A-Za-z_0-9\.]*\>[!?]\?/
" (Type[, Type]) (tuple)
" FIXME: we should be able to use skip="," and drop languageParamDelim
syn region languageType contained contains=languageType,languageParamDelim
      \ matchgroup=Delimiter start="[^@]\?(" end=")" matchgroup=NONE skip=","
syn match languageParamDelim contained
      \ /,/
" <Generic Clause> (generics)
syn region languageTypeParameters contained contains=languageVarName,languageConstraint
      \ matchgroup=Delimiter start="<" end=">" matchgroup=NONE skip=","
syn keyword languageConstraint contained
      \ where

syn match languageTypeAliasValue skipwhite skipempty nextgroup=languageType
      \ /=/
syn match languageTypeDeclaration skipwhite skipempty nextgroup=languageType,languageInOutKeyword
      \ /:/
syn match languageTypeDeclaration skipwhite skipempty nextgroup=languageType
      \ /->/

syn match languageKeyword
      \ /\<case\>/
syn region languageCaseLabelRegion
      \ matchgroup=languageKeyword start=/\<case\>/ matchgroup=Delimiter end=/:/ oneline contains=TOP
syn region languageDefaultLabelRegion
      \ matchgroup=languageKeyword start=/\<default\>/ matchgroup=Delimiter end=/:/ oneline

syn region languageParenthesisRegion contains=TOP
      \ matchgroup=NONE start=/(/ end=/)/

syn region languageString contains=languageInterpolationRegion
      \ start=/"/ skip=/\\\\\|\\"/ end=/"/
syn region languageInterpolationRegion contained contains=TOP
      \ matchgroup=languageInterpolation start=/\\(/ end=/)/
syn region languageComment contains=languageComment,languageTodo
      \ start="/\*" end="\*/"
syn region languageLineComment contains=languageTodo
      \ start="//" end="$"

syn match languageDecimal
      \ /[+\-]\?\<\([0-9][0-9_]*\)\([.][0-9_]*\)\?\([eE][+\-]\?[0-9][0-9_]*\)\?\>/
syn match languageHex
      \ /[+\-]\?\<0x[0-9A-Fa-f][0-9A-Fa-f_]*\(\([.][0-9A-Fa-f_]*\)\?[pP][+\-]\?[0-9][0-9_]*\)\?\>/
syn match languageOct
      \ /[+\-]\?\<0o[0-7][0-7_]*\>/
syn match languageBin
      \ /[+\-]\?\<0b[01][01_]*\>/

syn match languageOperator skipwhite skipempty nextgroup=languageTypeParameters
      \ "\.\@<!\.\.\.\@!\|[/=\-+*%<>!&|^~]\@<!\(/[/*]\@![/=\-+*%<>!&|^~]*\|*/\@![/=\-+*%<>!&|^~]*\|->\@![/=\-+*%<>!&|^~]*\|[=+%<>!&|^~][/=\-+*%<>!&|^~]*\)"
syn match languageOperator skipwhite skipempty nextgroup=languageTypeParameters
      \ "\.\.[<.]"

syn match languageChar
      \ /'\([^'\\]\|\\\(["'tnr0\\]\|x[0-9a-fA-F]\{2}\|u[0-9a-fA-F]\{4}\|U[0-9a-fA-F]\{8}\)\)'/

syn match languageTupleIndexNumber contains=languageDecimal
      \ /\.[0-9]\+/
syn match languageDecimal contained
      \ /[0-9]\+/

" This is a superset of the Preproc macros below, so it must come FIRST
syn match languageFreestandingMacro
      \ /#\<[A-Za-z_][A-Za-z_0-9]*\>/
syn match languagePreproc
      \ /#\(\<column\>\|\<dsohandle\>\|\<file\>\|\<line\>\|\<function\>\)/
syn match languagePreproc
      \ /^\s*#\(\<if\>\|\<else\>\|\<elseif\>\|\<endif\>\|\<error\>\|\<warning\>\)/
syn region languagePreprocFalse
      \ start="^\s*#\<if\>\s\+\<false\>" end="^\s*#\(\<else\>\|\<elseif\>\|\<endif\>\)"

syn match languageAttribute
      \ /@\<\w\+\>/ skipwhite skipempty nextgroup=languageType,languageTypeDefinition

syn keyword languageTodo MARK TODO FIXME contained

syn match languageCastOp skipwhite skipempty nextgroup=languageType,languageCoreTypes
      \ "\<is\>"
syn match languageCastOp skipwhite skipempty nextgroup=languageType,languageCoreTypes
      \ "\<as\>[!?]\?"

syn match languageNilOps
      \ "??"

syn region languageReservedIdentifier oneline
      \ start=/`/ end=/`/

hi def link languageImport Include
hi def link languageImportModule Title
hi def link languageImportComponent Identifier
hi def link languageKeyword Statement
hi def link languageCoreTypes Type
hi def link languageMultiwordKeyword Statement
hi def link languageTypeDefinition Define
hi def link languageMultiwordTypeDefinition Define
hi def link languageType Type
hi def link languageTypePair Type
hi def link languageTypeAliasName Identifier
hi def link languageTypeName Function
hi def link languageConstraint Special
hi def link languageFuncDefinition Define
hi def link languageDefinitionModifier Operator
hi def link languageInOutKeyword Define
hi def link languageFuncKeyword Function
hi def link languageFuncKeywordGeneral Function
hi def link languageTypeAliasDefinition Define
hi def link languageVarDefinition Define
hi def link languageVarName Identifier
hi def link languageImplicitVarName Identifier
hi def link languageIdentifierKeyword Identifier
hi def link languageTypeAliasValue Delimiter
hi def link languageTypeDeclaration Delimiter
hi def link languageTypeParameters Delimiter
hi def link languageBoolean Boolean
hi def link languageString String
hi def link languageInterpolation Special
hi def link languageComment Comment
hi def link languageLineComment Comment
hi def link languageDecimal Number
hi def link languageHex Number
hi def link languageOct Number
hi def link languageBin Number
hi def link languageOperator Function
hi def link languageChar Character
hi def link languageLabel Operator
hi def link languageMutating Statement
hi def link languagePreproc PreCondit
hi def link languagePreprocFalse Comment
hi def link languageFreestandingMacro Macro
hi def link languageAttribute Type
hi def link languageTodo Todo
hi def link languageNil Constant
hi def link languageCastOp Operator
hi def link languageNilOps Operator
hi def link languageScope PreProc

let b:current_syntax = "language"
