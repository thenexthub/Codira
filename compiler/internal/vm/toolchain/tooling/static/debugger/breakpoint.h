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

#ifndef PANDA_TOOLING_INSPECTOR_BREAKPOINT_H
#define PANDA_TOOLING_INSPECTOR_BREAKPOINT_H

#include <algorithm>
#include <functional>
#include <unordered_set>

#include "breakpoint_base.h"

namespace ark::tooling::inspector {
class EvaluationEngine;

/// @brief Breakpoint without condition, can be set in multiple locations
class Breakpoint final : public BreakpointBase {
public:
    explicit Breakpoint(BreakpointId id, SourceFileFilter &&filter, int32_t line, bool isUrlPattern)
        : BreakpointBase(id), sourceFileFilter_(std::move(filter)), lineNumber_(line), isUrlPattern_(isUrlPattern)
    {
    }

    NO_COPY_SEMANTIC(Breakpoint);
    NO_MOVE_SEMANTIC(Breakpoint);

    ~Breakpoint() override = default;

    bool SetLocations(std::set<std::string_view> &sourceFiles, const DebugInfoCache &debugCache,
                      std::unordered_multimap<PtLocation, BreakpointId, HashLocation> &breakpointLocations) override;

    void EnumerateLocations(const std::function<bool(BreakpointId, const PtLocation &)> &func) override
    {
        for (const auto &loc : locations_) {
            if (!func(GetId(), loc)) {
                return;
            }
        }
    }

    bool ShouldStopAt(const PtLocation &location, [[maybe_unused]] EvaluationEngine &engine) override
    {
        if (!IsResolved()) {
            return false;
        }
        return std::any_of(locations_.begin(), locations_.end(),
                           [location](const auto &loc) { return loc == location; });
    }

    bool IsUrlPattern()
    {
        return isUrlPattern_;
    }

protected:
    void TryResolveImpl(const panda_file::File &file, const panda_file::DebugInfoExtractor *debugInfo,
                        std::unordered_multimap<PtLocation, BreakpointId, HashLocation> &breakpointLocations) override;

private:
    std::unordered_set<PtLocation, HashLocation> locations_;
    SourceFileFilter sourceFileFilter_;
    int32_t lineNumber_ {0};
    bool isUrlPattern_ {false};
};

}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_BREAKPOINT_H
