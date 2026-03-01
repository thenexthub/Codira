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

#include "inspector.h"

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "debugger/breakpoint.h"
#include "libarkbase/macros.h"
#include "libarkbase/os/mutex.h"
#include "include/runtime.h"
#include "libarkbase/utils/logger.h"

#include "error.h"
#include "evaluation/base64.h"
#include "tooling/sampler/sampling_profiler.h"
#include "types/remote_object.h"
#include "types/scope.h"

using namespace std::placeholders;  // NOLINT(google-build-using-namespace)

namespace ark::tooling::inspector {
static void LogDebuggerNotPaused(std::string_view methodName)
{
    LOG(WARNING, DEBUGGER) << "Inspector method '" << methodName << "' must be called on pause";
}

Inspector::Inspector(Server &server, DebugInterface &debugger, bool breakOnStart)
    : breakOnStart_(breakOnStart), inspectorServer_(server), debugger_(debugger)
{
    if (!HandleError(debugger_.RegisterHooks(this))) {
        return;
    }

    // acquire lock to later release it either in `OnOpen` or `OnFail` callbacks
    inspectorServer_.OnValidate([this]() NO_THREAD_SAFETY_ANALYSIS {
        ASSERT(!connecting_);  // NOLINT(bugprone-lambda-function-name)
        debuggerEventsLock_.WriteLock();
        connecting_ = true;
    });
    inspectorServer_.OnOpen([this]() NO_THREAD_SAFETY_ANALYSIS {
        ASSERT(connecting_);  // NOLINT(bugprone-lambda-function-name)
        connecting_ = false;
        debuggerEventsLock_.Unlock();
    });
    inspectorServer_.OnFail([this]() NO_THREAD_SAFETY_ANALYSIS {
        if (connecting_) {
            connecting_ = false;
            debuggerEventsLock_.Unlock();
        }
    });

    RegisterMethodHandlers();
}

Inspector::~Inspector()
{
    // Current implementation destroys `Inspector` after server connection is closed,
    // hence no need to notify client
    inspectorServer_.Kill();
    HandleError(debugger_.UnregisterHooks());
}

void Inspector::CollectModules()
{
    os::memory::ReadLockHolder lock(debuggerEventsLock_);
    Runtime::GetCurrent()->GetClassLinker()->EnumeratePandaFiles([this](auto &file) {
        debugInfoCache_.AddPandaFile(file);
        // Do not call server, cause no connection at this stage
        return true;
    });
}

void Inspector::Run(const std::string& msg)
{
    inspectorServer_.Run(msg);
}

void Inspector::Stop()
{
    serverThread_.join();
}

void Inspector::ConsoleCall(PtThread thread, ConsoleCallType type, uint64_t timestamp,
                            const PandaVector<TypedValue> &arguments)
{
    os::memory::ReadLockHolder lock(debuggerEventsLock_);

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        inspectorServer_.CallRuntimeConsoleApiCalled(
            thread,
            type,
            timestamp,
            debuggableThread->OnConsoleCall(arguments)
        );
    }
}

// CC-OFFNXT(G.FUN.01-CPP) Decreasing the number of arguments will decrease the clarity of the code.
void Inspector::Exception(PtThread thread, Method * /* method */, const PtLocation & /* location */,
                          ObjectHeader * /* exception */, Method * /* catch_method */, const PtLocation &catchLocation)
{
    os::memory::ReadLockHolder lock(debuggerEventsLock_);

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        debuggableThread->OnException(catchLocation.GetBytecodeOffset() == panda_file::INVALID_OFFSET);
    }
}

void Inspector::FramePop(PtThread thread, Method * /* method */, bool /* was_popped_by_exception */)
{
    os::memory::ReadLockHolder lock(debuggerEventsLock_);

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        debuggableThread->OnFramePop();
    }
}

