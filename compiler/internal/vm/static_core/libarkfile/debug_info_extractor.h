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

#ifndef LIBPANDAFILE_DEBUG_INFO_EXTRACTOR_H_
#define LIBPANDAFILE_DEBUG_INFO_EXTRACTOR_H_

#include "libarkfile/file.h"
#include "libarkfile/file_items.h"
#include "libarkfile/proto_data_accessor-inl.h"
#include "libarkfile/method_data_accessor.h"
#include "libarkfile/class_data_accessor.h"
#include "libarkfile/debug_data_accessor.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace ark::panda_file {

struct LineTableEntry {
    uint32_t offset;
    size_t line;
};

struct ColumnTableEntry {
    uint32_t offset;
    size_t column;
};

using LineNumberTable = std::vector<LineTableEntry>;
using ColumnNumberTable = std::vector<ColumnTableEntry>;

struct LocalVariableInfo {
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    std::string name;
    std::string type;
    std::string typeSignature;
    int32_t regNumber;
    uint32_t startOffset;
    uint32_t endOffset;
    // NOLINTEND(misc-non-private-member-variables-in-classes)

    bool IsAccessibleAt(uint32_t offset) const
    {
        // Variable is accessible strictly after its definition
        return startOffset < offset && offset <= endOffset;
    }
};

using LocalVariableTable = std::vector<LocalVariableInfo>;

class DebugInfoExtractor {
public:
    struct ParamInfo {
        std::string name;
        std::string signature;
    };

    explicit DebugInfoExtractor(const File *pf);

    virtual ~DebugInfoExtractor() = default;

    DEFAULT_COPY_SEMANTIC(DebugInfoExtractor);
    DEFAULT_MOVE_SEMANTIC(DebugInfoExtractor);

    virtual const LineNumberTable &GetLineNumberTable(File::EntityId methodId) const;

    virtual const ColumnNumberTable &GetColumnNumberTable(File::EntityId methodId) const;

    virtual const LocalVariableTable &GetLocalVariableTable(File::EntityId methodId) const;

    virtual const std::vector<ParamInfo> &GetParameterInfo(File::EntityId methodId) const;

    virtual const char *GetSourceFile(File::EntityId methodId) const;

    virtual const char *GetSourceCode(File::EntityId methodId) const;

    virtual std::vector<File::EntityId> GetMethodIdList() const;

    virtual bool IsUserFile() const;

private:
    void Extract(const File *pf);
    std::vector<ParamInfo> EnumerateParameters(const File *pf, ProtoDataAccessor &pda, DebugInfoDataAccessor &dda,
                                               MethodDataAccessor &mda, ClassDataAccessor &cda);

    struct MethodDebugInfo {
        std::string sourceFile;
        std::string sourceCode;
        File::EntityId methodId;
        LineNumberTable lineNumberTable;
        LocalVariableTable localVariableTable;
        std::vector<ParamInfo> paramInfo;
        ColumnNumberTable columnNumberTable;
    };

    std::unordered_map<File::EntityId, MethodDebugInfo> methods_;
};

}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_DEBUG_INFO_EXTRACTOR_H_
