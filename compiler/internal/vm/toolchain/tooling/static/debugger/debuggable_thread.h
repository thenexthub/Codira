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

#ifndef PANDA_TOOLING_INSPECTOR_DEBUGGABLE_THREAD_H
#define PANDA_TOOLING_INSPECTOR_DEBUGGABLE_THREAD_H

#include "common.h"
#include "debug_info_cache.h"
#include "include/managed_thread.h"
#include "include/tooling/pt_location.h"

#include "evaluation/evaluation_engine.h"
#include "object_repository.h"
#include "thread_state.h"
#include "types/numeric_id.h"
#include "types/pause_on_exceptions_state.h"

namespace ark::tooling::inspector {
/**
 * @brief Application thread abstraction.
 * Note that private inheritance is used in order not to expose evaluation-related state details.
 */
class DebuggableThread final : private PtThreadEvaluationEngine {
public:
    struct SuspensionCallbacks final {
        std::function<void(ObjectRepository &, const std::vector<BreakpointId> &, ObjectHeader *)> preSuspend;
        std::function<void(ObjectRepository &, const std::vector<BreakpointId> &, ObjectHeader *, PauseReason)>
            postSuspend;
        std::function<void()> preWaitSuspension;
        std::function<void()> postWaitSuspension;
        std::function<void()> preResume;
        std::function<void()> postResume;
    };

public:
    DebuggableThread(ManagedThread *thread, DebugInterface *debugger, SuspensionCallbacks &&callbacks,
                     BreakpointStorage &bpStorage);
    ~DebuggableThread() override = default;

    NO_COPY_SEMANTIC(DebuggableThread);
    NO_MOVE_SEMANTIC(DebuggableThread);

    /// The following methods should be called on the server thread

    // Resets the state on a new connection
    void Reset();

    // Tells a newly created thread to pause on the next step
    void BreakOnStart();

    // Continues execution of a paused thread
    void Continue();

    // Continues execution of a paused thread until it reaches one of the locations
    void ContinueTo(std::unordered_set<PtLocation, HashLocation> locations);

    // Tells a paused thread to perform a step into
    void StepInto(std::unordered_set<PtLocation, HashLocation> locations);

    // Tells a paused thread to perform a step over
    void StepOver(std::unordered_set<PtLocation, HashLocation> locations);

    // Tells a paused thread to perform a step out
    void StepOut();

    // Checks if application thread is currently paused.
    // Used by server thread for validation before executing stepping requests.
    bool IsPaused();

    // Makes a paused thread to resume and suspend back (does nothing for running threads).
    // Used to notify about thread's state when processing `runIfWaitingForDebugger`
    void Touch();

    // Tells a running thread to pause on the next step
    void Pause();

    // Enables or disables skipping all pauses
    void SetSkipAllPauses(bool skip);

    // Enables or disables mixdebug && mixstack
    void SetMixedDebugEnabled(bool mixedDebugEnabled);

    // Tells when stops should be made on exceptions
    void SetPauseOnExceptions(PauseOnExceptionsState state);

    // Executes a request to object repository on a paused thread (does nothing for running threads)
    bool RequestToObjectRepository(std::function<void(ObjectRepository &)> request);

    /// The following methods should be called on an application thread

    // Notification that an exception was thrown. Pauses the thread if necessary
    void OnException(bool uncaught);

    // Notification that an "interesting" frame was popped
    void OnFramePop();

    // Notification that a new frame was pushed. Returns true if we want to be notified about the frame is popped
    // (i.e. the frame is "interesting"), false otherwise
    bool OnMethodEntry();

    // Notification that a next step will be performed. Pauses the thread if necessary
    void OnSingleStep(const PtLocation &location, const char *sourceFile);

    // Notification that a call to console was performed. Returns its arguments presented as remote objects
    std::vector<RemoteObject> OnConsoleCall(const PandaVector<TypedValue> &arguments);

    /**
     * @brief Evaluates the given bytecode expression.
     * @param frameNumber frame depth to evaluate expression in.
     * @param bytecode fragment with expression.
     * @returns pair of result (might be void) and optional exception objects OR error message
     */
    Expected<std::pair<RemoteObject, std::optional<RemoteObject>>, std::string> EvaluateExpression(
        uint32_t frameNumber, const ExpressionWrapper &bytecode);

    // Checks if the thread was paused by BreakOnStart
    bool IsPausedByBreakOnStart();

    // Checks if the thread was paused by BreakPoint
    bool IsPausedByBreakPoint();

private:
    using PtThreadEvaluationEngine::EvaluateExpression;

    // Suspends a paused thread. Should be called on an application thread
    void Suspend(ObjectRepository &objectRepository, const std::vector<BreakpointId> &hitBreakpoints,
                 ObjectHeader *exception, PauseReason pauseReason) REQUIRES(mutex_);

    // Marks a paused thread as not suspended. Should be called on the server thread
    void Resume() REQUIRES(mutex_);

private:
    SuspensionCallbacks callbacks_;

    os::memory::Mutex mutex_;
    ThreadState state_ GUARDED_BY(mutex_);
    bool suspended_ GUARDED_BY(mutex_) {false};
    std::optional<std::function<void(ObjectRepository &)>> request_ GUARDED_BY(mutex_);
    os::memory::ConditionVariable requestDone_ GUARDED_BY(mutex_);
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_DEBUGGABLE_THREAD_H
