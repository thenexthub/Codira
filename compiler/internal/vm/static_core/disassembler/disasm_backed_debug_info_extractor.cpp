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

#include "disasm_backed_debug_info_extractor.h"

namespace ark::disasm {
DisasmBackedDebugInfoExtractor::DisasmBackedDebugInfoExtractor(
    const panda_file::File &file,
    std::function<void(panda_file::File::EntityId, std::string_view)> &&onDisasmSourceName)
    : panda_file::DebugInfoExtractor(&file),
      sourceNamePrefix_("disasm://" + file.GetFilename() + ":"),
      onDisasmSourceName_(std::move(onDisasmSourceName))
{
    disassembler_.SetFile(file);
}

const panda_file::LineNumberTable &DisasmBackedDebugInfoExtractor::GetLineNumberTable(
    panda_file::File::EntityId methodId) const
{
    if (GetDisassemblySourceName(methodId)) {
        return GetDisassembly(methodId).lineTable;
    }

    return DebugInfoExtractor::GetLineNumberTable(methodId);
}

const panda_file::ColumnNumberTable &DisasmBackedDebugInfoExtractor::GetColumnNumberTable(
    panda_file::File::EntityId /* method_id */) const
{
    static panda_file::ColumnNumberTable empty;
    return empty;
}

const panda_file::LocalVariableTable &DisasmBackedDebugInfoExtractor::GetLocalVariableTable(
    panda_file::File::EntityId methodId) const
{
    if (GetDisassemblySourceName(methodId)) {
        return GetDisassembly(methodId).localVariableTable;
    }

    return DebugInfoExtractor::GetLocalVariableTable(methodId);
}

const std::vector<panda_file::DebugInfoExtractor::ParamInfo> &DisasmBackedDebugInfoExtractor::GetParameterInfo(
    panda_file::File::EntityId methodId) const
{
    if (GetDisassemblySourceName(methodId)) {
        return GetDisassembly(methodId).parameterInfo;
    }

    return DebugInfoExtractor::GetParameterInfo(methodId);
}

const char *DisasmBackedDebugInfoExtractor::GetSourceFile(panda_file::File::EntityId methodId) const
{
    if (auto &disassemblySourceName = GetDisassemblySourceName(methodId)) {
        return disassemblySourceName->c_str();
    }

    return DebugInfoExtractor::GetSourceFile(methodId);
}

const char *DisasmBackedDebugInfoExtractor::GetSourceCode(panda_file::File::EntityId methodId) const
{
    if (GetDisassemblySourceName(methodId)) {
        return GetDisassembly(methodId).sourceCode.c_str();
    }

    return DebugInfoExtractor::GetSourceCode(methodId);
}

const std::optional<std::string> &DisasmBackedDebugInfoExtractor::GetDisassemblySourceName(
    panda_file::File::EntityId methodId) const
{
    os::memory::LockHolder lock(mutex_);

    auto it = sourceNames_.find(methodId);
    if (it != sourceNames_.end()) {
        return it->second;
    }

    std::string_view sourceFile = DebugInfoExtractor::GetSourceFile(methodId);
    if (!sourceFile.empty()) {
        // Mark the queried method to be ignored, because debug information is available for it
        return sourceNames_.emplace(methodId, std::nullopt).first->second;
    }

    std::ostringstream name;
    name << sourceNamePrefix_ << methodId;
    auto &sourceName = sourceNames_.emplace(methodId, name.str()).first->second;

    onDisasmSourceName_(methodId, *sourceName);

    return sourceName;
}

const DisasmBackedDebugInfoExtractor::Disassembly &DisasmBackedDebugInfoExtractor::GetDisassembly(
    panda_file::File::EntityId methodId) const
{
    os::memory::LockHolder lock(mutex_);

    auto it = disassemblies_.find(methodId);
    if (it != disassemblies_.end()) {
        return it->second;
    }

    pandasm::Function method("", SourceLanguage::PANDA_ASSEMBLY);
    disassembler_.GetMethod(&method, methodId);

    std::ostringstream sourceCode;
    panda_file::LineNumberTable lineTable;
    disassembler_.Serialize(method, sourceCode, true, &lineTable);

    auto paramsNum = method.GetParamsNum();
    std::vector<ParamInfo> parameterInfo(paramsNum);
    for (auto argumentNum = 0U; argumentNum < paramsNum; ++argumentNum) {
        parameterInfo[argumentNum].name = "a" + std::to_string(argumentNum);
        parameterInfo[argumentNum].signature = method.params[argumentNum].type.GetDescriptor();
    }

    // We use -1 here as a number for Accumulator, because there is no other way
    // to specify Accumulator as a register for local variable
    auto totalRegNum = method.GetTotalRegs();
    panda_file::LocalVariableTable localVariableTable(totalRegNum + 1, panda_file::LocalVariableInfo {});
    for (auto vregNum = -1; vregNum < static_cast<int32_t>(totalRegNum); ++vregNum) {
        localVariableTable[vregNum + 1].regNumber = vregNum;
        localVariableTable[vregNum + 1].name = vregNum == -1 ? "acc" : "v" + std::to_string(vregNum);
        localVariableTable[vregNum + 1].startOffset = 0;
        localVariableTable[vregNum + 1].endOffset = UINT32_MAX;
    }

    return disassemblies_
        .emplace(methodId, Disassembly {sourceCode.str(), std::move(lineTable), std::move(parameterInfo),
                                        std::move(localVariableTable)})
        .first->second;
}

bool DisasmBackedDebugInfoExtractor::IsUserFile() const
{
    return isUserFile_;
}

void DisasmBackedDebugInfoExtractor::SetUserFile(bool isUser)
{
    isUserFile_ = isUser;
}

}  // namespace ark::disasm
