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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_VM_H_
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_VM_H_

#include <atomic>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include <libarkfile/include/source_lang_enum.h>

#include "include/mem/panda_smart_pointers.h"
#include "jit/profile_saver_worker.h"
#include "libarkbase/macros.h"
#include "libarkbase/mem/mem.h"
#include "libarkbase/utils/expected.h"
#include "libarkbase/os/mutex.h"
#include "runtime/coroutines/stackful_coroutine.h"
#include "runtime/include/compiler_interface.h"
#include "runtime/include/external_callback_poster.h"
#include "runtime/include/gc_task.h"
#include "runtime/include/language_context.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/method.h"
#include "runtime/include/object_header.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime_options.h"
#include "runtime/include/runtime.h"
#include "runtime/mem/gc/gc_stats.h"
#include "runtime/mem/gc/gc_trigger.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/reference-processor/reference_processor.h"
#include "runtime/mem/heap_manager.h"
#include "runtime/mem/memory_manager.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "runtime/mem/rendezvous.h"
#include "runtime/monitor_pool.h"
#include "runtime/string_table.h"
#include "runtime/thread_manager.h"
#include "plugins/ets/runtime/ets_class_linker.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/mem/ets_gc_stat.h"
#include "runtime/coroutines/coroutine_manager.h"
#include "plugins/ets/runtime/ani/ani.h"
#include "plugins/ets/runtime/ani/verify/ani_verifier.h"
#include "plugins/ets/runtime/ets_native_library_provider.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/mem/root_provider.h"
#include "plugins/ets/runtime/ets_object_state_table.h"
#include "plugins/ets/runtime/ets_stdlib_cache.h"
#include "plugins/ets/runtime/unhandled_manager/unhandled_object_manager.h"

namespace ark::ets {
class DoubleToStringCache;
class FloatToStringCache;
class LongToStringCache;
class EtsAbcRuntimeLinker;
class EtsFinalizableWeakRef;
class FinalizationRegistryManager;

using WalkEventLoopCallback = std::function<void(void *, void *)>;

class PandaEtsVM final : public PandaVM, public ani_vm {  // NOLINT(fuchsia-multiple-inheritance)
public:
    static Expected<PandaEtsVM *, PandaString> Create(Runtime *runtime, const RuntimeOptions &options);
    static bool Destroy(PandaEtsVM *vm);

    ~PandaEtsVM() override;

    PANDA_PUBLIC_API static PandaEtsVM *GetCurrent();

    /**
     * @brief Create TaskManager if needed and set it to runtime
     * @note It's temporary solution, TaskManager will be created outside VM in the future
     *
     * @param options runtime options
     *
     * @return true if TaskManager was created, false - otherwise
     */
    static bool CreateTaskManagerIfNeeded(const RuntimeOptions &options);

    bool Initialize() override;
    bool InitializeFinish() override;

    void PreStartup() override;
    void PreZygoteFork() override;
    void PostZygoteFork() override;
    void InitializeGC() override;
    void StartGC() override;
    void StopGC() override;
    void VisitVmRoots(const GCRootVisitor &visitor) override;
    void UpdateAndSweepVmRefs(const ReferenceUpdater &updater) override;
    void UninitializeThreads() override;

    void HandleReferences(const GCTask &task, const mem::GC::ReferenceClearPredicateT &pred) override;
    void HandleGCRoutineInMutator() override;
    void HandleGCFinished() override;

    const RuntimeOptions &GetOptions() const override
    {
        return Runtime::GetOptions();
    }

    LanguageContext GetLanguageContext() const override
    {
        return Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    }

    coretypes::String *ResolveString(const panda_file::File &pf, panda_file::File::EntityId id) override
    {
        coretypes::String *str = GetStringTable()->GetInternalStringFast(pf, id);
        if (str != nullptr) {
            return str;
        }
        str = GetStringTable()->GetOrInternInternalString(pf, id, GetLanguageContext());
        return str;
    }

    coretypes::String *ResolveString(Frame *frame, panda_file::File::EntityId id) override
    {
        return PandaEtsVM::ResolveString(*frame->GetMethod()->GetPandaFile(), id);
    }

    coretypes::String *CreateString(Method *ctor, ObjectHeader *obj) override;

    Runtime *GetRuntime() const
    {
        return runtime_;
    }

    mem::HeapManager *GetHeapManager() const override
    {
        ASSERT(mm_ != nullptr);
        return mm_->GetHeapManager();
    }

    mem::MemStatsType *GetMemStats() const override
    {
        ASSERT(mm_ != nullptr);
        return mm_->GetMemStats();
    }

    mem::GC *GetGC() const override
    {
        ASSERT(mm_ != nullptr);
        return mm_->GetGC();
    }

    mem::GCTrigger *GetGCTrigger() const override
    {
        ASSERT(mm_ != nullptr);
        return mm_->GetGCTrigger();
    }

    mem::GCStats *GetGCStats() const override
    {
        ASSERT(mm_ != nullptr);
        return mm_->GetGCStats();
    }

