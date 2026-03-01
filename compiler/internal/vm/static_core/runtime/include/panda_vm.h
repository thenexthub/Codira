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
#ifndef PANDA_RUNTIME_PANDA_VM_H
#define PANDA_RUNTIME_PANDA_VM_H

#include "include/coretypes/line_string.h"
#include "include/runtime_options.h"
#include "jit/profile_saver_worker.h"
#include "runtime/include/locks.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/mem/gc/gc_phase.h"

#include "libarkbase/utils/expected.h"

namespace ark {

class ManagedThread;
class StringTable;
class ThreadManager;

namespace compiler {
class RuntimeInterface;
}  // namespace compiler

namespace mem {
class HeapManager;
class GC;
class GCTrigger;
class ReferenceProcessor;
}  // namespace mem

enum class PandaVMType : size_t { ECMA_VM };  // Deprecated. Only for Compability with js_runtime.

enum class EventLoopRunMode : int { RUN_DEFAULT = 0, RUN_ONCE, RUN_NOWAIT };

class PandaVM {
public:
    static PandaVM *Create(Runtime *runtime, const RuntimeOptions &options, std::string_view runtimeType);

    explicit PandaVM() : mutatorLock_(Locks::NewMutatorLock()) {};

    virtual ~PandaVM() = default;

    static PandaVM *GetCurrent()
    {
        ASSERT(Thread::GetCurrent() != nullptr);
        return Thread::GetCurrent()->GetVM();
    }

    virtual coretypes::String *ResolveString([[maybe_unused]] const panda_file::File &pf,
                                             [[maybe_unused]] panda_file::File::EntityId id)
    {
        return nullptr;
    }
    virtual bool Initialize() = 0;
    virtual bool InitializeFinish() = 0;
    virtual void PreStartup() = 0;
    virtual void PreZygoteFork() = 0;
    virtual void PostZygoteFork() = 0;
    virtual void InitializeGC() = 0;
    virtual void StartGC() = 0;
    virtual void StopGC() = 0;
    virtual void VisitVmRoots(const GCRootVisitor &visitor);
    virtual void UpdateAndSweepVmRefs(const ReferenceUpdater &updater);
    virtual void UninitializeThreads() = 0;
    virtual void SaveProfileInfo() {}

    virtual Expected<int, Runtime::Error> InvokeEntrypoint(Method *entrypoint, const std::vector<std::string> &args);

    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    virtual void HandleReferences([[maybe_unused]] const GCTask &task,
                                  [[maybe_unused]] const mem::GC::ReferenceClearPredicateT &pred)
    {
    }

    virtual void HandleEnqueueReferences() {}
    virtual void HandleGCFinished() {}
    /*
     * Handle some vm related routine (e.g. finalization calls) after GC.
     * Method will be called only in a mutator thread
     */
    virtual void HandleGCRoutineInMutator() {}
    virtual void HandleLdaStr(Frame *frame, BytecodeId stringId);
    virtual void HandleReturnFrame() {}
    virtual void ProcessReferenceFinalizers();
    virtual void BeforeShutdown() {}

    virtual mem::GCStats *GetGCStats() const = 0;
    virtual mem::HeapManager *GetHeapManager() const = 0;
    virtual mem::GC *GetGC() const = 0;
    virtual mem::GCTrigger *GetGCTrigger() const = 0;
    virtual const RuntimeOptions &GetOptions() const = 0;
    virtual ManagedThread *GetAssociatedThread() const = 0;
    virtual StringTable *GetStringTable() const = 0;
    virtual mem::MemStatsType *GetMemStats() const = 0;
    virtual Rendezvous *GetRendezvous() const = 0;
    virtual mem::GlobalObjectStorage *GetGlobalObjectStorage() const = 0;
    virtual MonitorPool *GetMonitorPool() const = 0;
    virtual ThreadManager *GetThreadManager() const = 0;
    virtual coretypes::String *CreateString([[maybe_unused]] Method *ctor, [[maybe_unused]] ObjectHeader *obj)
    {
        UNREACHABLE();
        return nullptr;
    }
    virtual compiler::RuntimeInterface *GetCompilerRuntimeInterface() const
    {
        return nullptr;
    }
    virtual void VisitStringTable(const GCRootVisitor &visitor, mem::VisitGCRootFlags flags)
    {
        GetStringTable()->VisitRoots(visitor, flags);
    }

