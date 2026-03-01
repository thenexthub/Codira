/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <optional>
#include "runtime/coroutines/coroutine_manager.h"

namespace ark {

CoroutineManager::CoroutineManager(const CoroutineManagerConfig &config, CoroutineFactory factory)
    : coFactory_(factory), config_(config)
{
    ASSERT(factory != nullptr);
}

Coroutine *CoroutineManager::CreateMainCoroutine(Runtime *runtime, PandaVM *vm)
{
    CoroutineContext *ctx = CreateCoroutineContext(false);
    auto *main = coFactory_(runtime, vm, "_main_", ctx, std::nullopt, Coroutine::Type::MUTATOR,
                            CoroutinePriority::MEDIUM_PRIORITY);
    ASSERT(main != nullptr);

    Coroutine::SetCurrent(main);
    main->InitBuffers();
#ifdef ARK_HYBRID
    auto hasJsRuntime = common::ThreadHolder::GetCurrent() != nullptr;
    main->LinkToExternalHolder(true);
    // We need to unbind mutator before binding in the RequestResume,
    // as it was bound during JS runtime creation/execution
    if (hasJsRuntime) {
        main->UnbindMutator();
    }
#endif
    main->RequestResume();
    main->NativeCodeBegin();

    SetMainThread(main);
    return main;
}

void CoroutineManager::DestroyMainCoroutine()
{
    auto *main = static_cast<Coroutine *>(GetMainThread());
    auto *context = main->GetContext<CoroutineContext>();
    main->Destroy();
    Coroutine::SetCurrent(nullptr);
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(main);
    DeleteCoroutineContext(context);
}

Coroutine *CoroutineManager::CreateEntrypointlessCoroutine(Runtime *runtime, PandaVM *vm, bool makeCurrent,
                                                           PandaString name, Coroutine::Type type,
                                                           CoroutinePriority priority)
{
    if (GetCoroutineCount() >= GetCoroutineCountLimit()) {
        // resource limit reached
        return nullptr;
    }
    // In current design user (not system) coroutine is EP-ful mutator
    ASSERT(CoroutineManager::IsSystemCoroutine(type, false));
    CoroutineContext *ctx = CreateCoroutineContext(false);
    if (ctx == nullptr) {
        // do not proceed if we cannot create a context for the new coroutine
        return nullptr;
    }
    auto *co = coFactory_(runtime, vm, std::move(name), ctx, std::nullopt, type, priority);
    ASSERT(co != nullptr);
    co->InitBuffers();
    if (makeCurrent) {
        Coroutine::SetCurrent(co);
        co->LinkToExternalHolder(true);
        co->RequestResume();
        co->NativeCodeBegin();
    }
    return co;
}

void CoroutineManager::DestroyEntrypointlessCoroutine(Coroutine *co)
{
    ASSERT(co != nullptr);
    ASSERT(!co->HasManagedEntrypoint() && !co->HasNativeEntrypoint());

    auto *context = co->GetContext<CoroutineContext>();
    co->Destroy();
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(co);
    DeleteCoroutineContext(context);
}

Coroutine *CoroutineManager::CreateCoroutineInstance(EntrypointInfo &&epInfo, PandaString name, Coroutine::Type type,
                                                     CoroutinePriority priority)
{
    if (GetCoroutineCount() >= GetCoroutineCountLimit()) {
        // resource limit reached
        return nullptr;
    }
    if (!IsSystemCoroutine(type, true) && IsUserCoroutineLimitReached()) {
        // user coro limit reached
        return nullptr;
    }
    CoroutineContext *ctx = CreateCoroutineContext(true);
    if (ctx == nullptr) {
        // do not proceed if we cannot create a context for the new coroutine
        return nullptr;
    }
    // assuming that the main coro is already created and Coroutine::GetCurrent is not nullptr
    ASSERT(Coroutine::GetCurrent() != nullptr);
    auto *coro = coFactory_(Runtime::GetCurrent(), Coroutine::GetCurrent()->GetVM(), std::move(name), ctx,
                            std::move(epInfo), type, priority);
    return coro;
}

void CoroutineManager::DestroyEntrypointfulCoroutine(Coroutine *co)
{
    DeleteCoroutineContext(co->GetContext<CoroutineContext>());
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(co);
}

CoroutineManager::CoroutineFactory CoroutineManager::GetCoroutineFactory()
{
    return coFactory_;
}

uint32_t CoroutineManager::AllocateCoroutineId()
{
    // Taken by copy-paste from MTThreadManager. Need to generalize if possible.
    // NOTE(konstanting, #IAD5MH): try to generalize internal ID allocation
    os::memory::LockHolder lock(idsLock_);
    for (size_t i = 0; i < coroutineIds_.size(); i++) {
        lastCoroutineId_ = (lastCoroutineId_ + 1) % coroutineIds_.size();
        if (!coroutineIds_[lastCoroutineId_]) {
            coroutineIds_.set(lastCoroutineId_);
            return lastCoroutineId_ + 1;  // 0 is reserved as uninitialized value.
        }
    }
    LOG(FATAL, COROUTINES) << "Out of coroutine ids";
    UNREACHABLE();
}

void CoroutineManager::FreeCoroutineId(uint32_t id)
{
    // Taken by copy-paste from MTThreadManager. Need to generalize if possible.
    // NOTE(konstanting, #IAD5MH): try to generalize internal ID allocation
    id--;  // 0 is reserved as uninitialized value.
    os::memory::LockHolder lock(idsLock_);
    ASSERT(coroutineIds_[id]);
    coroutineIds_.reset(id);
}

void CoroutineManager::SetSchedulingPolicy(CoroutineSchedulingPolicy policy)
{
    os::memory::LockHolder lock(policyLock_);
    schedulingPolicy_ = policy;
}

CoroutineSchedulingPolicy CoroutineManager::GetSchedulingPolicy() const
{
    os::memory::LockHolder lock(policyLock_);
    return schedulingPolicy_;
}

void CoroutineManager::InitializeManagedStructures()
{
    ASSERT(Coroutine::GetCurrent() == GetMainThread());
    EnumerateWorkers([](CoroutineWorker *worker) {
        worker->InitializeManagedStructures();
        return true;
    });
    Coroutine::GetCurrent()->UpdateCachedObjects();
}

}  // namespace ark
