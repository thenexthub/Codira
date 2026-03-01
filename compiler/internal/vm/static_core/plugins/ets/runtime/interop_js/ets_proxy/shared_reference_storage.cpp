/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include <cstring>
#include <string>

#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference_storage.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/xgc/xgc.h"

namespace ark::ets::interop::js::ets_proxy {

class SharedReferenceSanity {
public:
    static ALWAYS_INLINE void Kill(SharedReference *ref)
    {
        ASSERT(ref->jsRef_ != nullptr);
        ref->jsRef_ = nullptr;
        ref->ctx_ = nullptr;
        ref->flags_.ClearFlags();
    }

    static ALWAYS_INLINE bool CheckAlive(SharedReference *ref)
    {
        return ref->jsRef_ != nullptr && !ref->flags_.IsEmpty();
    }
};

// NOTE(ipetrov, 20146): It's tempoary solution for SharedReferencesStorage. All napi calls should in native scope
class ScopedNativeCodeThreadIfNeeded {
public:
    explicit ScopedNativeCodeThreadIfNeeded(ManagedThread *thread) : thread_(thread)
    {
        ASSERT(thread_ != nullptr);
        if (thread_->IsInNativeCode()) {
            needToEndNativeCode_ = false;
        } else {
            thread_->NativeCodeBegin();
        }
    }
    NO_COPY_SEMANTIC(ScopedNativeCodeThreadIfNeeded);
    NO_MOVE_SEMANTIC(ScopedNativeCodeThreadIfNeeded);

