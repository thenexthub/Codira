/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

/**
 * @file
 *
 * TThis file declares the constants used in CodeGen.
 */

#ifndef CODIRA_CODEGENCONSTANTS_H
#define CODIRA_CODEGENCONSTANTS_H

#include <set>
#include <string>

namespace Codira {
namespace CodeGen {
const size_t UI64_WIDTH = 64;
const size_t I64_WIDTH = 64;
const unsigned long INHERITED_CLASS_NUM_FE_FLAG = 1UL << 15;
const std::string UNIT_TYPE_STR = "Unit.Type";
const std::string UNIT_VAL_STR = "Unit.Val";
const std::string ARRAY_LAYOUT_PREFIX = "ArrayLayout.";
const std::string INCREMENTAL_CFUNC_ATTR = "incremental";
const std::string INTERNAL_CFUNC_ATTR = "internal";
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
const std::string FAST_NATIVE_ATTR = "gc-leaf-function";
#endif
const std::string VTABLE_LOOKUP = "VTABLE_LOOKUP"; // Indicates that it is the instruction of VTable lookup.
const std::string GC_TYPE_META_NAME = "RelatedType";
const std::string GC_GLOBAL_VAR_TYPE = "GlobalVarType";
const std::string GC_KLASS_ATTR = "CFileKlass";
const std::string GC_TI_EXT_ATTR = "CFileTIExt";
const std::string NOT_MODIFIABLE_CLASS_ATTR = "NotModifiableClass";
const std::string HAS_EXT_PART = "HasExtPart";
const std::string GC_MTABLE_ATTR = "CFileMTable";
const std::string GC_FINALIZER_ATTR = "HasFinalizer";
const std::string STRUCT_MUT_FUNC_ATTR = "record_mut";
const std::string THIS_PARAM_HAS_BP = "thisParamHasBP";
const std::string STRUCT_TYPE_PREFIX = "record.";
const std::string CODE2C_ATTR = "code2c";
const std::string C2CODE_ATTR = "c2code";
const std::string CODESTUB_ATTR = "codestub";
const std::string CFUNC_ATTR = "cfunc";
const std::string PREFIX_OF_BUILT_IN_SYMS = "CODE_";
const std::string PREFIX_OF_RUNTIME_SYMS = PREFIX_OF_BUILT_IN_SYMS + "MRT_";
const std::string PREFIX_OF_BACKEND_SYMS = PREFIX_OF_BUILT_IN_SYMS + "MCC_";
const std::string FOR_KEEPING_SOME_TYPES_FUNC_NAME = "0_for_keeping_some_types";
const std::string USER_MAIN_MANGLED_NAME = "user.main";
const std::string CODE_ENTRY_FUNC_NAME = "code_entry$";
const std::string CONST_TUPLE_PREFIX = "$const_tuple.";
const std::string CONST_ARRAY_PREFIX = "$const_array.";
const std::string FUNC_USED_BY_CLOSURE = "UsedByClosure";
const std::string ENUM_TYPE_PREFIX = "enum.";
const std::string CODESTRING_LITERAL_PREFIX = "$const_codestring.";
const std::string CODESTRING_DATA_PREFIX = "$const_codestring_data.";
const std::string CODESTRING_LITERAL_ATTR = "codestring_literal";
const std::string CODESTRING_DATA_ATTR = "codestring_data";
const std::string CODEGLOBAL_VALUE_ATTR = "CODEGlobalValue";
const std::string CODETYPE_NAME_ATTR = "CODETypeName";
const std::string CODETI_OFFSETS_ATTR = "CODETIOffsets";
const std::string CODETI_TYPE_ARGS_ATTR = "CODETITypeArgs";
const std::string CODETI_FIELDS_ATTR = "CODETIFields";
const std::string CODETI_UPPER_BOUNDS_ATTR = "CODETIUpperBounds";
const std::string CODETT_FIELDS_FNS_ATTR = "CODETTFieldsFns";
const std::string CODEED_FUNC_TABLE_ATTR = "CODEFuncTable";
const std::string BASEPTR_SUFFIX = "$BP";
const std::string METADATA_TYPES = "types";
const std::string METADATA_PRIMITIVE_TYPES = "primitive_tis";
const std::string METADATA_PRIMITIVE_TYPETEMPLATES = "primitive_tts";
const std::string METADATA_TYPETEMPLATES = "type_templates";
const std::string METADATA_PKG = "pkg_info";
const std::string METADATA_FUNCTIONS = "functions";
const std::string ATTR_IMMUTABLE = "immutable"; // for let-fields
const std::string METADATA_ATTR_OPEN = "open";  // for mut-functions
const std::string METADATA_GLOBAL_VAR = "global_variables";
const std::string PREFIX_FOR_BB_NAME = "bb";
const std::string GENERIC_DECL_IN_IMPORTED_PKG_ATTR = "generic_decl_in_imported_pkg";
const std::string GENERIC_DECL_IN_CURRENT_PKG_ATTR = "generic_decl_in_current_pkg";
const std::string TYPE_TEMPLATE_ATTR = "code_tt";
const std::string GENERIC_TYPEINFO_ATTR = "code_generic_ti";
const std::string POSTFIX_WITHOUT_TI = "$withoutTI";
const std::string GENERIC_PREFIX = "$G_";
const std::string HAS_WITH_TI_WRAPPER_ATTR = "hasWithTIWrapper";
const std::string PKG_GV_INIT_PREFIX = "_CGP";
const std::string FILE_GV_INIT_PREFIX = "_CGF";
} // namespace CodeGen
} // namespace Codira
#endif // CODIRA_CODEGENCONSTANTS_H
