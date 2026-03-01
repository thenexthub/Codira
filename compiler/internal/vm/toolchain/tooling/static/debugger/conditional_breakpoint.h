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

#ifndef PANDA_TOOLING_INSPECTOR_CONDITIONAL_BREAKPOINT_H
#define PANDA_TOOLING_INSPECTOR_CONDITIONAL_BREAKPOINT_H

#include <algorithm>
#include <functional>
#include <unordered_set>

#include "breakpoint_base.h"

namespace ark::tooling::inspector {
class EvaluationEngine;

/// @brief Conditional breakpoint, allows only one location, condition is evaluated on breakpoint hit
class ConditionalBreakpoint final : public BreakpointBase {
public:
    explicit ConditionalBreakpoint(BreakpointId id, SourceFileFilter &&filter,
                                   int32_t line, const std::string *bytecode)
        : BreakpointBase(id), sourceFileFilter_(std::move(filter)), lineNumber_(line), bytecode_(*bytecode)
    {
    }

    NO_COPY_SEMANTIC(ConditionalBreakpoint);
    NO_MOVE_SEMANTIC(ConditionalBreakpoint);

    ~ConditionalBreakpoint() override = default;

    bool SetLocations(std::set<std::string_view> &sourceFiles, const DebugInfoCache &debugCache,
                      std::unordered_multimap<PtLocation, BreakpointId, HashLocation> &breakpointLocations) override;

    void EnumerateLocations(const std::function<bool(BreakpointId, const PtLocation &)> &func) override
    {
        if (location_) {
            func(GetId(), *location_);
        }
    }

    bool ShouldStopAt(const PtLocation &location, EvaluationEngine &engine) override;

protected:
    void TryResolveImpl(const panda_file::File &file, const panda_file::DebugInfoExtractor *debugInfo,
                        std::unordered_multimap<PtLocation, BreakpointId, HashLocation> &breakpointLocations) override;

private:
    std::optional<PtLocation> location_;
    SourceFileFilter sourceFileFilter_;
    int32_t lineNumber_ {0};
    std::string bytecode_;
    Method *method_ {nullptr};
};

}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_CONDITIONAL_BREAKPOINT_H
