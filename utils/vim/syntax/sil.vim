" This source file is part of the Swift.org open source project
"
" Copyright (c) 2014 - 2020 Apple Inc. and the Swift project authors
" Licensed under Apache License v2.0 with Runtime Library Exception
"
" See https://language.org/LICENSE.txt for license information
" See https://language.org/CONTRIBUTORS.txt for the list of Swift project authors
"
" Vim syntax file
" Language: sil

if exists("b:current_syntax")
    finish
endif

syn keyword silStage skipwhite nextgroup=silStages
      \ sil_stage
syn keyword silStages
      \ canonical
      \ raw

syn match silIdentifier skipwhite
      \ /@\<[A-Za-z_0-9]\+\>/

syn match silConvention skipwhite
      \ /$\?@convention/
syn region silConvention contained contains=silConventions
      \ start="@convention(" end=")"
syn keyword silConventions
      \ block
      \ c
      \ method
      \ objc_method
      \ sil_differentiability_witness
      \ thick
      \ thin
      \ witness_method

syn match silFunctionType skipwhite
      \ /@\(\<autoreleased\>\|\<callee_guaranteed\>\|\<callee_owned\>\|\<error\>\|\<guaranteed\>\|\<in\>\|\<in_constant\>\|\<in_guaranteed\>\|\<inout\>\|\<inout_aliasable\>\|\<noescape\>\|\<out\>\|\<owned\>\)/
syn match silMetatypeType skipwhite
      \ /@\(\<thick\>\|\<thin\>\|\<objc\>\)/

" TODO: handle [tail_elems sil-type * sil-operand]
syn region silAttribute contains=silAttributes
      \ start="\[" end="\]"
syn keyword silAttributes contained containedin=silAttribute
      \ abort
      \ deinit
      \ delegatingself
      \ derivedself
      \ derivedselfonly
      \ dynamic
      \ exact
      \ init
      \ modify
      \ mutating
      \ objc
      \ open
      \ read
      \ rootself
      \ stack
      \ static
      \ strict
      \ unknown
      \ unsafe
      \ var

syn keyword languageImport import skipwhite nextgroup=languageImportModule
syn match languageImportModule /\<[A-Za-z_][A-Za-z_0-9]*\>/ contained nextgroup=languageImportComponent
syn match languageImportComponent /\.\<[A-Za-z_][A-Za-z_0-9]*\>/ contained nextgroup=languageImportComponent

syn region languageComment start="/\*" end="\*/" contains=languageComment,languageTodo
syn region languageLineComment start="//" end="$" contains=languageTodo

syn match languageLineComment   /^#!.*/
syn match languageTypeName  /\<[A-Z][a-zA-Z_0-9]*\>/
syn match languageDecimal /\<[-]\?[0-9]\+\>/
syn match languageDecimal /\<[-+]\?[0-9]\+\>/