void Inspector::MethodEntry(PtThread thread, Method * /* method */)
{
    os::memory::ReadLockHolder lock(debuggerEventsLock_);

    auto *debuggableThread = GetDebuggableThread(thread);
    auto stack = StackWalker::Create(thread.GetManagedThread());
    if (stack.IsCFrame()) {
        return;
    }
    if (debuggableThread == nullptr) {
        return;
    }
    if (debuggableThread != nullptr) {
        if (debuggableThread->OnMethodEntry()) {
            HandleError(debugger_.NotifyFramePop(thread, 0));
        }
    }
}

void Inspector::SourceNameInsert(const panda_file::DebugInfoExtractor *extractor)
{
    const auto &methodList = extractor->GetMethodIdList();
    std::unordered_set<std::string> sourceNames;
    for (const auto &method : methodList) {
        sourceNames.insert(extractor->GetSourceFile(method));
    }
    for (const auto &sourceName : sourceNames) {
        // Get src file name
        auto [scriptId, isNew] = inspectorServer_.GetSourceManager().GetScriptId(sourceName);
        inspectorServer_.CallDebuggerScriptParsed(scriptId, sourceName);
    }
}

void Inspector::LoadModule(std::string_view fileName)
{
    os::memory::ReadLockHolder lock(debuggerEventsLock_);

    Runtime::GetCurrent()->GetClassLinker()->EnumeratePandaFiles(
        [this, fileName](auto &file) {
            if (file.GetFilename() == fileName) {
                debugInfoCache_.AddPandaFile(file, true);
            }

            return true;
        },
        !fileName.empty());
}

void Inspector::ResolveBreakpoints(const panda_file::File &file, const panda_file::DebugInfoExtractor *debugInfo)
{
    breakpointStorage_.ResolveBreakpoints(file, debugInfo);
}

void Inspector::SingleStep(PtThread thread, Method *method, const PtLocation &location)
{
    os::memory::ReadLockHolder lock(debuggerEventsLock_);

    auto sourceFile = debugInfoCache_.GetUserSourceFile(method);
    // NOTE(fangting, #IC98Z2): etsstdlib.ets should not call loadModule in pytest.
    if ((sourceFile == nullptr) || (strcmp(sourceFile, "etsstdlib.ets") == 0)) {
        return;
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        debuggableThread->OnSingleStep(location, sourceFile);
    }
}

void Inspector::ThreadStart(PtThread thread)
{
    os::memory::WriteLockHolder lock(debuggerEventsLock_);

    if (thread != PtThread::NONE) {
        inspectorServer_.CallTargetAttachedToTarget(thread); // IDE not adjusted
    }

    // NOLINTBEGIN(modernize-avoid-bind)
    auto callbacks = DebuggableThread::SuspensionCallbacks {
        [](auto &, auto &, auto) {},
        std::bind(&Inspector::DebuggableThreadPostSuspend, this, thread, _1, _2, _3, _4),
        [this]() NO_THREAD_SAFETY_ANALYSIS { debuggerEventsLock_.Unlock(); },
        [this]() NO_THREAD_SAFETY_ANALYSIS { debuggerEventsLock_.ReadLock(); },
        []() {},
        [this, thread]() { inspectorServer_.CallDebuggerResumed(thread); }};
    // NOLINTEND(modernize-avoid-bind)
    auto [it, inserted] = threads_.emplace(
        std::piecewise_construct, std::forward_as_tuple(thread),
        std::forward_as_tuple(thread.GetManagedThread(), &debugger_, std::move(callbacks), breakpointStorage_));
    (void)inserted;
    ASSERT(inserted);

    if (breakOnStart_) {
        it->second.BreakOnStart();
    }
}

void Inspector::ThreadEnd(PtThread thread)
{
    os::memory::WriteLockHolder lock(debuggerEventsLock_);

    if (thread != PtThread::NONE) {
        inspectorServer_.CallTargetDetachedFromTarget(thread);
    }

    [[maybe_unused]] auto erased = threads_.erase(thread);
    ASSERT(erased == 1);
}