    virtual void VisitStrings(const StringTable::StringVisitor &visitor)
    {
        GetStringTable()->VisitStrings(visitor);
    }

    // NOTE(maksenov): remove this method after fixing interpreter performance
    virtual PandaVMType GetPandaVMType() const
    {
        // Deprecated. Only for Compability with js_runtime.
        return PandaVMType::ECMA_VM;
    }

    virtual LanguageContext GetLanguageContext() const = 0;
    virtual CompilerInterface *GetCompiler() const = 0;

    virtual ProfileSaverWorker *GetProfileSaverWorker() const
    {
        return nullptr;
    }

    virtual ark::mem::ReferenceProcessor *GetReferenceProcessor() const = 0;

    virtual ObjectHeader *GetOOMErrorObject() = 0;

    virtual void RegisterSignalHandlers() {};

    virtual void DumpForSigQuit([[maybe_unused]] std::ostream &os) const {};

    virtual std::unique_ptr<const panda_file::File> OpenPandaFile(std::string_view location);

    virtual coretypes::String *ResolveStringFromCompiledCode(const panda_file::File &pf, panda_file::File::EntityId id)
    {
        return ResolveString(pf, id);
    }

    virtual coretypes::String *ResolveString([[maybe_unused]] Frame *frame,
                                             [[maybe_unused]] panda_file::File::EntityId id)
    {
        return nullptr;
    }

    virtual coretypes::String *GetNonMovableString(const panda_file::File &pf, panda_file::File::EntityId id) const;

    uint32_t GetFrameExtSize() const
    {
        return frameExtSize_;
    }

    virtual bool IsBytecodeProfilingEnabled() const
    {
        return false;
    }

    virtual void CleanUpTask([[maybe_unused]] Method *method) {};

    virtual bool ShouldEnableDebug();

    virtual bool IsStaticProfileEnabled() const
    {
        return false;
    }

    virtual void DumpHeap(PandaOStringStream *oStr) const
    {
        size_t objCnt = 0;
        *oStr << "Dumping heap" << std::endl;
        GetHeapManager()->IterateOverObjects([&objCnt, &oStr](ObjectHeader *mem) {
            DumpObject(mem, oStr);
            objCnt++;
        });
        *oStr << "Total dumped " << objCnt << std::endl;
    }

    virtual PandaString GetClassesFootprint() const;

    void LoadDebuggerAgent()
    {
        debuggerAgent_ = CreateDebuggerAgent();
    }

    void UnloadDebuggerAgent()
    {
        debuggerAgent_.reset();
    }

    MutatorLock *GetMutatorLock()
    {
        return mutatorLock_;
    }

    const MutatorLock *GetMutatorLock() const
    {
        return mutatorLock_;
    }

    // NOTE(konstanting): a potential candidate for moving out of the core part
    // Cleans up language-specific CFrame resources
    virtual void CleanupCompiledFrameResources([[maybe_unused]] Frame *frame) {}

    virtual bool SupportGCSinglePassCompaction() const
    {
        return false;
    }

    virtual void FreeInternalResources();

    // Current runtime implementation requires event loop support
    // in a core non-language-specific runtime component,
    // particularly StackfulCoroutineManager.
    virtual bool RunEventLoop([[maybe_unused]] EventLoopRunMode mode)
    {
        return false;
    }

    NO_MOVE_SEMANTIC(PandaVM);
    NO_COPY_SEMANTIC(PandaVM);

protected:
    virtual bool CheckEntrypointSignature(Method *entrypoint) = 0;
    virtual Expected<int, Runtime::Error> InvokeEntrypointImpl(Method *entrypoint,
                                                               const std::vector<std::string> &args) = 0;
    virtual void HandleUncaughtException() = 0;

    void SetFrameExtSize(uint32_t size)
    {
        frameExtSize_ = size;
    }

    virtual LoadableAgentHandle CreateDebuggerAgent();

private:
    /// Lock used for preventing object heap modifications (for example at GC<->JIT,ManagedCode interaction during STW)
    MutatorLock *mutatorLock_;
    uint32_t frameExtSize_ {EMPTY_EXT_FRAME_DATA_SIZE};
    LoadableAgentHandle debuggerAgent_;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_PANDA_VM_H
