/**
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_TOOLING_INSPECTOR_INSPECTOR_H
#define PANDA_TOOLING_INSPECTOR_INSPECTOR_H

#include <atomic>
#include <cstddef>
#include <optional>
#include <set>
#include <string_view>
#include <thread>
#include <vector>

#include "include/tooling/debug_interface.h"
#include "include/tooling/pt_thread.h"

#include "debugger/breakpoint_storage.h"
#include "debugger/debug_info_cache.h"
#include "debugger/debuggable_thread.h"
#include "debugger/object_repository.h"

#include "common.h"
#include "inspector_server.h"
#include "runtime/tooling/tools.h"
#include "libarkbase/os/mutex.h"
#include "types/evaluation_result.h"
#include "types/numeric_id.h"
#include "types/pause_on_exceptions_state.h"
#include "types/profile_result.h"
#include "types/property_descriptor.h"
#include "types/remote_object.h"

namespace ark::tooling {
class DebugInterface;

namespace inspector {
// NOLINTNEXTLINE(fuchsia-virtual-inheritance)
class Server;

class Inspector final : public PtHooks {
public:
    Inspector(Server &server, DebugInterface &debugger, bool breakOnStart);
    ~Inspector() override;

    NO_COPY_SEMANTIC(Inspector);
    NO_MOVE_SEMANTIC(Inspector);

    void ConsoleCall(PtThread thread, ConsoleCallType type, uint64_t timestamp,
                     const PandaVector<TypedValue> &arguments) override;
    // CC-OFFNXT(G.FUN.01-CPP) Decreasing the number of arguments will decrease the clarity of the code.
    void Exception(PtThread thread, Method *method, const PtLocation &location, ObjectHeader *exception,
                   Method *catchMethod, const PtLocation &catchLocation) override;
    void FramePop(PtThread thread, Method *method, bool wasPoppedByException) override;
    void MethodEntry(PtThread thread, Method *method) override;
    void LoadModule(std::string_view fileName) override;
    void SingleStep(PtThread thread, Method *method, const PtLocation &location) override;
    void ThreadStart(PtThread thread) override;
    void ThreadEnd(PtThread thread) override;
    void VmDeath() override;

    void Run(const std::string& msg);
    void Stop();
    void WaitForDebugger();

private:
    void RuntimeEnable(PtThread thread);

    void RunIfWaitingForDebugger(PtThread thread);

    void Pause(PtThread thread);
    void Continue(PtThread thread);
    void Disable(PtThread thread);
    void ClientDisconnect(PtThread thread);
    void SetAsyncCallStackDepth(PtThread thread);
    void SetBlackboxPatterns(PtThread thread);
    void SmartStepInto(PtThread thread);
    void DropFrame(PtThread thread);
    void SetNativeRange(PtThread thread);
    void ReplyNativeCalling(PtThread thread);
    void SetBreakpointsActive(PtThread thread, bool active);
    void SetSkipAllPauses(PtThread thread, bool skip);
    void SetMixedDebugEnabled(PtThread thread, bool mixedDebugEnabled);
    std::set<int32_t> GetPossibleBreakpoints(std::string_view sourceFile, int32_t startLine, int32_t endLine,
                                            bool restrictToFunction);
    std::optional<BreakpointId> SetBreakpoint(PtThread thread, SourceFileFilter &&sourceFilesFilter, int32_t lineNumber,
                                              std::set<std::string_view> &sourceFiles, const std::string *condition);
    void RemoveBreakpoint(PtThread thread, BreakpointId id);
    void RemoveBreakpointsByUrl(PtThread thread, const char* url, const SourceFileFilter &sourceFilesFilter);

    void SetPauseOnExceptions(PtThread thread, PauseOnExceptionsState state);

    void StepInto(PtThread thread);
    void StepOver(PtThread thread);
    void StepOut(PtThread thread);
    void ContinueToLocation(PtThread thread, std::string_view sourceFile, int32_t lineNumber);

    void RestartFrame(PtThread thread, FrameId frameId);

    std::vector<PropertyDescriptor> GetProperties(PtThread thread, RemoteObjectId objectId, bool generatePreview);
    std::string GetSourceCode(std::string_view sourceFile);

    void DebuggableThreadPostSuspend(PtThread thread, ObjectRepository &objectRepository,
                                     const std::vector<BreakpointId> &hitBreakpoints, ObjectHeader *exception,
                                     PauseReason pauseReason);

    void NotifyExecutionEnded();

    Expected<EvaluationResult, std::string> Evaluate(PtThread thread, const std::string &bytecodeBase64,
                                                     size_t frameNumber);
    void ProfilerSetSamplingInterval(int32_t interval);
    Expected<bool, std::string> ProfilerStart();
    Expected<Profile, std::string> ProfilerStop();

    ALWAYS_INLINE bool CheckVmDead() REQUIRES_SHARED(vmDeathLock_)
    {
        if (UNLIKELY(isVmDead_)) {
            LOG(WARNING, DEBUGGER) << "Killing inspector server after VM death";
            inspectorServer_.Kill();
            return true;
        }
        return false;
    }

    /// @brief Get verbose information about the raised exception.
    std::optional<ExceptionDetails> CreateExceptionDetails(PtThread thread, RemoteObject &&exception);

    size_t GetNewExceptionId();

    DebuggableThread *GetDebuggableThread(PtThread thread);

    void RegisterMethodHandlers();

    void ResolveBreakpoints(const panda_file::File &file, const panda_file::DebugInfoExtractor *debugInfoCache);
    void CollectModules();
    void DebuggerEnable();
    void SourceNameInsert(const panda_file::DebugInfoExtractor *extractor);
    void PauseOtherThreads(PtThread thread);

private:
    bool breakOnStart_;

    os::memory::RWLock debuggerEventsLock_;
    bool connecting_ {false};  // Should be accessed only from the server thread

    InspectorServer inspectorServer_;  // NOLINT(misc-non-private-member-variables-in-classes)
    DebugInterface &debugger_;
    DebugInfoCache debugInfoCache_;
    std::map<PtThread, DebuggableThread> threads_;

    std::atomic_size_t currentExceptionId_ {0};

    os::memory::RWLock vmDeathLock_;
    bool isVmDead_ GUARDED_BY(vmDeathLock_) {false};

    BreakpointStorage breakpointStorage_;

    std::thread serverThread_;
    uint32_t samplingInterval_ {0};
    std::shared_ptr<sampler::SamplesRecord> profileInfoBuffer_ = nullptr;
    bool cpuProfilerStarted_ = false;
    os::memory::Mutex waitDebuggerMutex_;
    os::memory::ConditionVariable waitDebuggerCond_ GUARDED_BY(waitDebuggerMutex_);
};
}  // namespace inspector
}  // namespace ark::tooling

#endif  // PANDA_TOOLING_INSPECTOR_INSPECTOR_H