void Inspector::PauseOtherThreads(PtThread thread)
{
    auto threadId = thread.GetId();
    for (auto &[threadTmp, dbgThread] : threads_) {
        if (threadTmp.GetId() == threadId) {
            continue;
        }
        dbgThread.Pause();
    }
}

void Inspector::VmDeath()
{
    os::memory::WriteLockHolder lock(vmDeathLock_);

    ASSERT(!isVmDead_);
    isVmDead_ = true;

    NotifyExecutionEnded();
}

void Inspector::RuntimeEnable(PtThread thread)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    inspectorServer_.CallRuntimeExecutionContextCreated(thread);
}

void Inspector::RunIfWaitingForDebugger(PtThread thread)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        debuggableThread->Touch();
    }

    os::memory::LockHolder<os::memory::Mutex> lockHolder(waitDebuggerMutex_);
    waitDebuggerCond_.Signal();
}

//For Hybrid it was not used, instead it use 1.0 waitForDebugger
void Inspector::WaitForDebugger()
{
    os::memory::LockHolder<os::memory::Mutex> lock(waitDebuggerMutex_);
    waitDebuggerCond_.Wait(&waitDebuggerMutex_);
}

void Inspector::Pause(PtThread thread)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    (void)thread;

    for (auto &[_, dbgThread] : threads_) {
        dbgThread.Pause();
    }
}

void Inspector::Continue(PtThread thread)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if ((debuggableThread != nullptr) && debuggableThread->IsPausedByBreakOnStart()) {
        for (auto &[ptThread, dbgThread] : threads_) {
            if (&dbgThread != debuggableThread && dbgThread.IsPausedByBreakPoint()) {
                LOG(INFO, DEBUGGER) << "other debuggableThread is paused by breakpoint";
                return;
            }
        }
        debuggableThread->Continue();
        return;
    }

    for (auto &[_, dbgThread] : threads_) {
        dbgThread.Continue();
    }
}

void Inspector::SetBreakpointsActive([[maybe_unused]] PtThread thread, bool active)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    breakpointStorage_.SetBreakpointsActive(active);
}

void Inspector::SetSkipAllPauses(PtThread thread, bool skip)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        debuggableThread->SetSkipAllPauses(skip);
    }
}

void Inspector::SetMixedDebugEnabled(PtThread thread, bool mixedDebugEnabled)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        debuggableThread->SetMixedDebugEnabled(mixedDebugEnabled);
    }
}

std::set<int32_t> Inspector::GetPossibleBreakpoints(std::string_view sourceFile, int32_t startLine,
                                                    int32_t endLine, bool restrictToFunction)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return {};
    }

    return debugInfoCache_.GetValidLineNumbers(sourceFile, startLine, endLine, restrictToFunction);
}

std::optional<BreakpointId> Inspector::SetBreakpoint([[maybe_unused]] PtThread thread,
                                                     SourceFileFilter &&sourceFilesFilter, int32_t lineNumber,
                                                     std::set<std::string_view> &sourceFiles,
                                                     const std::string *condition)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return {};
    }

    std::string optBytecode;
    if (condition != nullptr) {
        if (condition->empty()) {
            // Some debugger clients send empty condition by default
            condition = nullptr;
        } else {
            Base64Decoder::Decode(*condition, optBytecode);
            condition = &optBytecode;
        }
    }

    return breakpointStorage_.SetBreakpoint(std::move(sourceFilesFilter), lineNumber, sourceFiles, condition,
                                            debugInfoCache_);
}

void Inspector::RemoveBreakpoint([[maybe_unused]] PtThread thread, BreakpointId id)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    breakpointStorage_.RemoveBreakpoint(id);
}

