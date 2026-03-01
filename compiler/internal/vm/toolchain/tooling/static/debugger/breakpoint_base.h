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

#ifndef PANDA_TOOLING_INSPECTOR_BREAKPOINT_BASE_H
#define PANDA_TOOLING_INSPECTOR_BREAKPOINT_BASE_H

#include <algorithm>
#include <functional>
#include <unordered_set>

#include "common.h"
#include "debug_info_cache.h"
#include "tooling/debugger.h"
#include "types/numeric_id.h"

namespace ark::tooling::inspector {
class EvaluationEngine;

/// @brief Base breakpoint class
class BreakpointBase {
public:
    explicit BreakpointBase(BreakpointId id) : id_(id) {}

    DEFAULT_COPY_SEMANTIC(BreakpointBase);
    DEFAULT_MOVE_SEMANTIC(BreakpointBase);

    virtual ~BreakpointBase() = default;

    /**
     * @brief Get breakpoint id
     * @returns BreakpointId
     */
    BreakpointId GetId() const
    {
        return id_;
    }

    /**
     * @brief If breakpoint resolved and can be hit
     * @returns true/false
     */
    bool IsResolved() const
    {
        return resolved_;
    }

    /**
     * @brief Tries to resolve breakpoint, shuould be called on load new panda file
     * @param[in] file loaded panda file
     * @param[in] debugInfo extractor with already added file
     * @returns Was breakpoint resolved
     */
    bool TryResolve(const panda_file::File &file, const panda_file::DebugInfoExtractor *debugInfo,
                    std::unordered_multimap<PtLocation, BreakpointId, HashLocation> &breakpointLocations)
    {
        TryResolveImpl(file, debugInfo, breakpointLocations);
        return IsResolved();
    }

    /**
     * @brief Applies function to every breakpoint location
     * @param[in] func function to apply to every breakpoint
     */
    virtual void EnumerateLocations(const std::function<bool(BreakpointId, const PtLocation &)> &func) = 0;

    /**
     * @brief Check if should stop at location
     * @param[in] location
     * @param[in] engine to evaluate conditions for conditional breakpoint
     * @returns true/false
     */
    virtual bool ShouldStopAt(const PtLocation &location, EvaluationEngine &engine) = 0;

    /**
     * @brief Set locations for current breakpoint
     * @param[out] sourceFiles returns sourceFiles where breakpoint was set
     * @param[in] debugCache debug cache to link src files with bytecode executable
     * @param[out] breakpointLocations map of breakpoint locations from storage
     * @returns BreakpointId of set breakpoint.
     */
    virtual bool SetLocations(std::set<std::string_view> &sourceFiles, const DebugInfoCache &debugCache,
                              std::unordered_multimap<PtLocation, BreakpointId, HashLocation> &breakpointLocations) = 0;

protected:
    virtual void TryResolveImpl(
        const panda_file::File &file, const panda_file::DebugInfoExtractor *debugInfo,
        std::unordered_multimap<PtLocation, BreakpointId, HashLocation> &breakpointLocations) = 0;

    void Resolve()
    {
        resolved_ = true;
    }

private:
    BreakpointId id_;
    bool resolved_ {false};
};

}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_BREAKPOINT_BASE_H
