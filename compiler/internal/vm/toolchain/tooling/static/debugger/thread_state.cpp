/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "thread_state.h"

#include <memory>
#include <optional>
#include <sstream>

#include "include/tooling/pt_location.h"
#include "libarkbase/macros.h"
#include "breakpoint.h"
#include "error.h"
#include "types/numeric_id.h"

#include "../../hybrid_step/debug_step_flags.h"
namespace ark::tooling::inspector {

void ThreadState::Reset()
{
    stepKind_ = StepKind::BREAK_ON_START;
    stepLocations_.clear();
    methodEntered_ = false;
    skipAllPauses_ = false;
    mixedDebugEnabled_ = false;
    pauseOnExceptionsState_ = PauseOnExceptionsState::NONE;
}

void ThreadState::BreakOnStart()
{
    if (!paused_) {
        stepKind_ = StepKind::BREAK_ON_START;
    }
    breakOnStart_ = true;
}

void ThreadState::Continue()
{
    stepKind_ = StepKind::NONE;
    paused_ = false;
    pauseReason_ = PauseReason::OTHER;
}

void ThreadState::ContinueTo(std::unordered_set<PtLocation, HashLocation> locations)
{
    stepKind_ = StepKind::CONTINUE_TO;
    stepLocations_ = std::move(locations);
    paused_ = false;
    pauseReason_ = PauseReason::OTHER;
}

void ThreadState::StepInto(std::unordered_set<PtLocation, HashLocation> locations)
{
    stepKind_ = StepKind::STEP_INTO;
    methodEntered_ = false;
    stepLocations_ = std::move(locations);
    paused_ = false;
    pauseReason_ = PauseReason::STEP;
}

void ThreadState::StepOver(std::unordered_set<PtLocation, HashLocation> locations)
{
    stepKind_ = StepKind::STEP_OVER;
    methodEntered_ = false;
    stepLocations_ = std::move(locations);
    paused_ = false;
    pauseReason_ = PauseReason::STEP;
}

void ThreadState::StepOut()
{
    stepKind_ = StepKind::STEP_OUT;
    methodEntered_ = true;
    paused_ = false;
    pauseReason_ = PauseReason::STEP;
}

void ThreadState::Pause()
{
    if (!paused_ && !skipAllPauses_) {
        stepKind_ = StepKind::PAUSE;
    }
}

void ThreadState::SetSkipAllPauses(bool skip)
{
    skipAllPauses_ = skip;
}

// NOTE(fangting, #25108): implement "NativeOut" events when in mixed debug mode
void ThreadState::SetMixedDebugEnabled(bool mixedDebugEnabled)
{
    mixedDebugEnabled_ = mixedDebugEnabled;
}

void ThreadState::SetPauseOnExceptions(PauseOnExceptionsState state)
{
    pauseOnExceptionsState_ = state;
}

void ThreadState::OnException(bool uncaught)
{
    ASSERT(!paused_);
    if (skipAllPauses_) {
        return;
    }
    switch (pauseOnExceptionsState_) {
        case PauseOnExceptionsState::NONE:
            break;
        case PauseOnExceptionsState::CAUGHT:
            paused_ = !uncaught;
            break;
        case PauseOnExceptionsState::UNCAUGHT:
            paused_ = uncaught;
            break;
        case PauseOnExceptionsState::ALL:
            paused_ = true;
            break;
    }
    if (paused_) {
        pauseReason_ = PauseReason::EXCEPTION;
    }
}

void ThreadState::OnFramePop()
{
    ASSERT(!paused_);
    switch (stepKind_) {
        case StepKind::NONE:
        case StepKind::BREAK_ON_START:
        case StepKind::CONTINUE_TO:
        case StepKind::PAUSE:
        case StepKind::STEP_INTO: {
            break;
        }

        case StepKind::STEP_OUT:
        case StepKind::STEP_OVER: {
            methodEntered_ = false;
            break;
        }
    }
}

bool ThreadState::OnMethodEntry()
{
    ASSERT(!paused_);
    switch (stepKind_) {
        case StepKind::NONE:
        case StepKind::BREAK_ON_START:
        case StepKind::CONTINUE_TO:
        case StepKind::PAUSE:
        case StepKind::STEP_INTO: {
            return false;
        }

        case StepKind::STEP_OUT:
        case StepKind::STEP_OVER: {
            return !std::exchange(methodEntered_, true);
        }
    }

    UNREACHABLE();
}

void ThreadState::OnSingleStep(const PtLocation &location, const char *sourceFile)
{
    ASSERT(!paused_);

    if (breakOnStart_) {
        std::string_view file = sourceFile;
        if (sourceFiles_.find(file) == sourceFiles_.end()) {
            sourceFiles_.emplace(file);
            stepKind_ = StepKind::BREAK_ON_START;
            paused_ = true;
            pauseReason_ = PauseReason::BREAK_ON_START;
            return;
        }
    }

    DebugStepFlags::Get().SetStat2DynInto(true);
    if (DebugStepFlags::Get().GetDyn2StatInto()) {
        LOG(DEBUG, DEBUGGER) << "ThreadState::OnSingleStep SingleStep from Dynamic";
        paused_ = true;
        DebugStepFlags::Get().SetDyn2StatInto(false);
        return;
    }

    if (ShouldStopAtBreakpoint(location)) {
        paused_ = true;
        return;
    }

    switch (stepKind_) {
        case StepKind::NONE: {
            paused_ = false;
            break;
        }

        case StepKind::BREAK_ON_START: {
            paused_ = true;
            break;
        }

        case StepKind::CONTINUE_TO: {
            paused_ = stepLocations_.find(location) != stepLocations_.end();
            break;
        }

        case StepKind::PAUSE: {
            paused_ = true;
            break;
        }

        case StepKind::STEP_INTO: {
            paused_ = stepLocations_.find(location) == stepLocations_.end();
            break;
        }

        case StepKind::STEP_OUT: {
            paused_ = !methodEntered_;
            break;
        }

        case StepKind::STEP_OVER: {
            paused_ = !methodEntered_ && stepLocations_.find(location) == stepLocations_.end();
            break;
        }
    }
}

bool ThreadState::ShouldStopAtBreakpoint(const PtLocation &location)
{
    if (skipAllPauses_) {
        return false;
    }
    return bpStorage_.ShouldStopAtBreakpoint(location, engine_);
}

std::vector<BreakpointId> ThreadState::GetBreakpointsByLocation(const PtLocation &location) const
{
    return bpStorage_.GetBreakpointsByLocation(location);
}
}  // namespace ark::tooling::inspector
