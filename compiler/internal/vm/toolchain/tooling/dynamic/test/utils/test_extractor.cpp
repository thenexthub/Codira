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
// Taken from panda-tools/panda-debugger/debugger

#include "test/utils/test_extractor.h"

#include "libpandabase/utils/leb128.h"
#include "libpandabase/utils/utf.h"
#include "class_data_accessor-inl.h"
#include "code_data_accessor-inl.h"
#include "debug_data_accessor-inl.h"
#include "helpers.h"
#include "method_data_accessor-inl.h"
#include "proto_data_accessor-inl.h"

namespace panda::ecmascript::tooling::test {
std::pair<EntityId, uint32_t> TestExtractor::GetBreakpointAddress(const SourceLocation &sourceLocation)
{
    EntityId retId = EntityId();
    uint32_t retOffset = 0;
    auto callbackFunc = [&retId, &retOffset](const JSPtLocation &jsLocation) -> bool {
        retId = jsLocation.GetMethodId();
        retOffset = jsLocation.GetBytecodeOffset();
        return true;
    };
    std::unordered_set<std::string> recordName {};
    MatchWithLocation(callbackFunc, sourceLocation.line, sourceLocation.column, "", recordName);
    return {retId, retOffset};
}

SourceLocation TestExtractor::GetSourceLocation(const JSPandaFile *file, EntityId methodId, uint32_t bytecodeOffset)
{
    SourceLocation location {file, 0, 0};
    auto callbackLineFunc = [&location](int32_t line) -> bool {
        location.line = line;
        return true;
    };
    auto callbackColumnFunc = [&location](int32_t column) -> bool {
        location.column = column;
        return true;
    };
    MatchLineWithOffset(callbackLineFunc, methodId, bytecodeOffset);
    MatchColumnWithOffset(callbackColumnFunc, methodId, bytecodeOffset);
    return location;
}
}  // namespace  panda::ecmascript::tooling::test
