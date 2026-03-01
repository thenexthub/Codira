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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_PROXY_SHARED_REFERENCE_STORAGE_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_PROXY_SHARED_REFERENCE_STORAGE_H_

#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/mem/items_pool.h"
#include "plugins/ets/runtime/mem/root_provider.h"
#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets::interop::js {
// Forward declarations
class InteropCtx;
}  // namespace ark::ets::interop::js

namespace ark::ets::interop::js::ets_proxy {
class SharedReferenceStorageVerifier;
namespace testing {
class SharedReferenceStorage1GTest;
}  // namespace testing

using SharedReferencePool = ItemsPool<SharedReference, SharedReference::MAX_MARK_BITS>;

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class SharedReferenceStorage : private SharedReferencePool, public mem::RootProvider {
public:
    PANDA_PUBLIC_API static PandaUniquePtr<SharedReferenceStorage> Create(PandaEtsVM *vm);
    ~SharedReferenceStorage() override;

    using PreInitJSObjectCallback = std::function<napi_value(SharedReference **)>;

    /// @return current storage size
    size_t Size() const
    {
        return SharedReferencePool::Size();
    }

    /// @return maximum possible storage size
    size_t MaxSize() const
    {
        return SharedReferencePool::MaxSize();
    }

    PANDA_PUBLIC_API SharedReference *CreateETSObjectRef(InteropCtx *ctx, EtsHandle<EtsObject> &etsObject,
                                                         napi_value jsObject,
                                                         const PreInitJSObjectCallback &preInitCallback = nullptr);

    SharedReference *CreateJSObjectRef(InteropCtx *ctx, EtsHandle<EtsObject> &etsObject, napi_value jsObject);

    SharedReference *CreateJSObjectRefwithWrap(InteropCtx *ctx, EtsHandle<EtsObject> &etsObject, napi_value jsObject);

    SharedReference *CreateHybridObjectRef(InteropCtx *ctx, EtsHandle<EtsObject> &etsObject, napi_value jsObject);

    PANDA_PUBLIC_API SharedReference *GetReference(napi_env env, napi_value jsObject) const;
    PANDA_PUBLIC_API SharedReference *GetReference(EtsObject *etsObject) const;

    napi_value GetJsObject(EtsObject *etsObject, napi_env env) const;
    bool HasReference(EtsObject *etsObject, napi_env env);

    void NotifyXGCStarted();
    void NotifyXGCFinished();

    /**
     * @brief Visit all non-empty refs and call visitor as vm root for EtsObject from the ref
     * @param visitor gc root visitor
     */
    void VisitRoots(const GCRootVisitor &visitor) override;

    /// Update ref in the storage for EtsObject-s after moving phase
    void UpdateRefs(const GCRootUpdater &gcRootUpdater) override;

    /// Remove all unmarked refs after XGC (can be run concurrently)
    void SweepUnmarkedRefs();

    /**
     * Delete all shared references with specific context from the storage
     * @param ctx interop context
     * @remark interop context calls the method on destruction
     */
    void DeleteAllReferencesWithCtx(const InteropCtx *ctx);

    /// Clear mark bit for all shared references
    PANDA_PUBLIC_API void UnmarkAll();

    /**
     * Extract one SharedRefrence from set of references allocated during XGC concurrent marking pahse
     * @return valid SharedReference or nullptr if no more references in the set
     */
    SharedReference *ExtractRefAllocatedDuringXGC()
    {
        os::memory::WriteLockHolder lock(storageLock_);
        if (refsAllocatedDuringXGC_.empty()) {
            return nullptr;
        }
        return refsAllocatedDuringXGC_.extract(refsAllocatedDuringXGC_.begin()).value();
    }

    static constexpr size_t MAX_POOL_SIZE = (sizeof(void *) > 4) ? 1_GB : 64_MB;
    static constexpr std::string_view PROXY_NAPI_WRAPPER = "_proxynapiwrapper";

private:
    SharedReferenceStorage(PandaEtsVM *vm, void *data, size_t size) : SharedReferencePool(data, size), vm_(vm) {}
    NO_COPY_SEMANTIC(SharedReferenceStorage);
    NO_MOVE_SEMANTIC(SharedReferenceStorage);

    template <SharedReference::InitFn REF_INIT>
    SharedReference *CreateRefCommon(InteropCtx *ctx, EtsHandle<EtsObject> &etsObject, napi_value jsObject,
                                     const PreInitJSObjectCallback &callback = nullptr);

    PANDA_PUBLIC_API static SharedReferenceStorage *GetCurrent()
    {
        ASSERT(sharedStorage_ != nullptr);
        return sharedStorage_;
    }

    template <SharedReference::InitFn REF_INIT>
    PANDA_PUBLIC_API inline SharedReference *CreateReference(InteropCtx *ctx, EtsHandle<EtsObject> &etsObject,
                                                             napi_ref jsRef) REQUIRES(storageLock_);
    PANDA_PUBLIC_API SharedReference *GetReference(void *data) const;
    PANDA_PUBLIC_API void RemoveReference(SharedReference *sharedRef);
    /**
     * Remove all unmarked references from related chain which contains passed shared references
     * @param sharedRef non-empty shared reference from this storage table
     * @note the method should be called under the storage write-lock
     */
    void DeleteUnmarkedReferences(SharedReference *sharedRef) REQUIRES(storageLock_);

    /**
     * Delete napi_ref for passed shared reference and remove the shared reference from the table
     * @param sharedRef non-empty shared reference from this storage table
     * @note the method should be called under the storage write-lock
     */
    void DeleteJSRefAndRemoveReference(SharedReference *sharedRef) REQUIRES(storageLock_);

    /**
     * Delete one reference by predicate from references chain related with passed reference
     * @param sharedRef non-empty shared reference from processing references chain
     * @param deletePredicate predicate with condition for a reference deleting
     * @return true if chain contains reference for removing by predicate and the reference was deleted from the chain,
     * false - otherwise
     */
    bool DeleteReference(SharedReference *sharedRef,
                         const std::function<bool(const SharedReference *sharedRef)> &deletePredicate)
        REQUIRES(storageLock_);

    /// Helper method for DeleteReference
    void DeleteReferenceFromChain(EtsObject *etsObject, SharedReference *prevRef, SharedReference *removingRef,
                                  uint32_t nextIndex) REQUIRES(storageLock_);

    bool HasReferenceWithCtx(SharedReference *ref, InteropCtx *ctx) const;

    PANDA_PUBLIC_API bool CheckAlive(void *data);
    friend class SharedReference;
    friend class SharedReference::Iterator;
    friend class testing::SharedReferenceStorage1GTest;
    // Allocator calls our protected ctor
    friend class mem::Allocator;
    friend class SharedReferenceStorageVerifier;

    PANDA_PUBLIC_API static SharedReferenceStorage *sharedStorage_;

    PandaEtsVM *vm_ {nullptr};
    mutable os::memory::RWLock storageLock_;
    bool isXGCinProgress_ GUARDED_BY(storageLock_) {false};  // CC-OFF(G.FMT.03) project code style
    PandaUnorderedSet<SharedReference *> refsAllocatedDuringXGC_ GUARDED_BY(storageLock_);
};

}  // namespace ark::ets::interop::js::ets_proxy

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_PROXY_SHARED_REFERENCE_STORAGE_H_
