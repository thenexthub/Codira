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

#include "breakpoint_storage.h"

#include "breakpoint.h"
#include "conditional_breakpoint.h"
#include "libarkbase/os/mutex.h"

namespace ark::tooling::inspector {

bool BreakpointStorage::ShouldStopAtBreakpoint(const PtLocation &location, EvaluationEngine &engine)
{
    os::memory::ReadLockHolder lock(lock_);

    if (!breakpointsActive_) {
        return false;
    }

    auto rng = breakpointLocations_.equal_range(location);

    for (auto iter = rng.first; iter != rng.second; ++iter) {
        BreakpointId bpId = iter->second;
        bool shouldStop = breakpointStorage_[bpId]->ShouldStopAt(location, engine);
        if (shouldStop) {
            return true;
        }
    }
    return false;
}

void BreakpointStorage::ResolveBreakpoints(const panda_file::File &file,
                                           const panda_file::DebugInfoExtractor *debugInfoCache)
{
    os::memory::ReadLockHolder lock(lock_);

    for (auto &[_, bp] : breakpointStorage_) {
        bp->TryResolve(file, debugInfoCache, breakpointLocations_);
        // if bp resolved add locations
    }
}

void BreakpointStorage::RemoveBreakpoint(BreakpointId id)
{
    os::memory::WriteLockHolder lock(lock_);

    for (auto it = breakpointLocations_.begin(); it != breakpointLocations_.end();) {
        if (it->second == id) {
            it = breakpointLocations_.erase(it);
        } else {
            ++it;
        }
    }
    breakpointStorage_.erase(id);
}

std::optional<BreakpointId> BreakpointStorage::SetBreakpoint(SourceFileFilter &&sourceFilesFilter, int32_t lineNumber,
                                                             std::set<std::string_view> &sourceFiles,
                                                             const std::string *condition,
                                                             const DebugInfoCache &debugCache)
{
    os::memory::WriteLockHolder lock(lock_);

    auto id = nextBreakpointId_;
    std::unique_ptr<BreakpointBase> bbp;

    if (condition == nullptr) {
        bbp = std::make_unique<Breakpoint>(id, std::move(sourceFilesFilter), lineNumber, true);
    } else {
        bbp = std::make_unique<ConditionalBreakpoint>(id, std::move(sourceFilesFilter), lineNumber, condition);
    }

    if (!bbp->SetLocations(sourceFiles, debugCache, breakpointLocations_)) {
        // Only if many locations for conditional bp, it will be destroyed, no need to increase id.
        return std::nullopt;
    }
    // bp with correct locations, or pending with no locations
    ++nextBreakpointId_;
    breakpointStorage_[id] = std::move(bbp);
    return id;
}

void BreakpointStorage::Reset()
{
    os::memory::WriteLockHolder lock(lock_);

    breakpointsActive_ = true;
    nextBreakpointId_ = 0;
    breakpointLocations_.clear();
    breakpointStorage_.clear();
}

void BreakpointStorage::SetBreakpointsActive(bool active)
{
    os::memory::WriteLockHolder lock(lock_);

    breakpointsActive_ = active;
}

std::vector<BreakpointId> BreakpointStorage::GetBreakpointsByLocation(const PtLocation &location) const
{
    os::memory::ReadLockHolder lock(lock_);

    std::vector<BreakpointId> hitBreakpoints;

    auto range = breakpointLocations_.equal_range(location);
    std::transform(range.first, range.second, std::back_inserter(hitBreakpoints), [](auto &p) { return p.second; });

    return hitBreakpoints;
}

void BreakpointStorage::RemoveBreakpoints(const std::function<bool(const PtLocation &loc)> &filter)
{
    os::memory::WriteLockHolder lock(lock_);

    std::vector<BreakpointId> breakpointsToRemove;
    for (const auto &[loc, id] : breakpointLocations_) {
        if (filter(loc)) {
            breakpointsToRemove.emplace_back(id);
        }
    }

    for (auto id : breakpointsToRemove) {
        for (auto it = breakpointLocations_.begin(); it != breakpointLocations_.end();) {
            if (it->second == id) {
                it = breakpointLocations_.erase(it);
            } else {
                ++it;
            }
        }
        breakpointStorage_.erase(id);
    }
}

}  // namespace ark::tooling::inspector
