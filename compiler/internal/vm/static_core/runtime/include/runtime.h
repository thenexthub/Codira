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

#ifndef PANDA_RUNTIME_RUNTIME_H_
#define PANDA_RUNTIME_RUNTIME_H_

#include <atomic>
#include <csignal>
#include <memory>
#include <string>
#include <vector>

#include "libarkbase/mem/arena_allocator.h"
#include "libarkbase/os/mutex.h"
#include "libarkbase/taskmanager/task_scheduler.h"
#include "libarkbase/utils/expected.h"
#include "libarkbase/utils/dfx.h"
#include "../libarkfile/file_items.h"
#include "../libarkfile/literal_data_accessor.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime_options.h"
#include "runtime/include/gc_task.h"
#ifndef PANDA_TARGET_WINDOWS
#include "runtime/signal_handler.h"
#endif
#include "runtime/mem/allocator_adapter.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/gc_trigger.h"
#include "runtime/mem/memory_manager.h"
#include "runtime/monitor_pool.h"
#include "runtime/string_table.h"
#include "runtime/thread_manager.h"
#include "verification/public.h"
#include "libarkbase/os/native_stack.h"
#include "libarkbase/os/library_loader.h"
#include "runtime/include/loadable_agent.h"
#include "runtime/tooling/tools.h"

namespace ark {

class DProfiler;
class CompilerInterface;
class ClassHierarchyAnalysis;
class RuntimeController;
class PandaVM;
class RuntimeNotificationManager;
class Trace;

namespace tooling {
class DebugInterface;
class MemoryAllocationDumper;
class CoverageListener;
}  // namespace tooling

using UnwindStackFn = os::native_stack::FuncUnwindstack;

class PANDA_PUBLIC_API Runtime {
public:
    using ExitHook = void (*)(int32_t status);
    using AbortHook = void (*)();

    enum class Error {
        PANDA_FILE_LOAD_ERROR,
        INVALID_ENTRY_POINT,
        CLASS_NOT_FOUND,
        CLASS_NOT_INITIALIZED,
        METHOD_NOT_FOUND,
        CLASS_LINKER_EXTENSION_NOT_FOUND
    };

    class DebugSession final {
    public:
        explicit DebugSession(Runtime &runtime);
        ~DebugSession();

        tooling::DebugInterface &GetDebugger();

    private:
        Runtime &runtime_;
        bool isJitEnabled_;
        os::memory::LockHolder<os::memory::Mutex> lock_;
        PandaUniquePtr<tooling::DebugInterface> debugger_;

        NO_COPY_SEMANTIC(DebugSession);
        NO_MOVE_SEMANTIC(DebugSession);
    };

    using DebugSessionHandle = std::shared_ptr<DebugSession>;

    LanguageContext GetLanguageContext(const std::string &runtimeType);
    LanguageContext GetLanguageContext(const Method &method);
    LanguageContext GetLanguageContext(const Class &cls);
    LanguageContext GetLanguageContext(const BaseClass &cls);
    LanguageContext GetLanguageContext(panda_file::ClassDataAccessor *cda);
    LanguageContext GetLanguageContext(panda_file::SourceLang lang);

    static bool CreateInstance(const RuntimeOptions &options, mem::InternalAllocatorPtr internalAllocator);

    static bool Create(const RuntimeOptions &options);

    // Deprecated. Only for capability with js_runtime.
    static bool Create(const RuntimeOptions &options, const std::vector<LanguageContextBase *> &ctxs);

    static bool DestroyUnderLockHolder();

    static bool Destroy();

    static Runtime *GetCurrent();

    static bool GetCoverageEnable(RuntimeOptions &options);
    static std::string GetCodeCoverageOutput();
    static void InitCoverageOptions(RuntimeOptions &options);
    static void StartCoverageListener();
    static void StopCoverageListener();

    template <typename Handler>
    static auto GetCurrentSync(Handler &&handler)
    {
        os::memory::LockHolder lock(mutex_);
        return handler(*GetCurrent());
    }

    static os::memory::Mutex *GetRuntimeLock()
    {
        return &mutex_;
    }