    EtsClassLinker *GetClassLinker() const
    {
        return classLinker_.get();
    }

    EtsClassLinkerExtension *GetEtsClassLinkerExtension() const
    {
        return classLinker_.get()->GetEtsClassLinkerExtension();
    }

    mem::GlobalObjectStorage *GetGlobalObjectStorage() const override
    {
        ASSERT(mm_ != nullptr);
        return mm_->GetGlobalObjectStorage();
    }

    mem::ReferenceProcessor *GetReferenceProcessor() const override
    {
        ASSERT(referenceProcessor_ != nullptr);
        return referenceProcessor_;
    }

    StringTable *GetStringTable() const override
    {
        return stringTable_;
    }

    MonitorPool *GetMonitorPool() const override
    {
        return monitorPool_;
    }

    ManagedThread *GetAssociatedThread() const override
    {
        return ManagedThread::GetCurrent();
    }

    ThreadManager *GetThreadManager() const override
    {
        return coroutineManager_;
    }

    FinalizationRegistryManager *GetFinalizationRegistryManager() const
    {
        return finalizationRegistryManager_;
    }

    CoroutineManager *GetCoroutineManager() const
    {
        return static_cast<CoroutineManager *>(GetThreadManager());
    }

    Rendezvous *GetRendezvous() const override
    {
        return rendezvous_;
    }

    CompilerInterface *GetCompiler() const override
    {
        return compiler_;
    }

    UnhandledObjectManager *GetUnhandledObjectManager() const
    {
        return unhandledObjectManager_;
    }

    ProfileSaverWorker *GetProfileSaverWorker() const override
    {
        return saverWorker_;
    }

    const StdlibCache *GetStdlibCache() const
    {
        return stdLibCache_.get();
    }

    PANDA_PUBLIC_API ObjectHeader *GetOOMErrorObject() override;

    PANDA_PUBLIC_API ObjectHeader *GetNullValue() const;

    compiler::RuntimeInterface *GetCompilerRuntimeInterface() const override
    {
        return runtimeIface_;
    }

    bool LoadNativeLibrary(ani_env *env, const PandaString &name, bool shouldVerifyPermission,
                           const PandaString &fileName);

    static PandaEtsVM *FromAniVM(ani_vm *vm)
    {
        return static_cast<PandaEtsVM *>(vm);
    }

    [[noreturn]] static void Abort(const char *message = nullptr);

    std::mt19937 &GetRandomEngine()
    {
        ASSERT(randomEngine_);
        return *randomEngine_;
    }

    bool IsStaticProfileEnabled() const override
    {
        return true;
    }

    void CleanupCompiledFrameResources(Frame *frame) override
    {
        auto *coro = EtsCoroutine::GetCurrent();
        auto *ifaces = coro->GetExternalIfaceTable();
        if (ifaces->GetClearInteropHandleScopesFunction()) {
            ifaces->GetClearInteropHandleScopesFunction()(frame);
        }
    }

    os::memory::Mutex &GetAniBindMutex()
    {
        return aniBindMutex_;
    }

    bool IsVerifyANI() const
    {
        return aniVerifier_.get() != nullptr;
    }

    ani::verify::ANIVerifier *GetANIVerifier()
    {
        return aniVerifier_.get();
    }

    os::memory::Mutex &GetAtomicsMutex()
    {
        return atomicsMutex_;
    }

    bool SupportGCSinglePassCompaction() const override
    {
        return true;
    }

    DoubleToStringCache *GetDoubleToStringCache()
    {
        return doubleToStringCache_;
    }

    FloatToStringCache *GetFloatToStringCache()
    {
        return floatToStringCache_;
    }

    LongToStringCache *GetLongToStringCache()
    {
        return longToStringCache_;
    }

    static constexpr uint32_t GetDoubleToStringCacheOffset()
    {
        return MEMBER_OFFSET(PandaEtsVM, doubleToStringCache_);
    }

    EtsFinalizableWeakRef *RegisterFinalizerForObject(EtsCoroutine *coro, const EtsHandle<EtsObject> &object,
                                                      void (*finalizer)(void *), void *finalizerArg);

    bool UnregisterFinalizerForObject(EtsCoroutine *coro, EtsFinalizableWeakRef *object);

    void CleanFinalizableReferenceList();

    void BeforeShutdown() override;

    /**
     * @brief Method creates CallbackPosterFactory with your implementation. Here we use template to avoid
     * usage of InternalAllocator outside the virtual machine.
     */
    template <class FactoryImpl, class... Args>
    void CreateCallbackPosterFactory(Args... args)
    {
        static_assert(std::is_base_of_v<CallbackPosterFactoryIface, FactoryImpl>);
        static_assert(std::is_constructible_v<FactoryImpl, Args...>);
        ASSERT(callbackPosterFactory_ == nullptr);
        callbackPosterFactory_ = MakePandaUnique<FactoryImpl>(args...);
    }