void Inspector::RemoveBreakpointsByUrl(PtThread thread, const char* url, const SourceFileFilter &sourceFilesFilter)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    if (url == nullptr || strlen(url) == 0) {
        return;
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread == nullptr) {
        return;
    }
    auto pandaFilesPaths =  debugInfoCache_.GetPandaFiles(sourceFilesFilter);

    breakpointStorage_.RemoveBreakpoints([this, url, pfs = std::as_const(pandaFilesPaths)](const auto &loc) {
        for (const auto &pf : pfs) {
            const char* sourceFile = debugInfoCache_.GetDebugInfo(pf)->GetSourceFile(loc.GetMethodId());
            return sourceFile && std::strcmp(sourceFile, url) == 0;
        }
        return false;
    });
}

void Inspector::SetPauseOnExceptions(PtThread thread, PauseOnExceptionsState state)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        debuggableThread->SetPauseOnExceptions(state);
    }
}

void Inspector::StepInto(PtThread thread)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        if (UNLIKELY(!debuggableThread->IsPaused())) {
            LogDebuggerNotPaused("stepInto");
            return;
        }

        auto frame = debugger_.GetCurrentFrame(thread);
        if (!frame) {
            HandleError(frame.Error());
            return;
        }

        debuggableThread->StepInto(debugInfoCache_.GetCurrentLineLocations(*frame.Value()));
    }
}

void Inspector::StepOver(PtThread thread)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        if (UNLIKELY(!debuggableThread->IsPaused())) {
            LogDebuggerNotPaused("stepOver");
            return;
        }

        auto frame = debugger_.GetCurrentFrame(thread);
        if (!frame) {
            HandleError(frame.Error());
            return;
        }

        debuggableThread->StepOver(debugInfoCache_.GetCurrentLineLocations(*frame.Value()));
    }
}

void Inspector::StepOut(PtThread thread)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        if (UNLIKELY(!debuggableThread->IsPaused())) {
            LogDebuggerNotPaused("stepOut");
            return;
        }

        HandleError(debugger_.NotifyFramePop(thread, 0));
        debuggableThread->StepOut();
    }
}

void Inspector::ContinueToLocation(PtThread thread, std::string_view sourceFile, int32_t lineNumber)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        if (UNLIKELY(!debuggableThread->IsPaused())) {
            LogDebuggerNotPaused("continueToLocation");
            return;
        }

        debuggableThread->ContinueTo(debugInfoCache_.GetContinueToLocations(sourceFile, lineNumber));
    }
}

void Inspector::RestartFrame(PtThread thread, FrameId frameId)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        if (UNLIKELY(!debuggableThread->IsPaused())) {
            LogDebuggerNotPaused("restartFrame");
            return;
        }

        if (auto error = debugger_.RestartFrame(thread, frameId)) {
            HandleError(*error);
            return;
        }

        debuggableThread->StepInto({});
    }
}

std::vector<PropertyDescriptor> Inspector::GetProperties(PtThread thread, RemoteObjectId objectId, bool generatePreview)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return {};
    }

    std::optional<std::vector<PropertyDescriptor>> properties;

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread != nullptr) {
        debuggableThread->RequestToObjectRepository([objectId, generatePreview, &properties](auto &objectRepository) {
            properties = objectRepository.GetProperties(objectId, generatePreview);
        });
    }

    if (!properties) {
        LOG(INFO, DEBUGGER) << "Failed to resolve object id: " << objectId;
        return {};
    }

    return *properties;
}

std::string Inspector::GetSourceCode(std::string_view sourceFile)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return {};
    }

    return debugInfoCache_.GetSourceCode(sourceFile);
}

