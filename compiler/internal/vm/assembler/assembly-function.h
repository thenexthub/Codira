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

#ifndef ASSEMBLER_ASSEMBLY_FUNCTION_H
#define ASSEMBLER_ASSEMBLY_FUNCTION_H

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "assembly-ins.h"
#include "assembly-label.h"
#include "assembly-type.h"
#include "assembly-debug.h"
#include "assembly-file-location.h"
#include "bytecode_emitter.h"
#include "extensions/extensions.h"
#include "file_items.h"
#include "file_item_container.h"
#include "ide_helpers.h"
#include "meta.h"

namespace panda::pandasm {
struct LocalVarEmitContext {
    uint32_t &pc_inc;
    size_t instruction_number;
    size_t variable_index;
};

struct Function {
    struct CatchBlock {
        std::string whole_line;
        std::string exception_record;
        std::string try_begin_label;
        std::string try_end_label;
        std::string catch_begin_label;
        std::string catch_end_label;
    };

    struct TryCatchInfo {
        std::unordered_map<std::string_view, size_t> try_catch_labels;
        std::unordered_map<std::string, std::vector<const CatchBlock *>> try_catch_map;
        std::vector<std::string> try_catch_order;
        TryCatchInfo(std::unordered_map<std::string_view, size_t> &labels,
                     std::unordered_map<std::string, std::vector<const CatchBlock *>> &map,
                     std::vector<std::string> &param_try_catch_order)
            : try_catch_labels(labels), try_catch_map(map), try_catch_order(param_try_catch_order)
        {
        }
    };

    struct Parameter {
        Type type;

        Parameter(Type t, panda::panda_file::SourceLang lang) : type(std::move(t)) {}
    };

    std::string name = "";
    panda::panda_file::SourceLang language;
    std::unique_ptr<FunctionMetadata> metadata;

    std::unordered_map<std::string, panda::pandasm::Label> label_table;
    std::vector<InsPtr> ins; /* function instruction list */
    std::vector<panda::pandasm::debuginfo::LocalVariable> local_variable_debug;
    std::string source_file; /* The file in which the function is defined or empty */
    std::string source_code;
    std::vector<CatchBlock> catch_blocks;
    int64_t value_of_first_param = -1;
    size_t regs_num = 0;
    std::vector<Parameter> params;
    bool body_presence = false;
    Type return_type;
    SourceLocation body_location;
    std::optional<FileLocation> file_location;
    panda::panda_file::FunctionKind function_kind = panda::panda_file::FunctionKind::NONE;
    size_t slots_num = 0;
    std::vector<int> concurrent_module_requests;
    size_t expected_property_count = 0;

    void SetSlotsNum(size_t num)
    {
        slots_num = num;
    }

    size_t GetSlotsNum() const
    {
        return slots_num;
    }

    void SetFunctionKind(panda::panda_file::FunctionKind kind)
    {
        function_kind = kind;
    }

    panda::panda_file::FunctionKind GetFunctionKind() const
    {
        return function_kind;
    }

    void SetExpectedPropertyCount(size_t count)
    {
        expected_property_count = count;
    }

    size_t GetExpectedPropertyCount() const
    {
        return expected_property_count;
    }

    Function(std::string s, panda::panda_file::SourceLang lang, size_t b_l, size_t b_r, std::string f_c, bool d,
             size_t l_n)
        : name(std::move(s)),
          language(lang),
          metadata(extensions::MetadataExtension::CreateFunctionMetadata(lang)),
          file_location({f_c, b_l, b_r, l_n, d})
    {
    }

    Function(std::string s, panda::panda_file::SourceLang lang)
        : name(std::move(s)), language(lang), metadata(extensions::MetadataExtension::CreateFunctionMetadata(lang))
    {
    }

    NO_COPY_SEMANTIC(Function);
    DEFAULT_MOVE_SEMANTIC(Function);

    std::size_t GetParamsNum() const
    {
        return params.size();
    }

    std::size_t GetTotalRegs() const
    {
        return regs_num;
    }

    bool IsStatic() const
    {
        return (metadata->GetAccessFlags() & ACC_STATIC) != 0;
    }

    bool Emit(BytecodeEmitter &emitter, panda_file::MethodItem *method,
              const std::unordered_map<std::string, panda_file::BaseMethodItem *> &methods,
              const std::unordered_map<std::string, panda_file::BaseFieldItem *> &fields,
              const std::unordered_map<std::string, panda_file::BaseClassItem *> &classes,
              const std::unordered_map<std::string, panda_file::StringItem *> &strings,
              const std::unordered_map<std::string, panda_file::LiteralArrayItem *> &literalarrays) const;

    size_t GetLineNumber(size_t i) const;

    uint32_t GetColumnNumber(size_t i) const;

    struct LocalVariablePair {
        size_t insn_order;
        size_t variable_index;
        LocalVariablePair(size_t order, size_t index) : insn_order(order), variable_index(index) {}
    };
    void CollectLocalVariable(std::vector<LocalVariablePair> &local_variable_info) const;
    void EmitLocalVariable(panda_file::LineNumberProgramItem *program, const panda_file::ItemContainer *container,
                           std::vector<uint8_t> *constant_pool, const LocalVarEmitContext &context) const;
    void EmitNumber(panda_file::LineNumberProgramItem *program, std::vector<uint8_t> *constant_pool, uint32_t pc_inc,
                    int32_t line_inc) const;
    void EmitLineNumber(panda_file::LineNumberProgramItem *program, std::vector<uint8_t> *constant_pool,
                        int32_t &prev_line_number, uint32_t &pc_inc, size_t instruction_number) const;

    // column number is only for dynamic language now
    void EmitColumnNumber(panda_file::LineNumberProgramItem *program, std::vector<uint8_t> *constant_pool,
                          uint32_t &prev_column_number, uint32_t &pc_inc, size_t instruction_number) const;

    void BuildLineNumberProgram(panda_file::DebugInfoItem *debug_item, const std::vector<uint8_t> &bytecode,
                                const panda_file::ItemContainer *container, std::vector<uint8_t> *constant_pool,
                                bool emit_debug_info) const;

    Function::TryCatchInfo MakeOrderAndOffsets(const std::vector<uint8_t> &bytecode) const;

    std::vector<panda_file::CodeItem::TryBlock> BuildTryBlocks(
        panda_file::MethodItem *method, const std::unordered_map<std::string, panda_file::BaseClassItem *> &class_items,
        const std::vector<uint8_t> &bytecode) const;

    bool HasImplementation() const
    {
        return !metadata->IsForeign();
    }

    bool IsParameter(uint32_t reg_number) const
    {
        return reg_number >= regs_num;
    }

    bool CanThrow() const
    {
        return std::any_of(ins.cbegin(), ins.cend(), [](const InsPtr &insn) { return insn->CanThrow(); });
    }

    bool HasDebugInfo() const
    {
        return std::any_of(ins.cbegin(), ins.cend(), [](const InsPtr &insn) { return insn->HasDebugInfo(); });
    }

    PANDA_PUBLIC_API void DebugDump() const;

    std::set<std::string> CollectStringsFromFunctionInsns() const;
};

}  // namespace panda::pandasm

#endif  // ASSEMBLER_ASSEMBLY_FUNCTION_H