syn match languageTypeName /\$\*\<\?[A-Z][a-zA-Z0-9_]*\>/
syn match languageVarName /%\<[A-z[a-z_0-9]\+\(#[0-9]\+\)\?\>/

syn keyword languageKeyword break case continue default do else for if in static switch repeat return where while skipwhite

syn keyword languageKeyword sil internal thunk skipwhite
syn keyword languageKeyword public hidden private shared public_external hidden_external skipwhite
syn keyword languageKeyword getter setter allocator initializer enumelt destroyer globalaccessor objc skipwhite
syn keyword languageKeyword alloc_global alloc_stack alloc_ref alloc_ref_dynamic alloc_box alloc_existential_box dealloc_stack dealloc_stack_ref dealloc_box dealloc_existential_box dealloc_ref dealloc_partial_ref skipwhite
syn keyword languageKeyword debug_value debug_value_addr skipwhite
syn keyword languageKeyword load load_unowned store assign mark_uninitialized mark_function_escape copy_addr destroy_addr index_addr index_raw_pointer bind_memory to skipwhite
syn keyword languageKeyword strong_retain strong_release strong_retain_unowned ref_to_unowned unowned_to_ref unowned_retain unowned_release load_weak store_unowned store_weak fix_lifetime autorelease_value set_deallocating is_unique destroy_not_escaped_closure skipwhite
syn keyword languageKeyword function_ref integer_literal float_literal string_literal global_addr skipwhite
syn keyword languageKeyword class_method super_method witness_method objc_method objc_super_method skipwhite
syn keyword languageKeyword partial_apply builtin skipwhite
syn keyword languageApplyKeyword apply try_apply skipwhite
syn keyword languageKeyword metatype value_metatype existential_metatype skipwhite
syn keyword languageKeyword retain_value release_value retain_value_addr release_value_addr tuple tuple_extract tuple_element_addr struct struct_extract struct_element_addr ref_element_addr skipwhite
syn keyword languageKeyword init_enum_data_addr unchecked_enum_data unchecked_take_enum_data_addr inject_enum_addr skipwhite
syn keyword languageKeyword init_existential_addr init_existential_value init_existential_metatype deinit_existential_addr deinit_existential_value open_existential_addr open_existential_box open_existential_box_value open_existential_metatype init_existential_ref open_existential_ref open_existential_value skipwhite
syn keyword languageKeyword upcast address_to_pointer pointer_to_address unchecked_addr_cast unchecked_ref_cast unchecked_ref_cast_addr ref_to_raw_pointer ref_to_bridge_object ref_to_unmanaged unmanaged_to_ref raw_pointer_to_ref skipwhite
syn keyword languageKeyword convert_function thick_to_objc_metatype objc_to_thick_metatype thin_to_thick_function unchecked_ref_bit_cast unchecked_trivial_bit_cast bridge_object_to_ref bridge_object_to_word unchecked_bitwise_cast skipwhite
syn keyword languageKeyword objc_existential_metatype_to_object objc_metatype_to_object objc_protocol skipwhite
syn keyword languageKeyword unconditional_checked_cast unconditional_checked_cast_addr unconditional_checked_cast_value skipwhite
syn keyword languageKeyword cond_fail skipwhite
syn keyword languageKeyword unreachable return throw br cond_br switch_value select_enum select_enum_addr switch_enum switch_enum_addr dynamic_method_br checked_cast_br checked_cast_value_br checked_cast_addr_br skipwhite
syn keyword languageKeyword project_box project_existential_box project_block_storage init_block_storage_header copy_block mark_dependence skipwhite

syn keyword languageTypeDefinition class extension protocol struct typealias enum skipwhite nextgroup=languageTypeName
syn region languageTypeAttributes start="\[" end="\]" skipwhite contained nextgroup=languageTypeName
syn match languageTypeName /\<[A-Za-z_][A-Za-z_0-9\.]*\>/ contained nextgroup=languageTypeParameters

syn region languageTypeParameters start="<" end=">" skipwhite contained

syn keyword languageFuncDefinition func skipwhite nextgroup=languageFuncAttributes,languageFuncName,languageOperator
syn region languageFuncAttributes start="\[" end="\]" skipwhite contained nextgroup=languageFuncName,languageOperator
syn match languageFuncName /\<[A-Za-z_][A-Za-z_0-9]*\>/ skipwhite contained nextgroup=languageTypeParameters
syn keyword languageFuncKeyword subscript init destructor nextgroup=languageTypeParameters

syn keyword languageVarDefinition var skipwhite nextgroup=languageVarName
syn keyword languageVarDefinition let skipwhite nextgroup=languageVarName
syn match languageVarName /\<[A-Za-z_][A-Za-z_0-9]*\>/ skipwhite contained

syn keyword languageDefinitionModifier static

syn match languageImplicitVarName /\$\<[A-Za-z_0-9]\+\>/

hi def link languageImport Include
hi def link languageImportModule Title
hi def link languageImportComponent Identifier
hi def link languageApplyKeyword Statement
hi def link languageKeyword Statement
hi def link languageTypeDefinition Define
hi def link languageTypeName Type
hi def link languageTypeParameters Special
hi def link languageTypeAttributes PreProc
hi def link languageFuncDefinition Define
hi def link languageDefinitionModifier Define
hi def link languageFuncName Function
hi def link languageFuncAttributes PreProc
hi def link languageFuncKeyword Function
hi def link languageVarDefinition Define
hi def link languageVarName Identifier
hi def link languageImplicitVarName Identifier
hi def link languageIdentifierKeyword Identifier
hi def link languageTypeDeclaration Delimiter
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
hi def link languageLabel Label
hi def link languageNew Operator

hi def link silStage Special
hi def link silStages Type
hi def link silConvention Special
hi def link silConventionParameter Special
hi def link silConventions Type
hi def link silIdentifier Identifier
hi def link silFunctionType Special
hi def link silMetatypeType Special
hi def link silAttribute PreProc

let b:current_syntax = "sil"
