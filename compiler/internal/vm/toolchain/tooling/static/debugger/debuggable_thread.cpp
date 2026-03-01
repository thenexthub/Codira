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

#include "debuggable_thread.h"

#include "breakpoint_storage.h"
#include "error.h"
#include "include/method.h"
#include "types/numeric_id.h"

namespace ark::tooling::inspector {
DebuggableThread::DebuggableThread(ManagedThread *thread, DebugInterface *debugger, SuspensionCallbacks &&callbacks,
                                   BreakpointStorage &bpStorage)
    : PtThreadEvaluationEngine(debugger, thread), callbacks_(std::move(callbacks)), state_(*this, bpStorage)
{
}

void DebuggableThread::Reset()
{
    os::memory::LockHolder lock(mutex_);
    state_.Reset();
}

void DebuggableThread::BreakOnStart()
{
    os::memory::LockHolder lock(mutex_);
    state_.BreakOnStart();
}

void DebuggableThread::Continue()
{
    os::memory::LockHolder lock(mutex_);
    state_.Continue();
    Resume();
}

void DebuggableThread::ContinueTo(std::unordered_set<PtLocation, HashLocation> locations)
{
    os::memory::LockHolder lock(mutex_);
    state_.ContinueTo(std::move(locations));
    Resume();
}

void DebuggableThread::StepInto(std::unordered_set<PtLocation, HashLocation> locations)
{
    os::memory::LockHolder lock(mutex_);
    state_.StepInto(std::move(locations));
    Resume();
}

void DebuggableThread::StepOver(std::unordered_set<PtLocation, HashLocation> locations)
{
    os::memory::LockHolder lock(mutex_);
    state_.StepOver(std::move(locations));
    Resume();
}

void DebuggableThread::StepOut()
{
    os::memory::LockHolder lock(mutex_);
    state_.StepOut();
    Resume();
}

bool DebuggableThread::IsPaused()
{
    os::memory::LockHolder lock(mutex_);
    return suspended_;
}

void DebuggableThread::Touch()
{
    os::memory::LockHolder lock(mutex_);
    Resume();
}

void DebuggableThread::Pause()
{
    os::memory::LockHolder lock(mutex_);
    state_.Pause();
}

void DebuggableThread::SetSkipAllPauses(bool skip)
{
    os::memory::LockHolder lock(mutex_);
    state_.SetSkipAllPauses(skip);
}

void DebuggableThread::SetMixedDebugEnabled(bool mixedDebugEnabled)
{
    os::memory::LockHolder lock(mutex_);
    state_.SetMixedDebugEnabled(mixedDebugEnabled);
}

void DebuggableThread::SetPauseOnExceptions(PauseOnExceptionsState state)
{
    os::memory::LockHolder lock(mutex_);
    state_.SetPauseOnExceptions(state);
}

bool DebuggableThread::RequestToObjectRepository(std::function<void(ObjectRepository &)> request)
{
    os::memory::LockHolder lock(mutex_);

    ASSERT(!request_.has_value());

    if (!suspended_) {
        return false;
    }

    ASSERT(GetManagedThread()->IsSuspended());

    request_ = std::move(request);
    GetManagedThread()->Resume();

    while (request_) {
        requestDone_.Wait(&mutex_);
    }

    ASSERT(suspended_);
    ASSERT(GetManagedThread()->IsSuspended());

    return true;
}

Expected<std::pair<RemoteObject, std::optional<RemoteObject>>, std::string> DebuggableThread::EvaluateExpression(
    uint32_t frameNumber, const ExpressionWrapper &bytecode)
{
    std::optional<RemoteObject> optResult;
    std::optional<RemoteObject> optException;
    std::optional<Error> optError;
    RequestToObjectRepository(
        [this, frameNumber, &bytecode, &optResult, &optException, &optError](ObjectRepository &objectRepo) {
            Method *method = nullptr;
            auto res = EvaluateExpression(frameNumber, bytecode, &method);
            if (!res) {
                HandleError(res.Error());
                optError = res.Error();
                return;
            }

            auto [result, exc] = res.Value();
            optResult.emplace(objectRepo.CreateObject(result));
            if (exc != nullptr) {
                optException.emplace(objectRepo.CreateObject(TypedValue::Reference(exc)));
            }
        });
    if (optError) {
        ASSERT(!optResult);
        return Unexpected(optError->GetMessage());
    }
    return std::make_pair(*optResult, optException);
}

void DebuggableThread::OnException(bool uncaught)
{
    if (IsEvaluating()) {
        return;
    }
    os::memory::LockHolder lock(mutex_);
    state_.OnException(uncaught);
    while (state_.IsPaused()) {
        ObjectRepository objectRepository;
        Suspend(objectRepository, {}, GetManagedThread()->GetException(), state_.GetPauseReason());
    }
}

void DebuggableThread::OnFramePop()
{
    if (IsEvaluating()) {
        return;
    }
    os::memory::LockHolder lock(mutex_);
    state_.OnFramePop();
}

bool DebuggableThread::OnMethodEntry()
{
    if (IsEvaluating()) {
        return false;
    }
    os::memory::LockHolder lock(mutex_);
    return state_.OnMethodEntry();
}

void DebuggableThread::OnSingleStep(const PtLocation &location, const char *sourceFile)
{
    if (IsEvaluating()) {
        return;
    }
    os::memory::LockHolder lock(mutex_);
    state_.OnSingleStep(location, sourceFile);
    while (state_.IsPaused()) {
        ObjectRepository objectRepository;
        auto hitBreakpoints = state_.GetBreakpointsByLocation(location);
        Suspend(objectRepository, hitBreakpoints, {}, state_.GetPauseReason());
    }
}

std::vector<RemoteObject> DebuggableThread::OnConsoleCall(const PandaVector<TypedValue> &arguments)
{
    std::vector<RemoteObject> result;

    ObjectRepository objectRepository;
    std::transform(arguments.begin(), arguments.end(), std::back_inserter(result),
                   [&objectRepository](auto value) { return objectRepository.CreateObject(value); });

    return result;
}

void DebuggableThread::Suspend(ObjectRepository &objectRepository, const std::vector<BreakpointId> &hitBreakpoints,
                               ObjectHeader *exception, PauseReason pauseReason)
{
    ASSERT(ManagedThread::GetCurrent() == GetManagedThread());

    ASSERT(!suspended_);
    ASSERT(!request_.has_value());

    callbacks_.preSuspend(objectRepository, hitBreakpoints, exception);

    suspended_ = true;
    GetManagedThread()->Suspend();

    callbacks_.postSuspend(objectRepository, hitBreakpoints, exception, pauseReason);

    while (suspended_) {
        mutex_.Unlock();

        callbacks_.preWaitSuspension();
        GetManagedThread()->WaitSuspension();
        callbacks_.postWaitSuspension();

        mutex_.Lock();

        // We have three levels of suspension:
        // - state_.IsPaused() - tells if the thread is paused on breakpoint or step;
        // - suspended_ - tells if the thread generally sleeps (but could execute object repository requests);
        // - GetManagedThread()->IsSuspended() - tells if the thread is actually sleeping
        //
        // If we are here, then the latter is false (thread waked up). The following variants are possible:
        // - not paused and not suspended - e.g. continue / stepping action was performed;
        // - not paused and suspended - invalid;
        // - paused and not suspended - touch was performed, resume and sleep back
        //     (used to notify about the state on `runIfWaitingForDebugger`);
        // - paused and suspended - object repository request was made.

        ASSERT(suspended_ == request_.has_value());

        if (request_) {
            (*request_)(objectRepository);

            request_.reset();
            GetManagedThread()->Suspend();

            requestDone_.Signal();
        }
    }
}

void DebuggableThread::Resume()
{
    ASSERT(!request_.has_value());

    if (!suspended_) {
        return;
    }

    ASSERT(GetManagedThread()->IsSuspended());

    callbacks_.preResume();

    suspended_ = false;
    GetManagedThread()->Resume();

    callbacks_.postResume();
}

bool DebuggableThread::IsPausedByBreakOnStart()
{
    os::memory::LockHolder lock(mutex_);
    return state_.IsPaused() && (state_.GetPauseReason() == PauseReason::BREAK_ON_START);
}

bool DebuggableThread::IsPausedByBreakPoint()
{
    os::memory::LockHolder lock(mutex_);
    return state_.IsPaused() && (state_.GetPauseReason() == PauseReason::OTHER);
}
}  // namespace ark::tooling::inspector