    ClassLinker *GetClassLinker() const
    {
        return classLinker_;
    }

    RuntimeNotificationManager *GetNotificationManager() const
    {
        return notificationManager_;
    }

    static const RuntimeOptions &GetOptions()
    {
        return options_;
    }

    void SetZygoteNoThreadSection(bool val)
    {
        zygoteNoThreads_ = val;
    }

    static bool IsTaskManagerUsed()
    {
        return isTaskManagerUsed_;
    }

    static void SetTaskManagerUsed(bool value)
    {
        isTaskManagerUsed_ = value;
    }

    coretypes::String *ResolveString(PandaVM *vm, const Method &caller, panda_file::File::EntityId id);

    coretypes::String *ResolveStringFromCompiledCode(PandaVM *vm, const Method &caller, panda_file::File::EntityId id);

    coretypes::String *ResolveString(PandaVM *vm, const panda_file::File &pf, panda_file::File::EntityId id,
                                     const LanguageContext &ctx);

    coretypes::String *ResolveString(PandaVM *vm, const uint8_t *mutf8, uint32_t length, const LanguageContext &ctx);

    Class *GetClassRootForLiteralTag(const ClassLinkerExtension &ext, panda_file::LiteralTag tag) const;

    static bool GetLiteralTagAndValue(const panda_file::File &pf, uint32_t id, panda_file::LiteralTag *tag,
                                      panda_file::LiteralDataAccessor::LiteralValue *value);

    static void SetRuntimeOptions(RuntimeOptions &options);

    static void SetDebuggerOptions(RuntimeOptions &options);

    static void SetInterpreterTypeOptions(RuntimeOptions &options);

    static void SetUseLargerYoungSpaceOptions(RuntimeOptions &options);

    static void SetEnableClassLinkerTraceOptions(RuntimeOptions &options);

    uintptr_t GetPointerToConstArrayData(const panda_file::File &pf, uint32_t id) const;

    coretypes::Array *ResolveLiteralArray(PandaVM *vm, const Method &caller, uint32_t id);
    coretypes::Array *ResolveLiteralArray(PandaVM *vm, const panda_file::File &pf, uint32_t id,
                                          const LanguageContext &ctx);

    void PreZygoteFork();

    void PostZygoteFork();

    Expected<int, Error> ExecutePandaFile(std::string_view filename, std::string_view entryPoint,
                                          const std::vector<std::string> &args);

    int StartDProfiler(std::string_view appName);

    int StartMemAllocDumper(const PandaString &dumpFile);

    Expected<int, Error> Execute(std::string_view entryPoint, const std::vector<std::string> &args);

    int StartDProfiler(const PandaString &appName);

    bool IsDebugMode() const
    {
        return isDebugMode_;
    }

    void SetDebugMode(bool isDebugMode)
    {
        isDebugMode_ = isDebugMode;
    }

    bool IsDebuggerConnected() const
    {
        return isDebuggerConnected_;
    }

    void SetDebuggerConnected(bool dbgConnectedState)
    {
        isDebuggerConnected_ = dbgConnectedState;
    }

    bool IsProfileableFromShell() const
    {
        return isProfileableFromShell_;
    }

    void SetProfileableFromShell(bool profileableFromShell)
    {
        isProfileableFromShell_ = profileableFromShell;
    }

    PandaVector<PandaString> GetBootPandaFiles();

    PandaVector<PandaString> GetPandaFiles();

    // Returns true if profile saving is enabled.
    bool SaveProfileInfo() const;

    bool IncrementalSaveProfileInfo() const;

    bool TryCreateSaverTask();

    const std::string &GetProcessPackageName() const
    {
        return processPackageName_;
    }

    void SetProcessPackageName(const char *packageName)
    {
        if (packageName == nullptr) {
            processPackageName_.clear();
        } else {
            processPackageName_ = packageName;
        }
    }

    const std::string &GetProcessDataDirectory() const
    {
        return processDataDirectory_;
    }

    void SetProcessDataDirectory(const char *dataDir)
    {
        if (dataDir == nullptr) {
            processDataDirectory_.clear();
        } else {
            processDataDirectory_ = dataDir;
        }
    }

