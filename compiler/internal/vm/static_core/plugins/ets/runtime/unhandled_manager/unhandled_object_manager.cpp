/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/unhandled_manager/unhandled_object_manager.h"
#include "plugins/ets/runtime/ets_class_linker_context.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_escompat_array.h"
#include "plugins/ets/runtime/types/ets_job.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "runtime/include/method.h"

namespace ark::ets {

static void UpdateObjectsImpl(PandaUnorderedSet<EtsObject *> &objects, const GCRootUpdater &gcRootUpdater)
{
    PandaVector<PandaUnorderedSet<EtsObject *>::node_type> movedObjects {};
    for (auto iter = objects.begin(); iter != objects.end();) {
        auto *obj = (*iter)->GetCoreType();
        [[maybe_unused]] auto *oldObj = obj;
        // assuming that `gcRootUpdater(&obj) == false` means that `obj` was not modified
        if (gcRootUpdater(&obj)) {
            auto oldIter = iter;
            ++iter;
            auto node = objects.extract(oldIter);
            node.value() = EtsObject::FromCoreType(obj);
            movedObjects.push_back(std::move(node));
        } else {
            ASSERT(obj == oldObj);
            ++iter;
        }
    }
    for (auto &node : movedObjects) {
        objects.insert(std::move(node));
    }
}

void UnhandledObjectManager::UpdateObjects(const GCRootUpdater &gcRootUpdater)
{
    os::memory::LockHolder lh(mutex_);
    UpdateObjectsImpl(failedJobs_, gcRootUpdater);
    for (auto &it : rejectedPromises_) {
        UpdateObjectsImpl(it.second, gcRootUpdater);
    }
}

static void VisitObjectsImpl(PandaUnorderedSet<EtsObject *> &objects, const GCRootVisitor &visitor)
{
    PandaVector<EtsObject *> toInsert;
    for (auto it = objects.begin(); it != objects.end();) {
        ObjectHeader *obj = (*it)->GetCoreType();
        ObjectPointerType pointer = ToObjPtr(obj);
        visitor(mem::GCRoot(mem::RootType::ROOT_VM, &pointer));
        if (pointer != ToObjPtr(obj)) {
            toInsert.push_back(reinterpret_cast<EtsObject *>(pointer));
            auto del_it = it++;
            objects.erase(del_it);
        } else {
            ++it;
        }
    }
    for (auto pointer : toInsert) {
        objects.insert(pointer);
    }
}

void UnhandledObjectManager::VisitObjects(const GCRootVisitor &visitor)
{
    os::memory::LockHolder lh(mutex_);
    VisitObjectsImpl(failedJobs_, visitor);
    for (auto &it : rejectedPromises_) {
        VisitObjectsImpl(it.second, visitor);
    }
}

static PandaUnorderedSet<EtsObject *> FlattenMapOfSets(
    PandaUnorderedMap<CoroutineWorker::Id, PandaUnorderedSet<EtsObject *>> &mapOfSets)
{
    PandaUnorderedSet<EtsObject *> result;
    size_t count = 0;
    for (const auto &[_, set] : mapOfSets) {
        count += set.size();
    }
    result.reserve(count);

    for (auto &[_, set] : mapOfSets) {
        result.merge(std::move(set));
    }
    return result;
}

static void AddObjectImpl(PandaUnorderedSet<EtsObject *> &objects, EtsObject *object)
{
    objects.insert(object);
}

static void RemoveObjectImpl(PandaUnorderedSet<EtsObject *> &objects, EtsObject *object)
{
    objects.erase(object);
}

static PandaVector<EtsHandle<EtsObject>> TransformToVectorOfHandles(EtsCoroutine *coro,
                                                                    const PandaUnorderedSet<EtsObject *> &objects)
{
    ASSERT(coro != nullptr);
    PandaVector<EtsHandle<EtsObject>> handleVec;
    handleVec.reserve(objects.size());
    for (auto *obj : objects) {
        ASSERT(!obj->GetCoreType()->IsForwarded());
        handleVec.emplace_back(coro, obj);
    }
    return handleVec;
}

template <typename T>
static EtsHandle<EtsEscompatArray> CreateEtsObjectArrayFromHandles(EtsCoroutine *coro,
                                                                   const PandaVector<EtsHandle<EtsObject>> &handles)
{
    static_assert(std::is_same_v<T, EtsJob> || std::is_same_v<T, EtsPromise>);
    ASSERT(coro != nullptr);
    EtsHandle<EtsEscompatArray> arrayH(coro, EtsEscompatArray::Create(coro, handles.size()));
    if (UNLIKELY(arrayH.GetPtr() == nullptr)) {
        ASSERT(coro->HasPendingException());
        return EtsHandle<EtsEscompatArray>(coro, nullptr);
    }
    size_t i = 0;
    for (auto hobj : handles) {
        auto *objAndReason = EtsEscompatArray::Create(coro, 2U);
        if (UNLIKELY(objAndReason == nullptr)) {
            ASSERT(coro->HasPendingException());
            return EtsHandle<EtsEscompatArray>(coro, nullptr);
        }
        objAndReason->EscompatArraySetUnsafe(0, T::FromEtsObject(hobj.GetPtr())->GetValue(coro));
        objAndReason->EscompatArraySetUnsafe(1, hobj.GetPtr());

        arrayH->EscompatArraySetUnsafe(i, objAndReason);
        ++i;
    }
    return arrayH;
}

template <typename T>
static void ListObjectsFromEtsArray(EtsClassLinker *etsClassLinker, EtsCoroutine *coro,
                                    EtsHandle<EtsEscompatArray> &hobjects)
{
    static_assert(std::is_same_v<T, EtsJob> || std::is_same_v<T, EtsPromise>);
    auto *platformTypes = etsClassLinker->GetEtsClassLinkerExtension()->GetPlatformTypes();
    Method *method {};
    if constexpr (std::is_same_v<T, EtsJob>) {
        method = platformTypes->coreStdProcessListUnhandledJobs->GetPandaMethod();
        LOG(DEBUG, COROUTINES) << "List unhandled failed jobs";
    } else if (std::is_same_v<T, EtsPromise>) {
        method = platformTypes->coreStdProcessListUnhandledPromises->GetPandaMethod();
        LOG(DEBUG, COROUTINES) << "List unhandled rejected promises";
    }
    ASSERT(method != nullptr);
    auto *coroManager = coro->GetCoroutineManager();
    auto evt = Runtime::GetCurrent()->GetInternalAllocator()->New<CompletionEvent>(nullptr, coroManager);
    PandaVector<Value> args = {Value(hobjects->GetCoreType())};
    LaunchResult launchRes = coroManager->LaunchImmediately(
        evt, method, std::move(args), ark::CoroutineWorkerGroup::GenerateExactWorkerId(coro->GetWorker()->GetId()),
        EtsCoroutine::ASYNC_CALL, true);

    if UNLIKELY (launchRes != LaunchResult::OK) {
        LOG(DEBUG, COROUTINES) << "Failed to list unhandled rejections";
        ASSERT(launchRes == LaunchResult::COROUTINES_LIMIT_EXCEED);
        Runtime::GetCurrent()->GetInternalAllocator()->Delete(evt);
        return;
    }
    LOG(DEBUG, COROUTINES) << "List unhandled rejections end";
}

template <typename T>
static void ListUnhandledObjectsImpl(EtsClassLinker *etsClassLinker, EtsCoroutine *coro,
                                     const PandaUnorderedSet<EtsObject *> &objects)
{
    static_assert(std::is_same_v<T, EtsJob> || std::is_same_v<T, EtsPromise>);
    ASSERT(coro != nullptr);
    ASSERT(etsClassLinker != nullptr);
    [[maybe_unused]] EtsHandleScope scope(coro);

    auto handles = TransformToVectorOfHandles(coro, objects);
    auto hEtsArray = CreateEtsObjectArrayFromHandles<T>(coro, handles);
    if (UNLIKELY(hEtsArray.GetPtr() == nullptr)) {
        ASSERT(coro->HasPendingException());
        return;
    }
    ListObjectsFromEtsArray<T>(etsClassLinker, coro, hEtsArray);
}

void UnhandledObjectManager::AddFailedJob(EtsJob *job)
{
    os::memory::LockHolder lh(mutex_);
    AddObjectImpl(failedJobs_, job->AsObject());
}

void UnhandledObjectManager::RemoveFailedJob(EtsJob *job)
{
    os::memory::LockHolder lh(mutex_);
    RemoveObjectImpl(failedJobs_, job->AsObject());
}

void UnhandledObjectManager::ListFailedJobs(EtsCoroutine *coro)
{
    ASSERT_MANAGED_CODE();
    ASSERT(coro != nullptr);
    PandaUnorderedSet<EtsObject *> unhandledObjects {};
    {
        os::memory::LockHolder lh(mutex_);
        if (failedJobs_.empty()) {
            return;
        }
        unhandledObjects.swap(failedJobs_);
    }
    ListUnhandledObjectsImpl<EtsJob>(vm_->GetClassLinker(), coro, unhandledObjects);
}

void UnhandledObjectManager::AddRejectedPromise(EtsPromise *promise, EtsCoroutine *adderCoro)
{
    os::memory::LockHolder lh(mutex_);
    auto workerId = adderCoro->GetWorker()->GetId();
    AddObjectImpl(rejectedPromises_[workerId], promise->AsObject());
}

void UnhandledObjectManager::RemoveRejectedPromise(EtsPromise *promise, EtsCoroutine *removerCoro)
{
    os::memory::LockHolder lh(mutex_);
    auto workerId = removerCoro->GetWorker()->GetId();
    auto it = rejectedPromises_.find(workerId);
    if (it != rejectedPromises_.end()) {
        it->second.erase(promise->AsObject());
        if (it->second.empty()) {
            rejectedPromises_.erase(it);
        }
    }
}

void UnhandledObjectManager::ListRejectedPromises(EtsCoroutine *coro, bool listAllObjects)
{
    ASSERT_MANAGED_CODE();
    ASSERT(coro != nullptr);
    PandaUnorderedSet<EtsObject *> unhandledObjects {};
    if (listAllObjects) {
        PandaUnorderedMap<CoroutineWorker::Id, PandaUnorderedSet<EtsObject *>> rejectedPromisesLocal {};
        {
            os::memory::LockHolder lh(mutex_);
            if (rejectedPromises_.empty()) {
                return;
            }
            rejectedPromisesLocal.swap(rejectedPromises_);
        }
        unhandledObjects = FlattenMapOfSets(rejectedPromisesLocal);
    } else {
        auto workerId = coro->GetWorker()->GetId();
        {
            os::memory::LockHolder lh(mutex_);
            auto it = rejectedPromises_.find(workerId);
            if (it == rejectedPromises_.end() || it->second.empty()) {
                return;
            }
            unhandledObjects.swap(it->second);
            rejectedPromises_.erase(it);
        }
    }

    ListUnhandledObjectsImpl<EtsPromise>(vm_->GetClassLinker(), coro, unhandledObjects);
}

bool UnhandledObjectManager::HasFailedJobObjects() const
{
    os::memory::LockHolder lh(mutex_);
    return !(failedJobs_.empty());
}

bool UnhandledObjectManager::HasRejectedPromiseObjects(EtsCoroutine *coro, bool listAllObjects) const
{
    os::memory::LockHolder lh(mutex_);
    if (listAllObjects) {
        return !(rejectedPromises_.empty());
    }
    auto workerId = coro->GetWorker()->GetId();
    auto it = rejectedPromises_.find(workerId);
    if (it != rejectedPromises_.end()) {
        return !it->second.empty();
    }
    return false;
}

static void InvokeErrorHandlerImpl(EtsClassLinker *etsClassLinker, EtsCoroutine *coro, EtsHandle<EtsObject> &exception)
{
    ASSERT(coro != nullptr);
    auto descr = exception->GetClass()->GetDescriptor();
    if (descr == panda_file_items::class_descriptors::OUT_OF_MEMORY_ERROR ||
        descr == panda_file_items::class_descriptors::STACK_OVERFLOW_ERROR) {
        LOG(ERROR, RUNTIME) << "Unhandled exception: " << exception->GetCoreType()->ClassAddr<Class>()->GetName();
        PROCESS_EXIT(1);
        UNREACHABLE();
    }
    coro->ClearException();
    auto *platformTypes = etsClassLinker->GetEtsClassLinkerExtension()->GetPlatformTypes();
    auto *method = platformTypes->coreStdProcessHandleUncaughtError->GetPandaMethod();
    ASSERT(method != nullptr);
    std::array args = {Value(exception->GetCoreType())};
    LOG(DEBUG, COROUTINES) << "Invoking error handler";
    method->InvokeVoid(coro, args.data());
}

void UnhandledObjectManager::InvokeErrorHandler(EtsCoroutine *coro, EtsHandle<EtsObject> exception)
{
    ASSERT(coro != nullptr);
    ASSERT_MANAGED_CODE();

    InvokeErrorHandlerImpl(vm_->GetClassLinker(), coro, exception);
}

}  // namespace ark::ets
