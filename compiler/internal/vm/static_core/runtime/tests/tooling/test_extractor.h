/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_RUNTIME_TESTS_TOOLING_TEST_EXTRACTOR_H
#define PANDA_RUNTIME_TESTS_TOOLING_TEST_EXTRACTOR_H

#include "libarkfile/debug_info_extractor.h"
#include "libarkfile/line_number_program.h"
#include "runtime/include/tooling/debug_interface.h"

namespace ark::tooling::test {
using EntityId = panda_file::File::EntityId;

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct SourceLocation {
    PandaString path;  // NOLINT(misc-non-private-member-variables-in-classes)
    size_t line;       // NOLINT(misc-non-private-member-variables-in-classes)

    bool operator==(const SourceLocation &other) const
    {
        return path == other.path && line == other.line;
    }

    bool IsValid() const
    {
        return !path.empty();
    }
};

class TestExtractor {
public:
    TestExtractor() = default;

    explicit TestExtractor(const panda_file::File *pf);

    virtual ~TestExtractor() = default;

    std::pair<EntityId, uint32_t> GetBreakpointAddress(const SourceLocation &sourceLocation);

    PandaList<PtStepRange> GetStepRanges(EntityId methodId, uint32_t currentOffset);

    virtual std::vector<panda_file::LocalVariableInfo> GetLocalVariableInfo(EntityId methodId, size_t offset);
    const std::vector<panda_file::DebugInfoExtractor::ParamInfo> &GetParameterInfo(EntityId methodId);

    SourceLocation GetSourceLocation(EntityId methodId, uint32_t bytecodeOffset);

    static std::optional<size_t> GetLineNumberByTableOffset(const panda_file::LineNumberTable &table, uint32_t offset);
    static std::optional<uint32_t> GetOffsetByTableLineNumber(const panda_file::LineNumberTable &table, size_t line);

    NO_COPY_SEMANTIC(TestExtractor);
    NO_MOVE_SEMANTIC(TestExtractor);

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    PandaUniquePtr<panda_file::DebugInfoExtractor> langExtractor_;
};

class TestExtractorFactory {
public:
    TestExtractorFactory() = default;
    virtual ~TestExtractorFactory() = default;

    virtual PandaUniquePtr<TestExtractor> MakeTestExtractor(const panda_file::File *pf)
    {
        return MakePandaUnique<TestExtractor>(pf);
    }

    NO_COPY_SEMANTIC(TestExtractorFactory);
    NO_MOVE_SEMANTIC(TestExtractorFactory);
};
}  // namespace ark::tooling::test

#endif  // PANDA_RUNTIME_TESTS_TOOLING_TEST_EXTRACTOR_H
