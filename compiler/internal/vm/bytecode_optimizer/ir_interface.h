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

#ifndef BYTECODE_OPTIMIZER_IR_INTERFACE_H
#define BYTECODE_OPTIMIZER_IR_INTERFACE_H

#include <string>

#include "assembler/assembly-emitter.h"
#include "libpandafile/method_data_accessor-inl.h"
#include "compiler/optimizer/ir/constants.h"

namespace panda::bytecodeopt {
class BytecodeOptIrInterface {
public:
    explicit BytecodeOptIrInterface(const pandasm::AsmEmitter::PandaFileToPandaAsmMaps *maps,
                                    pandasm::Program *prog = nullptr)
        : prog_(prog), maps_(maps)
    {
    }

    virtual ~BytecodeOptIrInterface() = default;

    virtual std::string GetMethodIdByOffset(uint32_t offset) const
    {
        auto it = maps_->methods.find(offset);
        ASSERT(it != maps_->methods.cend());

        return std::string(it->second);
    }

    virtual std::string GetStringIdByOffset(uint32_t offset) const
    {
        auto it = maps_->strings.find(offset);
        ASSERT(it != maps_->strings.cend());

        return std::string(it->second);
    }

    virtual std::string GetLiteralArrayByOffset(uint32_t offset) const
    {
        auto it = maps_->literalarrays.find(offset);
        ASSERT(it != maps_->strings.cend());

        return std::string(it->second);
    }

    std::optional<std::string> GetLiteralArrayIdByOffset(uint32_t offset) const
    {
        ASSERT(prog_ != nullptr);
        if (prog_ == nullptr) {
            return std::nullopt;
        }
        auto id = std::to_string(offset);
        auto it = prog_->literalarray_table.find(id);
        ASSERT(it != prog_->literalarray_table.end());
        return it != prog_->literalarray_table.end() ? std::optional<std::string>(id) : std::nullopt;
    }

    virtual std::string GetTypeIdByOffset(uint32_t offset) const
    {
        auto it = maps_->classes.find(offset);
        ASSERT(it != maps_->classes.cend());

        return std::string(it->second);
    }

    virtual std::string GetFieldIdByOffset(uint32_t offset) const
    {
        auto it = maps_->fields.find(offset);
        ASSERT(it != maps_->fields.cend());

        return std::string(it->second);
    }

    std::unordered_map<size_t, pandasm::Ins *> *GetPcInsMap()
    {
        return &pc_ins_map_;
    }

    size_t GetLineNumberByPc(size_t pc) const
    {
        if (pc == compiler::INVALID_PC || pc_ins_map_.size() == 0) {
            return 0;
        }
        auto iter = pc_ins_map_.find(pc);
        if (iter == pc_ins_map_.end()) {
            return 0;
        }
        return iter->second->ins_debug.line_number;
    }

    uint32_t GetColumnNumberByPc(size_t pc) const
    {
        if (pc == compiler::INVALID_PC || pc_ins_map_.size() == 0) {
            return compiler::INVALID_COLUMN_NUM;
        }
        auto iter = pc_ins_map_.find(pc);
        if (iter == pc_ins_map_.end()) {
            return compiler::INVALID_COLUMN_NUM;
        }

        return iter->second->ins_debug.column_number;
    }

    void ClearPcInsMap()
    {
        pc_ins_map_.clear();
    }

    void StoreLiteralArray(std::string id, pandasm::LiteralArray &&literalarray)
    {
        ASSERT(prog_ != nullptr);
        if (prog_ == nullptr) {
            return;
        }
        prog_->literalarray_table.emplace(id, std::move(literalarray));
    }

    size_t GetLiteralArrayTableSize() const
    {
        ASSERT(prog_ != nullptr);
        if (prog_ == nullptr) {
            return 0;
        }
        return prog_->literalarray_table.size();
    }

    bool IsMapsSet() const
    {
        return maps_ != nullptr;
    }

    panda_file::SourceLang GetSourceLang()
    {
        return prog_ != nullptr ? prog_->lang : panda_file::SourceLang::PANDA_ASSEMBLY;
    }

    pandasm::Program *GetProgram() const
    {
        return prog_;
    }

private:
    pandasm::Program *prog_ {nullptr};
    const pandasm::AsmEmitter::PandaFileToPandaAsmMaps *maps_ {nullptr};
    std::unordered_map<size_t, pandasm::Ins *> pc_ins_map_;
};
}  // namespace panda::bytecodeopt

#endif  // BYTECODE_OPTIMIZER_IR_INTERFACE_H