    ~ScopedNativeCodeThreadIfNeeded()
    {
        if (needToEndNativeCode_) {
            thread_->NativeCodeEnd();
        }
    }

private:
    ManagedThread *thread_ {nullptr};
    bool needToEndNativeCode_ {true};
};

SharedReferenceStorage *SharedReferenceStorage::sharedStorage_ = nullptr;

/* static */
PandaUniquePtr<SharedReferenceStorage> SharedReferenceStorage::Create(PandaEtsVM *vm)
{
    if (sharedStorage_ != nullptr) {
        return nullptr;
    }
    size_t realSize = SharedReferenceStorage::MAX_POOL_SIZE;

    void *data = os::mem::MapRWAnonymousRaw(realSize);
    if (data == nullptr) {
        INTEROP_LOG(FATAL) << "Cannot allocate MemPool";
        return nullptr;
    }
    auto sharedStorage = MakePandaUnique<SharedReferenceStorage>(vm, data, realSize);
    sharedStorage_ = sharedStorage.get();
    vm->AddRootProvider(sharedStorage_);
    return sharedStorage;
}

SharedReferenceStorage::~SharedReferenceStorage()
{
    vm_->RemoveRootProvider(this);
    sharedStorage_ = nullptr;
}

SharedReference *SharedReferenceStorage::GetReference(EtsObject *etsObject) const
{
    os::memory::ReadLockHolder lock(storageLock_);
    ASSERT(SharedReference::HasReference(etsObject));
    return GetItemByIndex(SharedReference::ExtractMaybeIndex(etsObject));
}

napi_status NapiUnwrap(napi_env env, napi_value jsObject, void **result)
{
#if defined(PANDA_JS_ETS_HYBRID_MODE) || defined(PANDA_TARGET_OHOS)
    return napi_xref_unwrap(env, jsObject, result);
#else
    return napi_unwrap(env, jsObject, result);
#endif
}

/* static */
SharedReference *ExtractMaybeReference(napi_env env, napi_value jsObject)
{
    void *data;
    if (UNLIKELY(NapiUnwrap(env, jsObject, &data) != napi_ok)) {
        return nullptr;
    }
    if (UNLIKELY(data == nullptr)) {
        return nullptr;
    }
    // Atomic with acquire order reason: load visibility after shared reference initialization
    return AtomicLoad(static_cast<SharedReference **>(data), std::memory_order_acquire);
}

SharedReference *SharedReferenceStorage::GetReference(napi_env env, napi_value jsObject) const
{
    ScopedNativeCodeThread nativeScope(EtsCoroutine::GetCurrent());
    void *data = ExtractMaybeReference(env, jsObject);
    if (UNLIKELY(data == nullptr)) {
        return nullptr;
    }
    return GetReference(data);
}

SharedReference *SharedReferenceStorage::GetReference(void *data) const
{
    os::memory::ReadLockHolder<os::memory::RWLock, DEBUG_BUILD> lock(storageLock_);
    auto *sharedRef = static_cast<SharedReference *>(data);
    ASSERT(IsValidItem(sharedRef));
    ASSERT(SharedReferenceSanity::CheckAlive(sharedRef));
    return sharedRef;
}

bool SharedReferenceStorage::HasReference(EtsObject *etsObject, napi_env env)
{
    os::memory::ReadLockHolder lock(storageLock_);
    if (!SharedReference::HasReference(etsObject)) {
        return false;
    }
    uint32_t index = SharedReference::ExtractMaybeIndex(etsObject);
    do {
        const SharedReference *currentRef = GetItemByIndex(index);
        if (currentRef->ctx_->GetXGCVmAdaptor()->HasSameEnv(env)) {
            return true;
        }
        index = currentRef->flags_.GetNextIndex();
    } while (index != 0U);
    return false;
}

napi_value SharedReferenceStorage::GetJsObject(EtsObject *etsObject, napi_env env) const
{
    storageLock_.ReadLock();
    const SharedReference *currentRef = GetItemByIndex(SharedReference::ExtractMaybeIndex(etsObject));
    // CC-OFFNXT(G.CTL.03) false positive
    do {
        if (currentRef->ctx_->GetXGCVmAdaptor()->HasSameEnv(env)) {
            auto ref = currentRef->jsRef_;
            storageLock_.Unlock();
            ScopedNativeCodeThreadIfNeeded s(EtsCoroutine::GetCurrent());
            napi_value jsValue;
            NAPI_CHECK_FATAL(napi_get_reference_value(env, ref, &jsValue));
            return jsValue;
        }
        uint32_t index = currentRef->flags_.GetNextIndex();
        if (index == 0U) {
            // NOTE(MockMockBlack, #24062): to be replaced with a runtime exception
            InteropFatal("No JS Object for SharedReference (" + std::to_string(reinterpret_cast<std::uintptr_t>(this)) +
                         ") and napi_env: " + std::to_string(reinterpret_cast<std::uintptr_t>(env)));
        }
        currentRef = GetItemByIndex(index);
    } while (true);
}

bool SharedReferenceStorage::HasReferenceWithCtx(SharedReference *ref, InteropCtx *ctx) const
{
    uint32_t idx;
    do {
        if (ref->ctx_ == ctx) {
            return true;
        }
        idx = ref->flags_.GetNextIndex();
    } while (idx != 0U);
    return false;
}

#if defined(PANDA_JS_ETS_HYBRID_MODE) || defined(PANDA_TARGET_OHOS)
static napi_value WrapEtsObject(InteropCtx *ctx, ets_proxy::SharedReference *ref)
{
    auto *coro = EtsCoroutine::GetCurrent();
    EtsHandleScope scope(coro);
    EtsHandle<EtsObject> etsObject(coro, ref->GetEtsObject());
    auto klass = etsObject->GetClass()->GetRuntimeClass();
    JSRefConvert *refConv = JSRefConvertResolve<true>(ctx, klass);
    napi_value res = refConv->Wrap(ctx, etsObject.GetPtr());
    return res;
}

static napi_value ProxObjectAttachCb([[maybe_unused]] napi_env env, void *data)
{
    auto *coro = Coroutine::GetCurrent();
    ASSERT(coro != nullptr);
    auto *ctx = InteropCtx::Current(coro);
    ASSERT(ctx != nullptr);
    auto *ref = AtomicLoad(static_cast<ets_proxy::SharedReference **>(data), std::memory_order_acquire);
    if (coro->IsManagedCode()) {
        // TaskPool and EWorker call napi_deserialize_hybrid through intrinsics functions
        return WrapEtsObject(ctx, ref);
    }
    ScopedManagedCodeThread v(coro);
    return WrapEtsObject(ctx, ref);
}
#endif

static void DeleteSharedReferenceRefCallback([[maybe_unused]] napi_env env, void *data, [[maybe_unused]] void *hint)
{
    delete static_cast<SharedReference **>(data);
}
static void TriggerXGCIfNeeded([[maybe_unused]] InteropCtx *ctx)
{
    // NOTE(ipetrov, XGC): XGC should be triggered only in hybrid mode. Need to remove when interop will work only in
    // hybrid mode
#ifdef PANDA_JS_ETS_HYBRID_MODE
    XGC::GetInstance()->TriggerGcIfNeeded(ctx->GetPandaEtsVM()->GetGC());
#endif  // PANDA_JS_ETS_HYBRID_MODE
}

static SharedReference **CreateXRef(InteropCtx *ctx, napi_value jsObject, napi_ref *result,
                                    const SharedReferenceStorage::PreInitJSObjectCallback &preInitCallback = nullptr)
{
    auto *currentCoro = EtsCoroutine::GetCurrent();
    ScopedNativeCodeThreadIfNeeded scope(currentCoro);
    napi_env env = ctx->GetJSEnv();
    // Deleter can be called after Runtime::Destroy, so InternalAllocator can not be used
    auto **refRef = new SharedReference *(nullptr);
    if (preInitCallback != nullptr) {
        jsObject = preInitCallback(refRef);
    }
    napi_status status = napi_ok;
#if defined(PANDA_JS_ETS_HYBRID_MODE) || defined(PANDA_TARGET_OHOS)
    status = napi_wrap_with_xref(env, jsObject, refRef, DeleteSharedReferenceRefCallback, ProxObjectAttachCb, result);
#else
    status = napi_wrap(env, jsObject, refRef, DeleteSharedReferenceRefCallback, nullptr, nullptr);
    if (status == napi_ok) {
        status = napi_create_reference(env, jsObject, 1, result);
    }
#endif
    if (UNLIKELY(status != napi_ok)) {
        DeleteSharedReferenceRefCallback(env, refRef, nullptr);
        if (currentCoro->HasPendingException()) {
            ctx->ForwardEtsException(currentCoro);
        }
        ASSERT(ctx->SanityJSExceptionPending());
        return nullptr;
    }
    return refRef;
}

template <SharedReference::InitFn REF_INIT>
SharedReference *SharedReferenceStorage::CreateReference(InteropCtx *ctx, EtsHandle<EtsObject> &etsObject,
                                                         napi_ref jsRef)
{
    SharedReference *sharedRef = AllocItem();
    if (UNLIKELY(sharedRef == nullptr)) {
        ctx->ThrowJSError(ctx->GetJSEnv(), "no free space for SharedReference");
        return nullptr;
    }
    SharedReference *lastRefInChain = nullptr;
    // If EtsObject has been already marked as interop object then add new created SharedReference for a new interop
    // context to chain of references with this EtsObject
    ASSERT(etsObject.GetPtr() != nullptr);
    if (etsObject->HasInteropIndex()) {
        lastRefInChain = GetItemByIndex(etsObject->GetInteropIndex());
        ASSERT(!HasReferenceWithCtx(lastRefInChain, ctx));
        uint32_t index = lastRefInChain->flags_.GetNextIndex();
        while (index != 0U) {
            lastRefInChain = GetItemByIndex(index);
            index = lastRefInChain->flags_.GetNextIndex();
        }
    }
    auto newRefIndex = GetIndexByItem(sharedRef);
    (sharedRef->*REF_INIT)(ctx, etsObject.GetPtr(), jsRef, newRefIndex);
    if (lastRefInChain != nullptr) {
        lastRefInChain->flags_.SetNextIndex(newRefIndex);
    }
    // Ref allocated during XGC, so need to mark it on Remark to avoid removing
    if (isXGCinProgress_) {
        refsAllocatedDuringXGC_.insert(sharedRef);
    }
    LOG(DEBUG, ETS_INTEROP_JS) << "Alloc shared ref: " << sharedRef;
    return sharedRef;
}

template <SharedReference::InitFn REF_INIT>
SharedReference *SharedReferenceStorage::CreateRefCommon(InteropCtx *ctx, EtsHandle<EtsObject> &etsObject,
                                                         napi_value jsObject, const PreInitJSObjectCallback &callback)
{
    TriggerXGCIfNeeded(ctx);
    napi_ref jsRef;
    // Create XRef before SharedReferenceStorage lock to avoid deadlock situation with JS mutator lock in napi calls
    SharedReference **refRef = CreateXRef(ctx, jsObject, &jsRef, callback);
    if (refRef == nullptr) {
        return nullptr;
    }
    os::memory::WriteLockHolder lock(storageLock_);
    auto *sharedRef = CreateReference<REF_INIT>(ctx, etsObject, jsRef);
    if (sharedRef == nullptr) {
        napi_delete_reference(ctx->GetJSEnv(), jsRef);
        return nullptr;
    }
    // Atomic with release order reason: XGC thread should see all writes (initialization of SharedReference) before
    // check initialization status
    AtomicStore(refRef, sharedRef, std::memory_order_release);
    return sharedRef;
}

SharedReference *SharedReferenceStorage::CreateETSObjectRef(InteropCtx *ctx, EtsHandle<EtsObject> &etsObject,
                                                            napi_value jsObject,
                                                            const PreInitJSObjectCallback &callback)
{
    return CreateRefCommon<&SharedReference::InitETSObject>(ctx, etsObject, jsObject, callback);
}

SharedReference *SharedReferenceStorage::CreateJSObjectRef(InteropCtx *ctx, EtsHandle<EtsObject> &etsObject,
                                                           napi_value jsObject)
{
    napi_ref jsRef;
    auto coro = EtsCoroutine::GetCurrent();
    auto env = ctx->GetJSEnv();
    {
        ScopedNativeCodeThread nativeScope(coro);
#if defined(PANDA_JS_ETS_HYBRID_MODE)
        auto status = napi_create_xref(env, jsObject, 1, &jsRef);
#else
        auto status = napi_create_reference(env, jsObject, 1, &jsRef);
#endif
        if (status != napi_ok) {
            ctx->ForwardJSException(coro);
            return nullptr;
        }
    }
    os::memory::WriteLockHolder lock(storageLock_);
    auto *sharedRef = CreateReference<&SharedReference::InitJSObject>(ctx, etsObject, jsRef);
    return sharedRef;
}

SharedReference *SharedReferenceStorage::CreateJSObjectRefwithWrap(InteropCtx *ctx, EtsHandle<EtsObject> &etsObject,
                                                                   napi_value jsObject)
{
    return CreateRefCommon<&SharedReference::InitJSObject>(ctx, etsObject, jsObject);
}

SharedReference *SharedReferenceStorage::CreateHybridObjectRef(InteropCtx *ctx, EtsHandle<EtsObject> &etsObject,
                                                               napi_value jsObject)
{
    return CreateRefCommon<&SharedReference::InitHybridObject>(ctx, etsObject, jsObject);
}

void SharedReferenceStorage::RemoveReference(SharedReference *sharedRef)
{
    ASSERT(Size() > 0U);
    LOG(DEBUG, ETS_INTEROP_JS) << "Remove shared ref: " << sharedRef;
    FreeItem(sharedRef);
    SharedReferenceSanity::Kill(sharedRef);
}

void SharedReferenceStorage::DeleteJSRefAndRemoveReference(SharedReference *sharedRef)
{
    NAPI_CHECK_FATAL(napi_delete_reference(sharedRef->ctx_->GetJSEnv(), sharedRef->jsRef_));
    RemoveReference(sharedRef);
}

void SharedReferenceStorage::DeleteReferenceFromChain(EtsObject *etsObject, SharedReference *prevRef,
                                                      SharedReference *removingRef, uint32_t nextIndex)
{
    DeleteJSRefAndRemoveReference(removingRef);
    // The current reference is head of chain
    if (prevRef == nullptr) {
        if (nextIndex == 0U) {
            // Chain contains only one reference, so no more references for this EtsObject
            etsObject->DropInteropIndex();
        } else {
            // Update interop index to actual head
            etsObject->SetInteropIndex(nextIndex);
        }
    } else {
        prevRef->flags_.SetNextIndex(nextIndex);
    }
}

bool SharedReferenceStorage::DeleteReference(
    SharedReference *sharedRef, const std::function<bool(const SharedReference *sharedRef)> &deletePredicate)
{
    ASSERT(sharedRef != nullptr);
    ASSERT(!sharedRef->IsEmpty());
    auto *etsObject = sharedRef->GetEtsObject();
    // EtsObject contains interop index for the first reference in chain
    uint32_t nextIndex = etsObject->GetInteropIndex();
    ASSERT(nextIndex != 0U);
    SharedReference *prevRef = nullptr;
    do {
        auto *currentRef = GetItemByIndex(nextIndex);
        nextIndex = currentRef->flags_.GetNextIndex();
        if (deletePredicate(currentRef)) {
            DeleteReferenceFromChain(etsObject, prevRef, currentRef, nextIndex);
            return true;
        }
        prevRef = currentRef;
    } while (nextIndex != 0U);
    return false;
}

void SharedReferenceStorage::DeleteAllReferencesWithCtx(const InteropCtx *ctx)
{
    os::memory::WriteLockHolder lock(storageLock_);
    const std::function<bool(const SharedReference *)> contextPredicate = [ctx](const SharedReference *ref) {
        return ctx == ref->ctx_;
    };
    const size_t capacity = Capacity();
    for (size_t i = 1U; i < capacity; ++i) {
        SharedReference *ref = GetItemByIndex(i);
        if (ref->IsEmpty()) {
            continue;
        }
        if (contextPredicate(ref)) {
            [[maybe_unused]] bool isDeleted = DeleteReference(ref, contextPredicate);
            ASSERT(isDeleted == true);
        }
    }
}

void SharedReferenceStorage::DeleteUnmarkedReferences(SharedReference *sharedRef)
{
    ASSERT(sharedRef != nullptr);
    ASSERT(!sharedRef->IsEmpty());
    EtsObject *etsObject = sharedRef->GetEtsObject();
    // Get head of refs chain
    auto currentIndex = etsObject->GetInteropIndex();
    SharedReference *currentRef = GetItemByIndex(currentIndex);
    auto nextIndex = currentRef->flags_.GetNextIndex();
    // if nextIndex is 0, then refs chain contains only 1 element, so need to also drop interop index from EtsObject
    if (LIKELY(nextIndex == 0U)) {
        ASSERT(sharedRef == currentRef);
        DeleteJSRefAndRemoveReference(currentRef);
        etsObject->DropInteropIndex();
        return;
    }
    SharedReference *headRef = currentRef;
    // -- Remove all unmarked refs except head: START --
    SharedReference *prevRef = currentRef;
    currentRef = GetItemByIndex(nextIndex);
    nextIndex = currentRef->flags_.GetNextIndex();
    while (nextIndex != 0U) {
        if (!currentRef->IsMarked()) {
            DeleteJSRefAndRemoveReference(currentRef);
            prevRef->flags_.SetNextIndex(nextIndex);
        } else {
            prevRef = currentRef;
        }
        currentRef = GetItemByIndex(nextIndex);
        nextIndex = currentRef->flags_.GetNextIndex();
    }
    if (!currentRef->IsMarked()) {
        DeleteJSRefAndRemoveReference(currentRef);
        prevRef->flags_.SetNextIndex(0U);
    }
    // -- Remove all unmarked refs except head: FINISH --
    // Check mark state for headRef, we need to exchange or drop interop index in the EtsObject header
    if (!headRef->IsMarked()) {
        nextIndex = headRef->flags_.GetNextIndex();
        DeleteJSRefAndRemoveReference(headRef);
        if (nextIndex == 0U) {
            // Head reference is alone reference in the chain, so need to drop interop index from EtsObject header
            etsObject->DropInteropIndex();
            return;
        }
        // All unmarked references were removed from the chain, so reference after head should marked
        ASSERT(GetItemByIndex(nextIndex)->IsMarked());
        // Update interop index to actual head
        etsObject->SetInteropIndex(nextIndex);
    }
}

void SharedReferenceStorage::NotifyXGCStarted()
{
    os::memory::WriteLockHolder lock(storageLock_);
    isXGCinProgress_ = true;
}

void SharedReferenceStorage::NotifyXGCFinished()
{
    os::memory::WriteLockHolder lock(storageLock_);
    isXGCinProgress_ = false;
}

void SharedReferenceStorage::VisitRoots(const GCRootVisitor &visitor)
{
    // No need lock, because we visit roots on pause and we wait XGC ConcurrentSweep for local GCs
    size_t capacity = Capacity();
    for (size_t i = 1U; i < capacity; ++i) {
        SharedReference *ref = GetItemByIndex(i);
        if (!ref->IsEmpty() && ref->flags_.GetNextIndex() == 0U) {
            visitor(ref->GetGCRoot());
        }
    }
}

void SharedReferenceStorage::UpdateRefs(const GCRootUpdater &gcRootUpdater)
{
    // No need lock, because we visit roots on pause and we wait XGC ConcurrentSweep for local GCs
    size_t capacity = Capacity();
    for (size_t i = 1U; i < capacity; ++i) {
        SharedReference *ref = GetItemByIndex(i);
        if (ref->IsEmpty()) {
            continue;
        }
        ObjectHeader *obj = ref->GetEtsObject()->GetCoreType();
        if (gcRootUpdater(&obj)) {
            ref->SetETSObject(EtsObject::FromCoreType(obj));
        }
    }
}

void SharedReferenceStorage::SweepUnmarkedRefs()
{
    os::memory::WriteLockHolder lock(storageLock_);
    ASSERT_PRINT(refsAllocatedDuringXGC_.empty(),
                 "All references allocted during XGC should be proceesed on ReMark phase, unprocessed refs: "
                     << refsAllocatedDuringXGC_.size());
    size_t capacity = Capacity();
    for (size_t i = 1U; i < capacity; ++i) {
        SharedReference *ref = GetItemByIndex(i);
        if (ref->IsEmpty()) {
            continue;
        }
        // If the reference is unmarked, then we immediately remove all unmarked references from related chain
        if (!ref->IsMarked()) {
            DeleteUnmarkedReferences(ref);
        }
    }
}

void SharedReferenceStorage::UnmarkAll()
{
    os::memory::WriteLockHolder lock(storageLock_);
    size_t capacity = Capacity();
    for (size_t i = 1U; i < capacity; ++i) {
        auto *ref = GetItemByIndex(i);
        if (!ref->IsEmpty()) {
            ref->Unmark();
        }
    }
}

bool SharedReferenceStorage::CheckAlive(void *data)
{
    auto *sharedRef = reinterpret_cast<SharedReference *>(data);
    os::memory::ReadLockHolder lock(storageLock_);
    return IsValidItem(sharedRef) && SharedReferenceSanity::CheckAlive(sharedRef);
}

}  // namespace ark::ets::interop::js::ets_proxy
