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

#ifndef ABC2PROGRAM_DUMP_UTILS_H
#define ABC2PROGRAM_DUMP_UTILS_H

#include <string>
#include <string_view>
#include <unordered_map>
#include <iostream>
#include <assembly-program.h>
#include "file.h"

namespace panda::abc2program {

using LiteralTagToStringMap = std::unordered_map<panda_file::LiteralTag, std::string>;
using LabelMap = std::unordered_map<std::string, std::string>;
using FunctionKindToStringMap = std::unordered_map<panda_file::FunctionKind, std::string>;
using OpcodeLiteralIdIndexMap = std::unordered_map<pandasm::Opcode, size_t>;

// dump constant
constexpr std::string_view DUMP_TITLE_SOURCE_BINARY = "# source binary: ";
constexpr std::string_view DUMP_TITLE_LANGUAGE = ".language ";
constexpr std::string_view DUMP_TITLE_LITERALS = "# LITERALS";
constexpr std::string_view DUMP_TITLE_RECORDS = "# RECORDS";
constexpr std::string_view DUMP_TITLE_RECORD = ".record ";
constexpr std::string_view DUMP_TITLE_RECORD_SOURCE_FILE = ".record.source_file ";
constexpr std::string_view DUMP_TITLE_METHODS = "# METHODS";
constexpr std::string_view DUMP_TITLE_FUNCTION = ".function ";
constexpr std::string_view DUMP_TITLE_FUNCTION_KIND = ".function_kind ";
constexpr std::string_view DUMP_TITLE_CATCH_ALL = ".catchall :";
constexpr std::string_view DUMP_TITLE_CATCH = ".catch :";
constexpr std::string_view DUMP_TITLE_STRING = "# STRING\n";
constexpr std::string_view DUMP_TITLE_SEPARATOR = "# ====================\n";
constexpr std::string_view DUMP_TITLE_LOCAL_VAR_TABLE = "# LOCAL_VARIABLE_TABLE:";
constexpr std::string_view DUMP_CONTENT_ECMASCRIPT = "ECMAScript";
constexpr std::string_view DUMP_CONTENT_JAVASCRIPT  = "JavaScript";
constexpr std::string_view DUMP_CONTENT_TYPESCRIPT = "TypeScript";
constexpr std::string_view DUMP_CONTENT_ARKTS = "ArkTS";
constexpr std::string_view DUMP_CONTENT_PANDA_ASSEMBLY = "PandaAssembly";
constexpr std::string_view DUMP_CONTENT_SPACE = " ";
constexpr std::string_view DUMP_CONTENT_DOUBLE_SPACES = "  ";
constexpr std::string_view DUMP_CONTENT_TRIPLE_SPACES = "   ";
constexpr std::string_view DUMP_CONTENT_NONUPLE_SPACES = "         ";
constexpr std::string_view DUMP_CONTENT_TAB = "\t";
constexpr std::string_view DUMP_CONTENT_SINGLE_ENDL = "\n";
constexpr std::string_view DUMP_CONTENT_DOUBLE_ENDL = "\n\n";
constexpr std::string_view DUMP_CONTENT_COLON = ":";
constexpr std::string_view DUMP_CONTENT_ATTR_EXTERNAL = "<external>";
constexpr std::string_view DUMP_CONTENT_ATTR_STATIC = "<static>";
constexpr std::string_view DUMP_CONTENT_LEFT_CURLY_BRACKET = "{";
constexpr std::string_view DUMP_CONTENT_RIGHT_CURLY_BRACKET = "}";
constexpr std::string_view DUMP_CONTENT_LEFT_SQUARE_BRACKET = "[";
constexpr std::string_view DUMP_CONTENT_RIGHT_SQUARE_BRACKET = "]";
constexpr std::string_view DUMP_CONTENT_LEFT_PARENTHESIS = "(";
constexpr std::string_view DUMP_CONTENT_RIGHT_PARENTHESIS = ")";
constexpr std::string_view DUMP_CONTENT_COMMA = ",";
constexpr std::string_view DUMP_CONTENT_LOCAL_VAR_TABLE = "#\t Start   Length  Register           Name   Signature\n";
constexpr std::string_view DUMP_CONTENT_LINE_NUMBER = " # line: ";
constexpr std::string_view DUMP_CONTENT_COLUMN_NUMBER = " # column: ";
constexpr std::string_view DUMP_CONTENT_FUNCTION_PARAM_NAME_PREFIX = "a";
constexpr std::string_view DUMP_CONTENT_TRY_BEGIN_LABEL = "try_begin_label : ";
constexpr std::string_view DUMP_CONTENT_TRY_END_LABEL = "try_end_label : ";
constexpr std::string_view DUMP_CONTENT_CATCH_BEGIN_LABEL = "catch_begin_label : ";
constexpr std::string_view DUMP_CONTENT_CATCH_END_LABEL = "catch_end_label : ";

class PandasmDumperUtils {
public:
    static std::string GetFunctionKindString(panda_file::FunctionKind function_kind);
    static std::string LiteralTagToString(const panda_file::LiteralTag &tag);
    static bool IsMatchLiteralId(const pandasm::InsPtr &pa_ins);
    static size_t GetLiteralIdIndex4Ins(const pandasm::InsPtr &pa_ins);
    static std::string GetMappedLabel(const std::string &label, const LabelMap &label_map);
    static pandasm::Function::CatchBlock DeepCopyCatchBlock(const pandasm::Function::CatchBlock &catch_block);
    static uint32_t GetLiteralArrayIdFromName(const std::string &literal_array_id_name);

    static LiteralTagToStringMap literal_tag_to_string_map_;
    static OpcodeLiteralIdIndexMap opcode_literal_id_index_map_;
    static FunctionKindToStringMap function_kind_to_string_map_;
};  // class PandasmDumperUtils

}  // namespace panda::abc2program

#endif  // ABC2PROGRAM_DUMP_UTILS_H