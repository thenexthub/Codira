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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_COROUTINE_H
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_COROUTINE_H

#include "runtime/coroutines/coroutine_context.h"
#include "plugins/ets/runtime/ets_napi_env.h"
#include "plugins/ets/runtime/external_iface_table.h"
#include "runtime/coroutines/coroutine.h"
#include "runtime/coroutines/coroutine_manager.h"
#include "runtime/coroutines/local_storage.h"
#include "runtime/include/panda_vm.h"

namespace ark::ets {
class PandaEtsVM;

/// @brief The eTS coroutine. It is aware of the native interface and reference storage existance.
class EtsCoroutine : public Coroutine {
public:
    NO_COPY_SEMANTIC(EtsCoroutine);
    NO_MOVE_SEMANTIC(EtsCoroutine);

    enum class DataIdx { ETS_PLATFORM_TYPES_PTR, INTEROP_CTX_PTR, INTEROP_CALL_STACK_PTR, LAST_ID };
    using LocalStorage = StaticLocalStorage<DataIdx>;

    /**
     * @brief EtsCoroutine factory: the preferrable way to create a coroutine. See CoroutineManager::CoroutineFactory
     * for details.
     *
     * Since C++ requires function type to exactly match the formal parameter type, we have to make this factory a
     * template. The sole purpose for this is to be able to return both Coroutine* and EtsCoroutine*
     */
    template <class T>
    static T *Create(Runtime *runtime, PandaVM *vm, PandaString name, CoroutineContext *context,
                     std::optional<EntrypointInfo> &&epInfo = std::nullopt, Type type = Type::MUTATOR,
                     CoroutinePriority priority = CoroutinePriority::MEDIUM_PRIORITY)
    {
        mem::InternalAllocatorPtr allocator = runtime->GetInternalAllocator();
        auto co = allocator->New<EtsCoroutine>(os::thread::GetCurrentThreadId(), allocator, vm, std::move(name),
                                               context, std::move(epInfo), type, priority);
        ASSERT(co != nullptr);
        co->Initialize();
        return co;
    }

    ~EtsCoroutine() override
    {
        auto allocator = GetVM()->GetHeapManager()->GetInternalAllocator();
        allocator->Delete(etsNapiEnv_);
    }

    static EtsCoroutine *CastFromThread(Thread *thread)
    {
        ASSERT(thread != nullptr);
        return static_cast<EtsCoroutine *>(thread);
    }

    static EtsCoroutine *GetCurrent()
    {
        Coroutine *co = Coroutine::GetCurrent();
        if ((co != nullptr) && co->GetThreadLang() == ark::panda_file::SourceLang::ETS) {
            return CastFromThread(co);
        }
        return nullptr;
    }

    ExternalIfaceTable *GetExternalIfaceTable();

    void SetPromiseClass(void *promiseClass)
    {
        promiseClassPtr_ = promiseClass;
    }

    void SetJobClass(void *jobClass)
    {
        jobClassPtr_ = jobClass;
    }

    static constexpr uint32_t GetTlsPromiseClassPointerOffset()
    {
        return MEMBER_OFFSET(EtsCoroutine, promiseClassPtr_);
    }

    // Returns a unique object representing "null" reference
    ALWAYS_INLINE ObjectHeader *GetNullValue() const
    {
        return nullValue_;
    }

    // For mainthread initializer
    void SetupNullValue(ObjectHeader *obj)
    {
        nullValue_ = obj;
    }

    static constexpr uint32_t GetTlsNullValueOffset()
    {
        return MEMBER_OFFSET(EtsCoroutine, nullValue_);
    }

    static constexpr uint32_t GetTlsNapiEnvOffset()
    {
        return MEMBER_OFFSET(EtsCoroutine, etsNapiEnv_);
    }

    static constexpr uint32_t GetLocalStorageOffset()
    {
        return MEMBER_OFFSET(EtsCoroutine, localStorage_);
    }

    PANDA_PUBLIC_API PandaEtsVM *GetPandaVM() const;
    PANDA_PUBLIC_API CoroutineManager *GetCoroutineManager() const;

    PandaEtsNapiEnv *GetEtsNapiEnv() const
    {
        return etsNapiEnv_;
    }

    void Initialize() override;
    void ReInitialize(PandaString name, CoroutineContext *context, std::optional<EntrypointInfo> &&epInfo,
                      CoroutinePriority priority) override;
    void CleanUp() override;
    void RequestCompletion(Value returnValue) override;
    void FreeInternalMemory() override;
    void UpdateCachedObjects() override;
    void ListUnhandledEventsOnProgramExit() override;

    /// @brief traverse current unhandled failed jobs with custom handler
    void ProcessUnhandledFailedJobs();

    LocalStorage &GetLocalStorage()
    {
        return localStorage_;
    }

    const LocalStorage &GetLocalStorage() const
    {
        return localStorage_;
    }

    // event handlers
    void OnHostWorkerChanged() override;
    void OnContextSwitchedTo() override;
    void OnChildCoroutineCreated(Coroutine *child) override;

    /// @brief print stack and exit the program
    void HandleUncaughtException() override;

    /// The method returns true if there are interop JS code frames in the coroutine call stack
    bool IsContextSwitchRisky() const override;

    void PrintCallStack() const override;

    void SetTaskpoolTaskId(int32_t taskid);

    int32_t GetTaskpoolTaskId() const;

    static constexpr CoroutinePriority ASYNC_CALL = CoroutinePriority::HIGH_PRIORITY;
    static constexpr CoroutinePriority PROMISE_CALLBACK = CoroutinePriority::HIGH_PRIORITY;
    static constexpr CoroutinePriority TIMER_CALLBACK = CoroutinePriority::MEDIUM_PRIORITY;
    static constexpr CoroutinePriority LAUNCH = CoroutinePriority::MEDIUM_PRIORITY;

protected:
    // we would like everyone to use the factory to create a EtsCoroutine
    explicit EtsCoroutine(ThreadId id, mem::InternalAllocatorPtr allocator, PandaVM *vm, PandaString name,
                          CoroutineContext *context, std::optional<EntrypointInfo> &&epInfo, Type type,
                          CoroutinePriority priority);

private:
    panda_file::Type GetReturnType();
    EtsObject *GetReturnValueAsObject(panda_file::Type returnType, Value returnValue);
    EtsObject *GetValueFromPromiseSync(EtsPromise *promise);

    void RequestPromiseCompletion(mem::Reference *promiseRef, Value returnValue);
    void RequestJobCompletion(mem::Reference *jobRef, Value returnValue);

    void ProcessUnhandledRejectedPromises(bool listAllObjects);

    PandaEtsNapiEnv *etsNapiEnv_ {nullptr};
    void *promiseClassPtr_ {nullptr};
    void *jobClassPtr_ {nullptr};

    ObjectHeader *nullValue_ {};

    static ExternalIfaceTable emptyExternalIfaceTable_;

    LocalStorage localStorage_;

    static_assert(std::is_pointer_v<decltype(etsNapiEnv_)>,
                  "we load a raw pointer in compiled code, please don't change the type!");

    // Allocator calls our protected ctor
    friend class mem::Allocator;

    // NOTE(atlantiswang, #32562): this member variable should be moved to the coroutine's local storage
    static constexpr int32_t INVALID_TASKPOOL_TASK_ID = 0;
    int32_t taskpoolTaskid_ {INVALID_TASKPOOL_TASK_ID};
};
}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ETS_COROUTINE_H