    std::string GetPandaPath()
    {
        return pandaPathString_;
    }

    static const std::string &GetRuntimeType()
    {
        return runtimeType_;
    }

    void UpdateProcessState(int state);

    bool IsZygote() const
    {
        return isZygote_;
    }

    bool IsInitialized() const
    {
        return isInitialized_;
    }

    // NOTE(00510180): lack NativeBridgeAction action
    void InitNonZygoteOrPostFork(bool isSystemServer, const char *isa, const std::function<void()> &initHook = {},
                                 bool profileSystemServer = false);

    static const char *GetVersion()
    {
        // NOTE(chenmudan): change to the correct version when we have one;
        return "1.0.0";
    }

    PandaString GetFingerprint()
    {
        return fingerPrint_;
    }

    [[noreturn]] static void Halt(int32_t status);

    void SetExitHook(ExitHook exitHook)
    {
        ASSERT(exit_ == nullptr);
        exit_ = exitHook;
    }

    void SetAbortHook(AbortHook abortHook)
    {
        ASSERT(abort_ == nullptr);
        abort_ = abortHook;
    }

    [[noreturn]] static void Abort(const char *message = nullptr);

    Expected<Method *, Error> ResolveEntryPoint(std::string_view entryPoint);

    void RegisterSensitiveThread() const;

    // Deprecated.
    // Get VM instance from the thread. In multi-vm runtime this method returns
    // the first VM. It is undeterminated which VM will be first.
    PandaVM *GetPandaVM() const
    {
        return pandaVm_;
    }

    ClassHierarchyAnalysis *GetCha() const
    {
        return cha_;
    }

    ark::verifier::Config const *GetVerificationConfig() const
    {
        return verifierConfig_;
    }

    ark::verifier::Config *GetVerificationConfig()
    {
        return verifierConfig_;
    }

    ark::verifier::Service *GetVerifierService()
    {
        return verifierService_;
    }

    bool IsDebuggerAttached()
    {
        return debugSession_.use_count() > 0;
    }

    void DumpForSigQuit(std::ostream &os);

    bool IsDumpNativeCrash()
    {
        return isDumpNativeCrash_;
    }

    bool IsChecksSuspend() const
    {
        return checksSuspend_;
    }

    bool IsChecksStack() const
    {
        return checksStack_;
    }

    bool IsChecksNullptr() const
    {
        return checksNullptr_;
    }

    bool IsStacktrace() const
    {
        return isStacktrace_;
    }

    bool IsJitEnabled() const
    {
        return isJitEnabled_;
    }

    bool IsProfilerEnabled() const
    {
        return isProfilerEnabled_ || isJitEnabled_;
    }

    bool IsProfileBranches() const
    {
        return isProfileBranches_;
    }

    void ForceEnableJit()
    {
        isJitEnabled_ = true;
    }

    void ForceDisableJit()
    {
        isJitEnabled_ = false;
    }

#ifndef PANDA_TARGET_WINDOWS
    SignalManager *GetSignalManager()
    {
        return signalManager_;
    }
#endif

    static mem::GCType GetGCType(const RuntimeOptions &options, panda_file::SourceLang lang);

    static void SetDaemonMemoryLeakThreshold(uint32_t daemonMemoryLeakThreshold);

    static void SetDaemonThreadsCount(uint32_t daemonThreadsCnt);

    DebugSessionHandle StartDebugSession();

    mem::InternalAllocatorPtr GetInternalAllocator() const
    {
        return internalAllocator_;
    }

    PandaString GetMemoryStatistics();
    PandaString GetFinalStatistics();

    Expected<LanguageContext, Error> ExtractLanguageContext(const panda_file::File *pf, std::string_view entryPoint);

    UnwindStackFn GetUnwindStackFn() const
    {
        return unwindStackFn_;
    }

    void SetUnwindStackFn(UnwindStackFn unwindStackFn)
    {
        unwindStackFn_ = unwindStackFn;
    }

    inline tooling::Tools &GetTools()
    {
        return tools_;
    }

