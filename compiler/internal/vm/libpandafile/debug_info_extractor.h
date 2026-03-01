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

#ifndef LIBPANDAFILE_DEBUG_INFO_EXTRACTOR_H
#define LIBPANDAFILE_DEBUG_INFO_EXTRACTOR_H

#include "file.h"
#include "file_items.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace panda::panda_file {

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
    std::string name;
    std::string type;
    std::string type_signature;
    int32_t reg_number;
    uint32_t start_offset;
    uint32_t end_offset;
};

using LocalVariableTable = std::vector<LocalVariableInfo>;

class DebugInfoExtractor {
public:
    struct ParamInfo {
        std::string name;
        std::string signature;
    };

    PANDA_PUBLIC_API explicit DebugInfoExtractor(const File *pf);

    ~DebugInfoExtractor() = default;

    DEFAULT_COPY_SEMANTIC(DebugInfoExtractor);
    DEFAULT_MOVE_SEMANTIC(DebugInfoExtractor);

    PANDA_PUBLIC_API const LineNumberTable &GetLineNumberTable(File::EntityId method_id) const;

    PANDA_PUBLIC_API const ColumnNumberTable &GetColumnNumberTable(File::EntityId method_id) const;

    PANDA_PUBLIC_API const LocalVariableTable &GetLocalVariableTable(File::EntityId method_id) const;

    const std::vector<ParamInfo> &GetParameterInfo(File::EntityId method_id) const;

    PANDA_PUBLIC_API const char *GetSourceFile(File::EntityId method_id) const;

    PANDA_PUBLIC_API const char *GetSourceCode(File::EntityId method_id) const;

    std::vector<File::EntityId> GetMethodIdList() const;

private:
    void Extract(const File *pf);

    struct MethodDebugInfo {
        std::string source_file;
        std::string source_code;
        File::EntityId method_id;
        LineNumberTable line_number_table;
        LocalVariableTable local_variable_table;
        std::vector<ParamInfo> param_info;
        ColumnNumberTable column_number_table;
    };

    std::unordered_map<uint32_t, MethodDebugInfo> methods_;
};

}  // namespace panda::panda_file

#endif  // LIBPANDAFILE_DEBUG_INFO_EXTRACTOR_H
