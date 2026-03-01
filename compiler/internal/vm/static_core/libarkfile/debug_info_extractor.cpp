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

#include "libarkfile/debug_helpers.h"
#include "libarkfile/debug_info_extractor.h"
#include "libarkfile/line_number_program.h"
#include "libarkfile/class_data_accessor-inl.h"
#include "libarkfile/debug_data_accessor-inl.h"
#include "libarkbase/utils/utf.h"

namespace ark::panda_file {
DebugInfoExtractor::DebugInfoExtractor(const File *pf)
{
    Extract(pf);
}

class LineNumberProgramHandler {
public:
    explicit LineNumberProgramHandler(LineProgramState *state) : state_(state) {}
    ~LineNumberProgramHandler() = default;

    NO_COPY_SEMANTIC(LineNumberProgramHandler);
    NO_MOVE_SEMANTIC(LineNumberProgramHandler);

    LineProgramState *GetState() const
    {
        return state_;
    }

    void ProcessBegin()
    {
        lnt_.push_back({state_->GetAddress(), state_->GetLine()});
    }

    void ProcessEnd()
    {
        ProcessVars();
    }

    bool HandleAdvanceLine(int32_t lineDiff) const
    {
        state_->AdvanceLine(lineDiff);
        return true;
    }

    bool HandleAdvancePc(uint32_t pcDiff) const
    {
        state_->AdvancePc(pcDiff);
        return true;
    }

    bool HandleSetFile(uint32_t sourceFileId) const
    {
        state_->SetFile(sourceFileId);
        return true;
    }

    bool HandleSetSourceCode(uint32_t sourceCodeId) const
    {
        state_->SetSourceCode(sourceCodeId);
        return true;
    }

    bool HandleSetPrologueEnd() const
    {
        return true;
    }

    bool HandleSetEpilogueBegin() const
    {
        return true;
    }

    bool HandleStartLocal(int32_t regNumber, uint32_t nameId, uint32_t typeId)
    {
        const char *name = debug_helpers::GetStringFromConstantPool(state_->GetPandaFile(), nameId);
        const char *type = debug_helpers::GetStringFromConstantPool(state_->GetPandaFile(), typeId);
        lvt_.push_back({name, type, type, regNumber, state_->GetAddress(), 0});
        return true;
    }

    bool HandleStartLocalExtended(int32_t regNumber, uint32_t nameId, uint32_t typeId, uint32_t typeSignatureId)
    {
        const char *name = debug_helpers::GetStringFromConstantPool(state_->GetPandaFile(), nameId);
        const char *type = debug_helpers::GetStringFromConstantPool(state_->GetPandaFile(), typeId);
        const char *typeSign = debug_helpers::GetStringFromConstantPool(state_->GetPandaFile(), typeSignatureId);
        lvt_.push_back({name, type, typeSign, regNumber, state_->GetAddress(), 0});
        return true;
    }

    bool HandleEndLocal(int32_t regNumber)
    {
        bool found = false;
        for (auto it = lvt_.rbegin(); it != lvt_.rend(); ++it) {
            if (it->regNumber == regNumber) {
                it->endOffset = state_->GetAddress();
                found = true;
                break;
            }
        }
        if (!found) {
            LOG(FATAL, PANDAFILE) << "Unknown variable";
        }
        return true;
    }

    bool HandleRestartLocal([[maybe_unused]] int32_t regNumber) const
    {
        LOG(FATAL, PANDAFILE) << "Opcode RESTART_LOCAL is not supported";
        return true;
    }

    bool HandleSetColumn(int32_t columnNumber)
    {
        state_->SetColumn(columnNumber);
        cnt_.push_back({state_->GetAddress(), state_->GetColumn()});
        return true;
    }

    bool HandleSpecialOpcode(uint32_t pcOffset, int32_t lineOffset)
    {
        state_->AdvancePc(pcOffset);
        state_->AdvanceLine(lineOffset);
        // NOTE(fangting, #IC98Z2): make line numbers do not repeat
        lnt_.push_back({state_->GetAddress(), state_->GetLine()});
        return true;
    }

    LineNumberTable GetLineNumberTable() const
    {
        return lnt_;
    }

    LocalVariableTable GetLocalVariableTable() const
    {
        return lvt_;
    }

    ColumnNumberTable GetColumnNumberTable() const
    {
        return cnt_;
    }

    const uint8_t *GetFile() const
    {
        return state_->GetFile();
    }

    bool HasFile() const
    {
        return state_->HasFile();
    }

    const uint8_t *GetSourceCode() const
    {
        return state_->GetSourceCode();
    }

    bool HasSourceCode() const
    {
        return state_->HasSourceCode();
    }

private:
    using Opcode = LineNumberProgramItem::Opcode;

    void ProcessVars()
    {
        for (auto &var : lvt_) {
            if (var.endOffset == 0) {
                var.endOffset = state_->GetAddress();
            }
        }
    }

