/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_TOOLING_INSPECTOR_BREAKPOINT_STORAGE_H
#define PANDA_TOOLING_INSPECTOR_BREAKPOINT_STORAGE_H

#include <algorithm>
#include <functional>
#include <unordered_set>

#include "breakpoint_base.h"
#include "types/numeric_id.h"

namespace ark::tooling::inspector {

/**
 * @brief Blocking breakpoint storage class, adds functionality to add/remove/enable/disable breakpoints,
 * Allows threads to check if it should stop at location
 */
class BreakpointStorage final {
public:
    BreakpointStorage() = default;

    NO_COPY_SEMANTIC(BreakpointStorage);
    NO_MOVE_SEMANTIC(BreakpointStorage);

    ~BreakpointStorage() = default;

    /**
     * @brief Enable/disable breakpoints
     * @param[in] active bool arg
     */
    void SetBreakpointsActive(bool active);

    /**
     * @brief Get breakpoint id vector by location
     * @param[in] location
     * @returns std::vector of ids
     */
    std::vector<BreakpointId> GetBreakpointsByLocation(const PtLocation &location) const;

    /**
     * @brief Set a breakpoint with optional condition.
     * @param[in] sourceFilesFilter handler to filter src files.
     * @param[in] lineNumber line in filtered src files.
     * @param[out] sourceFiles returns sourceFiles where breakpoint was set
     * @param[in] condition code fragment to compiled and evaluated for conditional bp
     * @param[in] debugCache debug cache to link src files with bytecode executable
     * @returns BreakpointId of set breakpoint.
     */
    std::optional<BreakpointId> SetBreakpoint(SourceFileFilter &&sourceFilesFilter, int32_t lineNumber,
                                              std::set<std::string_view> &sourceFiles, const std::string *condition,
                                              const DebugInfoCache &debugCache);

    /**
     * @brief Removes breakpoint by id
     * @param[in] id
     */
    void RemoveBreakpoint(BreakpointId id);

    /**
     * @brief Check if should stop at location
     * @param[in] location
     * @param[in] engine to evaluate conditions for conditional breakpoint
     * @returns true/false
     */
    bool ShouldStopAtBreakpoint(const PtLocation &location, EvaluationEngine &engine);

    /**
     * @brief Resolves pending breakpoints on new pandafile load
     * @param[in] file loaded file
     * @param[in] debugInfoCache debugcache with already added file
     */
    void ResolveBreakpoints(const panda_file::File &file, const panda_file::DebugInfoExtractor *debugInfoCache);

    /// @brief Resets storage to empty state
    void Reset();

    /**
     * @brief Removes breakpoint by filter function
     * @param[in] filter function to apply to every breakpoint
     */
    void RemoveBreakpoints(const std::function<bool(const PtLocation &loc)> &filter);

private:
    mutable os::memory::RWLock lock_;
    bool breakpointsActive_ GUARDED_BY(lock_) {true};
    BreakpointId nextBreakpointId_ GUARDED_BY(lock_) = 0;
    std::unordered_multimap<PtLocation, BreakpointId, HashLocation> breakpointLocations_ GUARDED_BY(lock_);
    std::unordered_map<BreakpointId, std::unique_ptr<BreakpointBase>> breakpointStorage_ GUARDED_BY(lock_);
};

}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_BREAKPOINT_STORAGE_H