void Inspector::DebuggableThreadPostSuspend(PtThread thread, ObjectRepository &objectRepository,
                                            const std::vector<BreakpointId> &hitBreakpoints, ObjectHeader *exception,
                                            PauseReason pauseReason)
{
    auto exceptionRemoteObject = exception != nullptr ? objectRepository.CreateObject(TypedValue::Reference(exception))
                                                      : std::optional<RemoteObject>();

    inspectorServer_.CallDebuggerPaused(
        thread, hitBreakpoints, exceptionRemoteObject, pauseReason, [this, thread, &objectRepository](auto &handler) {
            FrameId frameId = 0;
            HandleError(debugger_.EnumerateFrames(thread, [this, &objectRepository, &handler,
                                                           &frameId](const PtFrame &frame) {
                std::string_view sourceFile;
                std::string_view methodName;
                int32_t lineNumber;
                debugInfoCache_.GetSourceLocation(frame, sourceFile, methodName, lineNumber);
                if (sourceFile.empty()) {
                    return false;
                }

                std::optional<RemoteObject> objThis;
                auto frameObject = objectRepository.CreateFrameObject(frame, debugInfoCache_.GetLocals(frame), objThis);
                auto scopeChain = std::vector {Scope(Scope::Type::LOCAL, std::move(frameObject)),
                                               Scope(Scope::Type::GLOBAL, objectRepository.CreateGlobalObject())};

                handler(frameId++, methodName, sourceFile, lineNumber, scopeChain, objThis);

                return true;
            }));
        });

    if (!hitBreakpoints.empty()) {
        PauseOtherThreads(thread);
    }
}

void Inspector::NotifyExecutionEnded()
{
    inspectorServer_.CallRuntimeExecutionContextsCleared();
}

Expected<EvaluationResult, std::string> Inspector::Evaluate(PtThread thread, const std::string &bytecodeBase64,
                                                            size_t frameNumber)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return Unexpected(std::string("Fatal, VM is dead"));
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread == nullptr) {
        return Unexpected(std::string("No thread found"));
    }

    if (UNLIKELY(!debuggableThread->IsPaused())) {
        LogDebuggerNotPaused("evaluate");
        return Unexpected(std::string("Expression evaluation can be done only on pause"));
    }

    std::string bytecode;
    Base64Decoder::Decode(bytecodeBase64, bytecode);
    auto optResult = debuggableThread->EvaluateExpression(frameNumber, bytecode);
    if (!optResult) {
        return Unexpected(std::move(optResult.Error()));
    }
    auto optExceptionDetails = (optResult->second) ? CreateExceptionDetails(thread, std::move(*optResult->second))
                                                   : std::optional<ExceptionDetails>();
    return EvaluationResult(std::move(optResult->first), std::move(optExceptionDetails));
}

std::optional<ExceptionDetails> Inspector::CreateExceptionDetails(PtThread thread, RemoteObject &&exception)
{
    auto frame = debugger_.GetCurrentFrame(thread);
    if (!frame) {
        HandleError(frame.Error());
        return {};
    }

    std::string_view sourceFile;
    std::string_view methodName;
    int32_t lineNumber;
    debugInfoCache_.GetSourceLocation(*frame.Value(), sourceFile, methodName, lineNumber);

    ExceptionDetails exceptionDetails(GetNewExceptionId(), "", lineNumber, 0);
    return exceptionDetails.SetUrl(sourceFile).SetExceptionObject(std::move(exception));
}

size_t Inspector::GetNewExceptionId()
{
    // Atomic with relaxed order reason: data race on concurrent exceptions happening in conditional breakpoints.
    return currentExceptionId_.fetch_add(1, std::memory_order_relaxed);
}

DebuggableThread *Inspector::GetDebuggableThread(PtThread thread)
{
    auto it = threads_.find(thread);
    return it != threads_.end() ? &it->second : nullptr;
}

void Inspector::Disable(PtThread thread)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return;
    }

    auto *debuggableThread = GetDebuggableThread(thread);
    if (debuggableThread == nullptr) {
        return;
    }
    debuggableThread->Reset();
    debuggableThread->Continue();
}

void Inspector::ClientDisconnect(PtThread thread)
{
    (void)thread;
}

void Inspector::SetAsyncCallStackDepth(PtThread thread)
{
    (void)thread;
}