    /// @brief Method creates CallBackPoster using factory.
    PandaUniquePtr<CallbackPoster> CreateCallbackPoster()
    {
        if (callbackPosterFactory_ == nullptr) {
            return nullptr;
        }
        return callbackPosterFactory_->CreatePoster();
    }

    using RunEventLoopFunction = std::function<bool(ark::EventLoopRunMode)>;

    void SetRunEventLoopFunction(RunEventLoopFunction &&cb)
    {
        ASSERT(!runEventLoop_);
        runEventLoop_ = std::move(cb);
    }

    bool RunEventLoop(ark::EventLoopRunMode mode) override
    {
        if (!runEventLoop_) {
            return false;
        }

        return runEventLoop_(mode);
    }

    using WalkEventLoopFunction = std::function<void(WalkEventLoopCallback &, void *)>;
    void SetWalkEventLoopFunction(WalkEventLoopFunction &&cb)
    {
        ASSERT(!walkEventLoop_);
        walkEventLoop_ = std::move(cb);
    }

    void WalkEventLoop(WalkEventLoopCallback &callback, void *args)
    {
        if (walkEventLoop_) {
            walkEventLoop_(callback, args);
        }
    }

    EtsObjectStateTable *GetEtsObjectStateTable() const
    {
        return objStateTable_.get();
    }

    void FreeInternalResources() override
    {
        objStateTable_->DeflateInfo();
    }

    uint64_t GetFullGCLongTimeCount() const
    {
        ASSERT(fullGCLongTimeListener_);
        return fullGCLongTimeListener_->GetFullGCLongTimeCount();
    }

    PANDA_PUBLIC_API void AddRootProvider(mem::RootProvider *provider);
    PANDA_PUBLIC_API void RemoveRootProvider(mem::RootProvider *provider);

    /// @brief Create application `AbcRuntimeLinker` in managed scope.
    ClassLinkerContext *CreateApplicationRuntimeLinker(const PandaVector<PandaString> &abcFiles);

    /// @brief print stack and exit the program
    void HandleUncaughtException() override;

protected:
    bool CheckEntrypointSignature(Method *entrypoint) override;
    Expected<int, Runtime::Error> InvokeEntrypointImpl(Method *entrypoint,
                                                       const std::vector<std::string> &args) override;

private:
    void InitializeRandomEngine()
    {
        ASSERT(!randomEngine_);
        std::random_device rd;
        randomEngine_.emplace(rd());
    }

    explicit PandaEtsVM(Runtime *runtime, const RuntimeOptions &options, mem::MemoryManager *mm);

    Runtime *runtime_ {nullptr};
    mem::MemoryManager *mm_ {nullptr};
    PandaUniquePtr<EtsClassLinker> classLinker_;
    mem::ReferenceProcessor *referenceProcessor_ {nullptr};
    PandaVector<ObjectHeader *> gcRoots_;
    Rendezvous *rendezvous_ {nullptr};
    ProfileSaverWorker *saverWorker_ {nullptr};
    CompilerInterface *compiler_ {nullptr};
    StringTable *stringTable_ {nullptr};
    MonitorPool *monitorPool_ {nullptr};
    FinalizationRegistryManager *finalizationRegistryManager_ {nullptr};
    CoroutineManager *coroutineManager_ {nullptr};
    mem::Reference *oomObjRef_ {nullptr};
    compiler::RuntimeInterface *runtimeIface_ {nullptr};
    mem::Reference *nullValueRef_ {nullptr};
    mem::Reference *finalizableWeakRefList_ {nullptr};
    os::memory::Mutex finalizableWeakRefListLock_;
    NativeLibraryProvider nativeLibraryProvider_;
    PandaUniquePtr<CallbackPosterFactoryIface> callbackPosterFactory_;
    os::memory::Mutex rootProviderlock_;
    PandaUnorderedSet<mem::RootProvider *> rootProviders_ GUARDED_BY(rootProviderlock_);
    os::memory::Mutex aniBindMutex_;
    PandaUniquePtr<ani::verify::ANIVerifier> aniVerifier_;
    // optional for lazy initialization
    std::optional<std::mt19937> randomEngine_;
    // for JS Atomics
    os::memory::Mutex atomicsMutex_;
    DoubleToStringCache *doubleToStringCache_ {nullptr};
    FloatToStringCache *floatToStringCache_ {nullptr};
    LongToStringCache *longToStringCache_ {nullptr};
    UnhandledObjectManager *unhandledObjectManager_ {nullptr};
    FullGCLongTimeListener *fullGCLongTimeListener_ {nullptr};

    PandaUniquePtr<EtsObjectStateTable> objStateTable_ {nullptr};
    RunEventLoopFunction runEventLoop_ = nullptr;
    WalkEventLoopFunction walkEventLoop_ = nullptr;
    PandaUniquePtr<StdlibCache> stdLibCache_ {nullptr};

    size_t preForkWorkerCount_ = 0;

    NO_MOVE_SEMANTIC(PandaEtsVM);
    NO_COPY_SEMANTIC(PandaEtsVM);

    friend class mem::Allocator;
};

}  // namespace ark::ets

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_ETS_VM_H_
