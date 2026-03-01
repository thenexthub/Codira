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

#ifndef PANDA_ASSEMBLER_ASSEMBLY_FUNCTION_H
#define PANDA_ASSEMBLER_ASSEMBLY_FUNCTION_H

#include "assembly-ins.h"
#include "assembly-label.h"
#include "assembly-type.h"
#include "assembly-debug.h"
#include "assembly-file-location.h"
#include "libarkfile/bytecode_emitter.h"
#include "extensions/extensions.h"
#include "libarkfile/file_items.h"
#include "libarkfile/file_item_container.h"
#include "ide_helpers.h"
#include "meta.h"

namespace ark::pandasm {

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
class Function {
public:
    Function() = delete;
    ~Function() = default;
    NO_COPY_SEMANTIC(Function);
    DEFAULT_MOVE_SEMANTIC(Function);

    struct CatchBlock {
        CatchBlock() = default;
        ~CatchBlock() = default;
        DEFAULT_COPY_SEMANTIC(CatchBlock);
        DEFAULT_MOVE_SEMANTIC(CatchBlock);

        std::string wholeLine;
        std::string exceptionRecord;
        std::string tryBeginLabel;
        std::string tryEndLabel;
        std::string catchBeginLabel;
        std::string catchEndLabel;
    };

    struct TryCatchInfo {
        TryCatchInfo() = delete;
        ~TryCatchInfo() = default;
        NO_COPY_SEMANTIC(TryCatchInfo);
        NO_MOVE_SEMANTIC(TryCatchInfo);

        std::unordered_map<std::string_view, size_t> tryCatchLabels;
        std::unordered_map<std::string, std::vector<const CatchBlock *>> tryCatchMap;
        std::vector<std::string> tryCatchOrder;

        explicit TryCatchInfo(std::unordered_map<std::string_view, size_t> &&labels,
                              std::unordered_map<std::string, std::vector<const CatchBlock *>> &&map,
                              std::vector<std::string> &&paramTryCatchOrder)
            : tryCatchLabels(std::move(labels)),
              tryCatchMap(std::move(map)),
              tryCatchOrder(std::move(paramTryCatchOrder))
        {
        }
    };

    struct Parameter {
        Parameter() = delete;
        ~Parameter() = default;
        NO_COPY_SEMANTIC(Parameter);
        DEFAULT_MOVE_SEMANTIC(Parameter);

        explicit Parameter(Type t, ark::panda_file::SourceLang sourceLang) : type(std::move(t)), lang(sourceLang) {}

        ParamMetadata *GetOrCreateMetadata() const
        {
            if (!metadata_) {
                metadata_ = extensions::MetadataExtension::CreateParamMetadata(lang);
            }
            return metadata_.get();
        }

        Type type;
        ark::panda_file::SourceLang lang;

    private:
        mutable std::unique_ptr<ParamMetadata> metadata_;
    };

    Type returnType;
    FileLocation fileLocation {};

    std::unordered_map<std::string, ark::pandasm::Label> labelTable;

    std::string name;
    std::string sourceFile; /* The file in which the function is defined or empty */
    std::string sourceCode;

    std::vector<ark::pandasm::Ins> ins; /* function instruction list */
    std::vector<ark::pandasm::debuginfo::LocalVariable> localVariableDebug;
    std::vector<CatchBlock> catchBlocks;
    std::vector<Parameter> params;

    SourceLocation bodyLocation;

    std::unique_ptr<FunctionMetadata> metadata;

    int64_t valueOfFirstParam = -1;
    size_t profileSize {0};
    std::uint32_t regsNum = 0U;

    ark::panda_file::SourceLang language;
    bool bodyPresence = false;

    void SetInsDebug(const std::vector<debuginfo::Ins> &insDebug)
    {
        ASSERT(insDebug.size() == ins.size());
        for (std::size_t i = 0U; i < ins.size(); ++i) {
            ins[i].insDebug = insDebug[i].Clone();
        }
    }

    void AddInstruction(ark::pandasm::Ins &&instruction)
    {
        ins.emplace_back(std::move(instruction));
    }

#if defined(NOT_OPTIMIZE_PERFORMANCE)
    void AddInstruction(ark::pandasm::Ins const &instruction)
    {
        ins.emplace_back(instruction);
    }
#endif