void Inspector::SetBlackboxPatterns(PtThread thread)
{
    (void)thread;
}

void Inspector::SmartStepInto(PtThread thread)
{
    (void)thread;
}

void Inspector::DropFrame(PtThread thread)
{
    (void)thread;
}

void Inspector::SetNativeRange(PtThread thread)
{
    (void)thread;
}

void Inspector::ReplyNativeCalling(PtThread thread)
{
    Continue(thread);
}

void Inspector::ProfilerSetSamplingInterval(int32_t interval)
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead() || interval < 0)) {
        return;
    }
    samplingInterval_ = static_cast<uint32_t>(interval);
}

Expected<bool, std::string> Inspector::ProfilerStart()
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return Unexpected(std::string("Fatal, VM is dead"));
    }
    if (cpuProfilerStarted_) {
        return Unexpected(std::string("Fatal, profiling operation is already running."));
    }
    cpuProfilerStarted_ = true;
    profileInfoBuffer_ = std::make_shared<sampler::SamplesRecord>();
    profileInfoBuffer_->SetThreadStartTime(sampler::Sampler::GetMicrosecondsTimeStamp());
    Runtime::GetCurrent()->GetTools().StartSamplingProfiler(
        std::make_unique<sampler::InspectorStreamWriter>(profileInfoBuffer_), samplingInterval_);
    return true;
}

Expected<Profile, std::string> Inspector::ProfilerStop()
{
    os::memory::ReadLockHolder lock(vmDeathLock_);
    if (UNLIKELY(CheckVmDead())) {
        return Unexpected(std::string("Fatal, VM is dead"));
    }

    if (!cpuProfilerStarted_) {
        return Unexpected(std::string("Fatal, profiler inactive"));
    }

    Runtime::GetCurrent()->GetTools().StopSamplingProfiler();
    auto profileInfoPtr = profileInfoBuffer_->GetAllThreadsProfileInfos();
    if (!profileInfoPtr) {
        return Unexpected(std::string("Fatal, profiler info is empty"));
    }
    profileInfoBuffer_.reset();
    cpuProfilerStarted_ = false;
    return Profile(std::move(profileInfoPtr));
}

void Inspector::DebuggerEnable()
{
    os::memory::WriteLockHolder lock(debuggerEventsLock_);
    for (auto &[_, dbgThread] : threads_) {
        (void)_;
        dbgThread.Reset();
    }
    breakpointStorage_.Reset();
}

