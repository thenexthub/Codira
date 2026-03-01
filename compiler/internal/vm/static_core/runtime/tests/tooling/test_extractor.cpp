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

#include "test_extractor.h"

#include <algorithm>
#include <limits>

#include "libarkbase/utils/leb128.h"
#include "libarkbase/utils/utf.h"
#include "libarkfile/class_data_accessor-inl.h"
#include "libarkfile/code_data_accessor-inl.h"
#include "libarkfile/debug_data_accessor-inl.h"
#include "libarkfile/helpers.h"
#include "libarkfile/method_data_accessor-inl.h"
#include "libarkfile/proto_data_accessor-inl.h"

namespace ark::tooling::test {

TestExtractor::TestExtractor(const panda_file::File *pf)
    : langExtractor_(MakePandaUnique<panda_file::DebugInfoExtractor>(pf))
{
}

std::pair<EntityId, uint32_t> TestExtractor::GetBreakpointAddress(const SourceLocation &sourceLocation)
{
    auto pos = sourceLocation.path.find_last_of("/\\");
    auto name = sourceLocation.path;

    if (pos != PandaString::npos) {
        name = name.substr(pos + 1);
    }

    std::vector<panda_file::File::EntityId> methods = langExtractor_->GetMethodIdList();
    for (const auto &method : methods) {
        auto srcName = PandaString(langExtractor_->GetSourceFile(method));
        auto posSf = srcName.find_last_of("/\\");
        if (posSf != PandaString::npos) {
            srcName = srcName.substr(posSf + 1);
        }
        if (srcName == name) {
            const panda_file::LineNumberTable &lineTable = langExtractor_->GetLineNumberTable(method);
            if (lineTable.empty()) {
                continue;
            }

            std::optional<size_t> offset = GetOffsetByTableLineNumber(lineTable, sourceLocation.line);
            if (offset == std::nullopt) {
                continue;
            }
            return {method, offset.value()};
        }
    }
    return {EntityId(), 0};
}

PandaList<PtStepRange> TestExtractor::GetStepRanges(EntityId methodId, uint32_t currentOffset)
{
    const panda_file::LineNumberTable &lineTable = langExtractor_->GetLineNumberTable(methodId);
    if (lineTable.empty()) {
        return {};
    }

    std::optional<size_t> line = GetLineNumberByTableOffset(lineTable, currentOffset);
    if (line == std::nullopt) {
        return {};
    }

    PandaList<PtStepRange> res;
    for (auto it = lineTable.begin(); it != lineTable.end(); ++it) {
        if (it->line == line) {
            size_t idx = it - lineTable.begin();
            if (it + 1 != lineTable.end()) {
                res.push_back({lineTable[idx].offset, lineTable[idx + 1].offset});
            } else {
                res.push_back({lineTable[idx].offset, std::numeric_limits<uint32_t>::max()});
            }
        }
    }
    return res;
}

std::vector<panda_file::LocalVariableInfo> TestExtractor::GetLocalVariableInfo(EntityId methodId, size_t offset)
{
    const std::vector<panda_file::LocalVariableInfo> &variables = langExtractor_->GetLocalVariableTable(methodId);
    std::vector<panda_file::LocalVariableInfo> result;

    for (const auto &variable : variables) {
        if (variable.startOffset <= offset && offset <= variable.endOffset) {
            result.push_back(variable);
        }
    }
    return result;
}

const std::vector<panda_file::DebugInfoExtractor::ParamInfo> &TestExtractor::GetParameterInfo(EntityId methodId)
{
    return langExtractor_->GetParameterInfo(methodId);
}

std::optional<size_t> TestExtractor::GetLineNumberByTableOffset(const panda_file::LineNumberTable &table,
                                                                uint32_t offset)
{
    for (const auto &value : table) {
        if (value.offset == offset) {
            return value.line;
        }
    }
    return std::nullopt;
}

std::optional<uint32_t> TestExtractor::GetOffsetByTableLineNumber(const panda_file::LineNumberTable &table, size_t line)
{
    for (const auto &value : table) {
        if (value.line == line) {
            return value.offset;
        }
    }
    return std::nullopt;
}

SourceLocation TestExtractor::GetSourceLocation(EntityId methodId, uint32_t bytecodeOffset)
{
    const panda_file::LineNumberTable &lineTable = langExtractor_->GetLineNumberTable(methodId);
    if (lineTable.empty()) {
        return SourceLocation();
    }

    std::optional<size_t> line = GetLineNumberByTableOffset(lineTable, bytecodeOffset);
    if (line == std::nullopt) {
        return SourceLocation();
    }

    return SourceLocation {langExtractor_->GetSourceFile(methodId), line.value()};
}
}  // namespace  ark::tooling::test
