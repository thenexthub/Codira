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

#ifndef ECMASCRIPT_TOOLING_TEST_UTILS_TEST_EXTRACTOR_H
#define ECMASCRIPT_TOOLING_TEST_UTILS_TEST_EXTRACTOR_H

#include "ecmascript/jspandafile/debug_info_extractor.h"

namespace panda::ecmascript::tooling::test {
using EntityId = panda_file::File::EntityId;

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct SourceLocation {
    const JSPandaFile *file;
    int32_t line;
    int32_t column;

    bool operator==(const SourceLocation &other) const
    {
        return file == other.file && line == other.line && column == other.column;
    }
};

class TestExtractor : public DebugInfoExtractor {
public:
    explicit TestExtractor(const JSPandaFile *pandaFile) : DebugInfoExtractor(pandaFile) {}
    ~TestExtractor() = default;

    std::pair<EntityId, uint32_t> GetBreakpointAddress(const SourceLocation &sourceLocation);

    SourceLocation GetSourceLocation(const JSPandaFile *file, EntityId methodId, uint32_t bytecodeOffset);
};
}  // namespace panda::ecmascript::tooling::test

#endif  // ECMASCRIPT_TOOLING_TEST_UTILS_TEST_EXTRACTOR_H
