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

#include "conditional_breakpoint.h"

#include "libarkfile/debug_info_extractor.h"
#include "error.h"
#include "evaluation/evaluation_engine.h"

namespace ark::tooling::inspector {

bool ConditionalBreakpoint::ShouldStopAt(const PtLocation &location, EvaluationEngine &engine)
{
    if (!location_.has_value() || !(*location_ == location)) {
        return false;
    }
    // Condition is evaluated in the context of top frame
    auto evalRes = engine.EvaluateExpression(0, bytecode_, &method_);
    if (!evalRes) {
        HandleError(evalRes.Error());
        return false;
    }
    if (evalRes.Value().second != nullptr) {
        LOG(WARNING, DEBUGGER) << "Breakpoint #" << GetId() << " condition evaluated with an exception";
        return false;
    }
    return evalRes.Value().first.GetAsU1();
}

bool ConditionalBreakpoint::SetLocations(
    std::set<std::string_view> &sourceFiles, const DebugInfoCache &debugCache,
    std::unordered_multimap<PtLocation, BreakpointId, HashLocation> &breakpointLocations)
{
    auto locations = debugCache.GetBreakpointLocations(sourceFileFilter_, lineNumber_, sourceFiles);
    if (locations.size() > 1) {
        LOG(WARNING, DEBUGGER) << "Will not set conditional breakpoint for more than one location";
        return false;
    }

    if (locations.empty()) {
        LOG(WARNING, DEBUGGER) << "Pending breakpoint, 0 locations resolved currently, id = " << GetId();
        return true;
    }

    location_ = *locations.begin();
    breakpointLocations.emplace(location_.value(), GetId());
    Resolve();
    return true;
}

void ConditionalBreakpoint::TryResolveImpl(
    const panda_file::File &file, const panda_file::DebugInfoExtractor *debugInfo,
    std::unordered_multimap<PtLocation, BreakpointId, HashLocation> &breakpointLocations)
{
    if (IsResolved()) {
        return;
    }

    auto lineHandler = [this, &breakpointLocations](const auto &pandaFile, auto methodId, auto &entry) {
        if (entry.line == lineNumber_) {
            location_.emplace(pandaFile.GetFilename().data(), methodId, entry.offset);
            breakpointLocations.emplace(location_.value(), GetId());
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
