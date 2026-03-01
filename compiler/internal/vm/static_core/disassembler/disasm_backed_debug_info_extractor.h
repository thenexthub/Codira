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

#ifndef PANDA_DISASM_BACKED_DEBUG_INFO_EXTRACTOR_H
#define PANDA_DISASM_BACKED_DEBUG_INFO_EXTRACTOR_H

#include "disassembler.h"

namespace ark::disasm {
class PANDA_PUBLIC_API DisasmBackedDebugInfoExtractor : public panda_file::DebugInfoExtractor {
public:
    explicit DisasmBackedDebugInfoExtractor(
        const panda_file::File &file,
        std::function<void(panda_file::File::EntityId, std::string_view)> &&onDisasmSourceName = [](auto, auto) {});

    const panda_file::LineNumberTable &GetLineNumberTable(panda_file::File::EntityId methodId) const override;
    const panda_file::ColumnNumberTable &GetColumnNumberTable(panda_file::File::EntityId methodId) const override;
    const panda_file::LocalVariableTable &GetLocalVariableTable(panda_file::File::EntityId methodId) const override;
    const std::vector<ParamInfo> &GetParameterInfo(panda_file::File::EntityId methodId) const override;
    const char *GetSourceFile(panda_file::File::EntityId methodId) const override;
    const char *GetSourceCode(panda_file::File::EntityId methodId) const override;
    bool IsUserFile() const override;
    void SetUserFile(bool isUser);

private:
    struct Disassembly {
        std::string sourceCode;
        panda_file::LineNumberTable lineTable;
        std::vector<ParamInfo> parameterInfo;
        panda_file::LocalVariableTable localVariableTable;
    };

    const std::optional<std::string> &GetDisassemblySourceName(panda_file::File::EntityId methodId) const;
    const Disassembly &GetDisassembly(panda_file::File::EntityId methodId) const;

    const std::string sourceNamePrefix_;
    const std::function<void(panda_file::File::EntityId, std::string_view)> onDisasmSourceName_;

    mutable os::memory::Mutex mutex_;
    mutable Disassembler disassembler_ GUARDED_BY(mutex_);
    mutable std::unordered_map<panda_file::File::EntityId, std::optional<std::string>> sourceNames_ GUARDED_BY(mutex_);
    mutable std::unordered_map<panda_file::File::EntityId, Disassembly> disassemblies_ GUARDED_BY(mutex_);
    bool isUserFile_ = false;
};
}  // namespace ark::disasm

#endif  // PANDA_DISASM_BACKED_DEBUG_INFO_EXTRACTOR_H