    LineProgramState *state_;
    LineNumberTable lnt_;
    LocalVariableTable lvt_;
    ColumnNumberTable cnt_;
};

std::vector<DebugInfoExtractor::ParamInfo> DebugInfoExtractor::EnumerateParameters(
    const File *pf, ProtoDataAccessor &pda, DebugInfoDataAccessor &dda, MethodDataAccessor &mda, ClassDataAccessor &cda)
{
    std::vector<ParamInfo> paramInfo;

    size_t idx = 0;
    size_t idxRef = pda.GetReturnType().IsReference() ? 1 : 0;
    bool firstParam = true;
    dda.EnumerateParameters([&](File::EntityId &paramId) {
        ParamInfo info;
        if (!paramId.IsValid()) {
            return;
        }
        info.name = utf::Mutf8AsCString(pf->GetStringData(paramId).data);
        if (firstParam && !mda.IsStatic()) {
            info.signature = utf::Mutf8AsCString(pf->GetStringData(cda.GetClassId()).data);
        } else {
            Type paramType = pda.GetArgType(idx++);
            if (paramType.IsPrimitive()) {
                info.signature = Type::GetSignatureByTypeId(paramType);
            } else {
                auto refType = pda.GetReferenceType(idxRef++);
                info.signature = utf::Mutf8AsCString(pf->GetStringData(refType).data);
            }
        }
        firstParam = false;
        paramInfo.emplace_back(info);
    });

    return paramInfo;
}

void DebugInfoExtractor::Extract(const File *pf)
{
    auto classes = pf->GetClasses();
    for (size_t i = 0; i < classes.Size(); i++) {
        File::EntityId id(classes[i]);
        if (pf->IsExternal(id)) {
            continue;
        }

        ClassDataAccessor cda(*pf, id);

        auto sourceFileId = cda.GetSourceFileId();

        cda.EnumerateMethods([this, pf, &sourceFileId, &cda](MethodDataAccessor &mda) {
            auto debugInfoId = mda.GetDebugInfoId();
            if (!debugInfoId) {
                return;
            }

            DebugInfoDataAccessor dda(*pf, debugInfoId.value());
            ProtoDataAccessor pda(*pf, mda.GetProtoId());

            auto paramInfo = EnumerateParameters(pf, pda, dda, mda, cda);

            LineProgramState state(*pf, sourceFileId.value_or(File::EntityId(0)), dda.GetLineStart(),
                                   dda.GetConstantPool());

            LineNumberProgramHandler handler(&state);
            LineNumberProgramProcessor<LineNumberProgramHandler> programProcessor(dda.GetLineNumberProgram(), &handler);
            programProcessor.Process();

            File::EntityId methodId = mda.GetMethodId();
            const char *sourceFile = handler.HasFile() ? utf::Mutf8AsCString(handler.GetFile()) : "";
            const char *sourceCode = handler.HasSourceCode() ? utf::Mutf8AsCString(handler.GetSourceCode()) : "";
            methods_.emplace(methodId, MethodDebugInfo {sourceFile, sourceCode, methodId, handler.GetLineNumberTable(),
                                                        handler.GetLocalVariableTable(), std::move(paramInfo),
                                                        handler.GetColumnNumberTable()});
        });
    }
}

const LineNumberTable &DebugInfoExtractor::GetLineNumberTable(File::EntityId methodId) const
{
    auto it = methods_.find(methodId);
    if (it != methods_.end()) {
        return it->second.lineNumberTable;
    }

    static const LineNumberTable EMPTY_LINE_TABLE {};  // NOLINT(fuchsia-statically-constructed-objects)
    return EMPTY_LINE_TABLE;
}

const ColumnNumberTable &DebugInfoExtractor::GetColumnNumberTable(File::EntityId methodId) const
{
    auto it = methods_.find(methodId);
    if (it != methods_.end()) {
        return it->second.columnNumberTable;
    }

    static const ColumnNumberTable EMPTY_COLUMN_TABLE {};  // NOLINT(fuchsia-statically-constructed-objects)
    return EMPTY_COLUMN_TABLE;
}

const LocalVariableTable &DebugInfoExtractor::GetLocalVariableTable(File::EntityId methodId) const
{
    auto it = methods_.find(methodId);
    if (it != methods_.end()) {
        return it->second.localVariableTable;
    }

    static const LocalVariableTable EMPTY_VARIABLE_TABLE {};  // NOLINT(fuchsia-statically-constructed-objects)
    return EMPTY_VARIABLE_TABLE;
}

const std::vector<DebugInfoExtractor::ParamInfo> &DebugInfoExtractor::GetParameterInfo(File::EntityId methodId) const
{
    auto it = methods_.find(methodId);
    if (it != methods_.end()) {
        return it->second.paramInfo;
    }

    static const std::vector<ParamInfo> EMPTY_PARAM_INFO {};  // NOLINT(fuchsia-statically-constructed-objects)
    return EMPTY_PARAM_INFO;
}

const char *DebugInfoExtractor::GetSourceFile(File::EntityId methodId) const
{
    auto it = methods_.find(methodId);
    if (it != methods_.end()) {
        return it->second.sourceFile.c_str();
    }
    return "";
}

const char *DebugInfoExtractor::GetSourceCode(File::EntityId methodId) const
{
    auto it = methods_.find(methodId);
    if (it != methods_.end()) {
        return it->second.sourceCode.c_str();
    }
    return "";
}

bool DebugInfoExtractor::IsUserFile() const
{
    return false;
}

std::vector<File::EntityId> DebugInfoExtractor::GetMethodIdList() const
{
    std::vector<File::EntityId> list;

    for (const auto &p : methods_) {
        list.push_back(p.first);
    }
    return list;
}
}  // namespace ark::panda_file