void Inspector::RegisterMethodHandlers()
{
    // NOLINTBEGIN(modernize-avoid-bind)
    inspectorServer_.OnCallDebuggerContinueToLocation(std::bind(&Inspector::ContinueToLocation, this, _1, _2, _3));
    inspectorServer_.OnCallDebuggerEnable(std::bind(&Inspector::DebuggerEnable, this));
    inspectorServer_.OnCallDebuggerGetPossibleBreakpoints(
        std::bind(&Inspector::GetPossibleBreakpoints, this, _1, _2, _3, _4));
    inspectorServer_.OnCallDebuggerGetScriptSource(std::bind(&Inspector::GetSourceCode, this, _1));
    inspectorServer_.OnCallDebuggerPause(std::bind(&Inspector::Pause, this, _1));
    inspectorServer_.OnCallDebuggerRemoveBreakpoint(std::bind(&Inspector::RemoveBreakpoint, this, _1, _2));
    inspectorServer_.OnCallDebuggerRemoveBreakpointsByUrl(
        std::bind(&Inspector::RemoveBreakpointsByUrl, this, _1, _2, _3));
    inspectorServer_.OnCallDebuggerRestartFrame(std::bind(&Inspector::RestartFrame, this, _1, _2));
    inspectorServer_.OnCallDebuggerResume(std::bind(&Inspector::Continue, this, _1));
    inspectorServer_.OnCallDebuggerSetAsyncCallStackDepth(std::bind(&Inspector::SetAsyncCallStackDepth, this, _1));
    inspectorServer_.OnCallDebuggerSetBlackboxPatterns(std::bind(&Inspector::SetBlackboxPatterns, this, _1));
    inspectorServer_.OnCallDebuggerSmartStepInto(std::bind(&Inspector::SmartStepInto, this, _1));
    inspectorServer_.OnCallDebuggerSetBreakpoint(std::bind(&Inspector::SetBreakpoint, this, _1, _2, _3, _4, _5));
    inspectorServer_.OnCallDebuggerSetBreakpointByUrl(std::bind(&Inspector::SetBreakpoint, this, _1, _2, _3, _4, _5));
    inspectorServer_.OnCallDebuggerGetPossibleAndSetBreakpointByUrl(
        std::bind(&Inspector::SetBreakpoint, this, _1, _2, _3, _4, _5));
    inspectorServer_.OnCallDebuggerSetBreakpointsActive(std::bind(&Inspector::SetBreakpointsActive, this, _1, _2));
    inspectorServer_.OnCallDebuggerSetSkipAllPauses(std::bind(&Inspector::SetSkipAllPauses, this, _1, _2));
    inspectorServer_.OnCallDebuggerSetPauseOnExceptions(std::bind(&Inspector::SetPauseOnExceptions, this, _1, _2));
    inspectorServer_.OnCallDebuggerStepInto(std::bind(&Inspector::StepInto, this, _1));
    inspectorServer_.OnCallDebuggerStepOut(std::bind(&Inspector::StepOut, this, _1));
    inspectorServer_.OnCallDebuggerStepOver(std::bind(&Inspector::StepOver, this, _1));
    inspectorServer_.OnCallDebuggerEvaluateOnCallFrame(std::bind(&Inspector::Evaluate, this, _1, _2, _3));
    inspectorServer_.OnCallDebuggerDisable(std::bind(&Inspector::Disable, this, _1));
    inspectorServer_.OnCallDebuggerClientDisconnect(std::bind(&Inspector::ClientDisconnect, this, _1));
    inspectorServer_.OnCallDebuggerDropFrame(std::bind(&Inspector::DropFrame, this, _1));
    inspectorServer_.OnCallDebuggerSetNativeRange(std::bind(&Inspector::SetNativeRange, this, _1));
    inspectorServer_.OnCallDebuggerReplyNativeCalling(std::bind(&Inspector::ReplyNativeCalling, this, _1));
    inspectorServer_.OnCallDebuggerCallFunctionOn(std::bind(&Inspector::Evaluate, this, _1, _2, _3));
    inspectorServer_.OnCallDebuggerSetMixedDebugEnabled(std::bind(&Inspector::SetMixedDebugEnabled, this, _1, _2));
    inspectorServer_.OnCallRuntimeEnable(std::bind(&Inspector::RuntimeEnable, this, _1));
    inspectorServer_.OnCallRuntimeGetProperties(std::bind(&Inspector::GetProperties, this, _1, _2, _3));
    inspectorServer_.OnCallRuntimeRunIfWaitingForDebugger(std::bind(&Inspector::RunIfWaitingForDebugger, this, _1));
    inspectorServer_.OnCallRuntimeEvaluate(std::bind(&Inspector::Evaluate, this, _1, _2, 0));
    inspectorServer_.OnCallProfilerEnable();
    inspectorServer_.OnCallProfilerDisable();
    inspectorServer_.OnCallProfilerSetSamplingInterval(std::bind(&Inspector::ProfilerSetSamplingInterval, this, _1));
    inspectorServer_.OnCallProfilerStart(std::bind(&Inspector::ProfilerStart, this));
    inspectorServer_.OnCallProfilerStop(std::bind(&Inspector::ProfilerStop, this));
    // NOLINTEND(modernize-avoid-bind)
}
}  // namespace ark::tooling::inspector