    explicit Function(std::string s, ark::panda_file::SourceLang lang, FileLocation &&fLoc)
        : fileLocation {std::move(fLoc)},
          name(std::move(s)),
          metadata(extensions::MetadataExtension::CreateFunctionMetadata(lang)),
          language(lang)
    {
    }

    explicit Function(std::string s, ark::panda_file::SourceLang lang)
        : name(std::move(s)), metadata(extensions::MetadataExtension::CreateFunctionMetadata(lang)), language(lang)
    {
    }

    std::size_t GetParamsNum() const
    {
        return params.size();
    }

    std::size_t GetTotalRegs() const
    {
        return static_cast<std::size_t>(regsNum);
    }

    bool IsStatic() const
    {
        return (metadata->GetAccessFlags() & ACC_STATIC) != 0;
    }

    // CC-OFFNXT(G.FUN.01-CPP) solid logic
    bool Emit(BytecodeEmitter &emitter, panda_file::MethodItem *method,
              const std::unordered_map<std::string, panda_file::BaseMethodItem *> &methods,
              const std::unordered_map<std::string, panda_file::BaseMethodItem *> &staticMethods,
              const std::unordered_map<std::string, panda_file::BaseFieldItem *> &fields,
              const std::unordered_map<std::string, panda_file::BaseFieldItem *> &staticFields,
              const std::unordered_map<std::string, panda_file::BaseClassItem *> &classes,
              const std::unordered_map<std::string_view, panda_file::StringItem *> &strings,
              const std::unordered_map<std::string, panda_file::LiteralArrayItem *> &literalarrays) const;

    size_t GetLineNumber(size_t i) const;

    uint32_t GetColumnNumber(size_t i) const;

    struct LocalVariablePair {
        size_t insnOrder;
        size_t variableIndex;
        LocalVariablePair(size_t order, size_t index) : insnOrder(order), variableIndex(index) {}
    };
    void CollectLocalVariable(std::vector<LocalVariablePair> &localVariableInfo) const;
    // CC-OFFNXT(G.FUN.01-CPP) solid logic
    void EmitLocalVariable(panda_file::LineNumberProgramItem *program, panda_file::ItemContainer *container,
                           std::vector<uint8_t> *constantPool, uint32_t &pcInc, size_t instructionNumber,
                           size_t variableIndex) const;
    void EmitNumber(panda_file::LineNumberProgramItem *program, std::vector<uint8_t> *constantPool, uint32_t pcInc,
                    int32_t lineInc) const;
    void EmitLineNumber(panda_file::LineNumberProgramItem *program, std::vector<uint8_t> *constantPool,
                        uint32_t &prevLineNumber, uint32_t &pcInc, size_t instructionNumber) const;

    // column number is only for dynamic language now
    void EmitColumnNumber(panda_file::LineNumberProgramItem *program, std::vector<uint8_t> *constantPool,
                          uint32_t &prevColumnNumber, uint32_t &pcInc, size_t instructionNumber) const;

    void BuildLineNumberProgram(panda_file::DebugInfoItem *debugItem, const std::vector<uint8_t> &bytecode,
                                panda_file::ItemContainer *container, std::vector<uint8_t> *constantPool,
                                bool emitDebugInfo) const;

    Function::TryCatchInfo MakeOrderAndOffsets(const std::vector<uint8_t> &bytecode) const;

    std::vector<panda_file::CodeItem::TryBlock> BuildTryBlocks(
        panda_file::MethodItem *method, const std::unordered_map<std::string, panda_file::BaseClassItem *> &classItems,
        const std::vector<uint8_t> &bytecode) const;

    bool HasImplementation() const
    {
        return !metadata->IsForeign() && metadata->HasImplementation();
    }

    bool IsParameter(uint32_t regNumber) const
    {
        return regNumber >= regsNum;
    }

    bool CanThrow() const
    {
        return std::any_of(ins.cbegin(), ins.cend(), [](const Ins &insn) { return insn.CanThrow(); });
    }

    bool HasDebugInfo() const
    {
        return std::any_of(ins.cbegin(), ins.cend(), [](const Ins &insn) { return insn.HasDebugInfo(); });
    }

    PANDA_PUBLIC_API void DebugDump() const;
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

}  // namespace ark::pandasm

#endif  // PANDA_ASSEMBLER_ASSEMBLY_FUNCTION_H
