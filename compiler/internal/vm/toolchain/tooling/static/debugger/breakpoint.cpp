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

#include "breakpoint.h"

#include "libarkfile/debug_info_extractor.h"
#include "error.h"
#include "evaluation/evaluation_engine.h"

namespace ark::tooling::inspector {

bool Breakpoint::SetLocations(std::set<std::string_view> &sourceFiles, const DebugInfoCache &debugCache,
                              std::unordered_multimap<PtLocation, BreakpointId, HashLocation> &breakpointLocations)
{
    locations_ = debugCache.GetBreakpointLocations(sourceFileFilter_, lineNumber_, sourceFiles);
    if (locations_.empty()) {
        LOG(ERROR, DEBUGGER) << "Pending breakpoint, 0 locations resolved currently, id = " << GetId();
        return false;
    }

    for (auto &location : locations_) {
        breakpointLocations.emplace(location, GetId());
    }
    Resolve();
    return true;
}

void Breakpoint::TryResolveImpl(const panda_file::File &file, const panda_file::DebugInfoExtractor *debugInfo,
                                std::unordered_multimap<PtLocation, BreakpointId, HashLocation> &breakpointLocations)
{
    auto lineHandler = [this, &breakpointLocations](const auto &pandaFile, auto methodId, auto &entry) {
        if (entry.line == lineNumber_) {
            auto [it, _] = locations_.emplace(pandaFile.GetFilename().data(), methodId, entry.offset);
            breakpointLocations.emplace(*it, GetId());
            Resolve();
            // Must choose the first found bytecode location in each method
            return false;
        }
        // Continue search
        return true;
    };

    for (const auto &methodId : debugInfo->GetMethodIdList()) {
        if (!sourceFileFilter_(debugInfo->GetSourceFile(methodId))) {
            continue;
        }

        auto &table = debugInfo->GetLineNumberTable(methodId);
        for (auto &entry : table) {
            if (!lineHandler(file, methodId, entry)) {
                break;
            }
        }
    }
}

}  // namespace ark::tooling::inspector