    EntrypointsTable *GetEntrypointsTable()
    {
        return &entrypoints_;
    }

private:
    void NotifyAboutLoadedModules();

    std::optional<Error> CreateApplicationClassLinkerContext(std::string_view filename, std::string_view entryPoint);

    bool LoadVerificationConfig();

    bool CreatePandaVM(std::string_view runtimeType);

    bool InitializePandaVM();

    bool HandleAotOptions();

    void HandleJitOptions();

    bool CheckOptionsConsistency();

    void CheckOptionsFromOs() const;

    void SetPandaPath();

    void SetThreadClassPointers();

    bool Initialize();

    bool Shutdown();

    bool LoadBootPandaFiles(panda_file::File::OpenMode openMode);

    void CheckBootPandaFiles();

    bool IsEnableMemoryHooks() const;

    /**
     * @brief Unload debugger library and destroy debug session.
     * As side effect, `Debugger` instance will be destroyed. Hence the method must be called
     * during runtime destruction after sending `VmDeath` event and before uninitializing threads.
     */
    void UnloadDebugger();

    static void CreateDfxController(const RuntimeOptions &options);

    static void BlockSignals();

    inline void InitializeVerifierRuntime();

    Runtime(const RuntimeOptions &options, mem::InternalAllocatorPtr internalAllocator);

    ~Runtime();

    static Runtime *instance_;
    static RuntimeOptions options_;
    static std::string runtimeType_;
    static os::memory::Mutex mutex_;
    static bool isTaskManagerUsed_;

    // NOTE(dtrubenk): put all of it in the permanent space
    mem::InternalAllocatorPtr internalAllocator_;
    RuntimeNotificationManager *notificationManager_;
    ClassLinker *classLinker_;
    ClassHierarchyAnalysis *cha_;
    DProfiler *dprofiler_ = nullptr;
    tooling::MemoryAllocationDumper *memAllocDumper_ = nullptr;

    PandaVM *pandaVm_ = nullptr;

#ifndef PANDA_TARGET_WINDOWS
    SignalManager *signalManager_ {nullptr};
#endif

    // For IDE is real connected.
    bool isDebugMode_ {false};
    bool isDebuggerConnected_ {false};
    bool isProfileableFromShell_ {false};
    os::memory::Mutex debugSessionCreationMutex_ {};
    os::memory::Mutex debugSessionUniquenessMutex_ {};
    DebugSessionHandle debugSession_ {};

    // Additional VMInfo
    std::string processPackageName_;
    std::string processDataDirectory_;

    // For saving class path.
    std::string pandaPathString_;

    AbortHook abort_ = nullptr;
    ExitHook exit_ = nullptr;

    bool zygoteNoThreads_ {false};
    bool isZygote_;
    bool isInitialized_ {false};

    bool saveProfilingInfo_ {false};
    bool incrementalSaveProfilingInfo_ {false};

    bool checksSuspend_ {false};
    bool checksStack_ {true};
    bool checksNullptr_ {true};
    bool isStacktrace_ {false};
    bool isJitEnabled_ {false};
    bool isProfilerEnabled_ {false};
    bool isProfileBranches_ {false};

    bool isDumpNativeCrash_ {true};

    PandaString fingerPrint_ = "unknown";

    // Verification
    ark::verifier::Config *verifierConfig_ = nullptr;
    ark::verifier::Service *verifierService_ = nullptr;

    struct AppContext {
        ClassLinkerContext *ctx {nullptr};
        std::optional<panda_file::SourceLang> lang;
    };
    AppContext appContext_ {};

    RuntimeController *runtimeController_ {nullptr};
    UnwindStackFn unwindStackFn_ {nullptr};

    tooling::Tools tools_;

    EntrypointsTable entrypoints_ {};

    NO_COPY_SEMANTIC(Runtime);
    NO_MOVE_SEMANTIC(Runtime);
};

inline mem::AllocatorAdapter<void> GetInternalAllocatorAdapter(const Runtime *runtime)
{
    return runtime->GetInternalAllocator()->Adapter();
}

void InitSignals();

}  // namespace ark

#endif  // PANDA_RUNTIME_RUNTIME_H_
