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

#ifndef PANDA_PLUGINGS_ETS_RUNTIME_INTEROP_JS_HYBRID_XGC_XGC_H
#define PANDA_PLUGINGS_ETS_RUNTIME_INTEROP_JS_HYBRID_XGC_XGC_H

#include "hybrid/ecma_vm_interface.h"
#include "plugins/ets/runtime/interop_js/app_state_manager.h"
#include "plugins/ets/runtime/interop_js/sts_vm_interface_impl.h"
#include "runtime/mem/gc/gc_trigger.h"

namespace ark::ets {
class EtsObject;
class PandaEtsVM;
}  // namespace ark::ets

namespace ark::ets::interop::js {

class InteropCtx;
namespace ets_proxy {
class SharedReferenceStorage;
class SharedReferenceStorageVerifier;
enum class XgcStatus;
}  // namespace ets_proxy

/**
 * Cross-reference garbage collector.
 * Implements logic to collect cross-references between JS and ETS in SharedReferenceStorage.
 */
class XGC final : public mem::GCTrigger {
public:
    NO_COPY_SEMANTIC(XGC);
    NO_MOVE_SEMANTIC(XGC);
    ~XGC() override = default;

    /// @enum represents XGC trigger policy types
    enum class TriggerPolicy : uint8_t {
        INVALID,  // Invalid trigger, it should not be used
        DEFAULT,  // Default trigger
        FORCE,    // Run on each allocation
        NEVER,    // Never trigger XGC (disable)
    };

    /**
     * Create instance of XGC if it was not created before. Runtime should be existed before the XGC creation
     * @param mainCoro main coroutine
     * @return true if the instance successfully created, false - otherwise
     */
    [[nodiscard]] PANDA_PUBLIC_API static bool Create(PandaEtsVM *vm, ets_proxy::SharedReferenceStorage *storage,
                                                      STSVMInterfaceImpl *stsVmIface);

    /// @return current instance of XGC
    static XGC *GetInstance();

    /**
     * Destroy the current instance of XGC if the instance is existed. Runtime should be existed before the XGC
     * destruction
     * @return true if the instance successfully destroyed, false - otherwise
     */
    PANDA_PUBLIC_API static bool Destroy();

    /**
     * Check trigger condition and post XGC task to gc queue and
     * trigger XMark for all related JS virtual machines if needed
     * @param gc GC using in the current VM
     */
    void TriggerGcIfNeeded(mem::GC *gc) override;

    /**
     * Notify XGC about the new interop context attached to STS VM
     * @param context attached interop context
     * @remark the passed interop context must not be registered (attached) in XGC
     */
    void OnAttach(const InteropCtx *context);

    /**
     * Notify XGC about the interop context detached from STS VM
     * @param context detached interop context
     * @remark the passed interop context must be registered (attached) in XGC
     * @see OnAttach
     */
    void OnDetach(const InteropCtx *context);

    /**
     * Post XGC task to gc queue and trigger XMark for all related JS virtual machines
     * @param gc GC using in the current VM
     * @return true if XGC successfully triggered, false - otherwise
     */
    bool Trigger(mem::GC *gc, PandaUniquePtr<GCTask> task);

    /// @return XGC trigger type value
    mem::GCTriggerType GetType() const override
    {
        return mem::GCTriggerType::XGC;
    }

    /// GCListener specific public methods ///

    void GCStarted(const GCTask &task, size_t heapSize) override;
    void GCFinished(const GCTask &task, size_t heapSizeBeforeGc, size_t heapSize) override;
    void GCPhaseStarted(mem::GCPhase phase) override;
    void GCPhaseFinished(mem::GCPhase phase) override;

private:
    // For allocation of XGC with the private constructor by internal allocator
    friend mem::Allocator;
    friend void STSVMInterfaceImpl::MarkFromObject([[maybe_unused]] void *obj);

    XGC(PandaEtsVM *vm, STSVMInterfaceImpl *stsVmIface, ets_proxy::SharedReferenceStorage *storage);
    static XGC *instance_;

    /// @return true if need to trigger XGC by policy and special conditions, false - otherwise
    bool NeedToTriggerXGC(const mem::GC *gc) const;

    /// @return new target threshold storage size for XGC trigger
    size_t ComputeNewSize();

    /// Unmark all cross references before initial mark
    void UnmarkAll();

    /**
     * Start marking from specific cross reference object
     * @param data native data from a JS VM with cross reference data
     */
    void MarkFromObject(void *data);

    /// Remark cross references allocated during concurrent mark phase
    void Remark();

    /// Sweep unmarked cross references on STW
    void Sweep();

    /// Finish XGC process (on the end of Remark phase or the end of GC)
    void Finish();

    /**
     * Notify all related waiters that the XGC process has been finished
     * @see WaitForFinishXGC
     * @see Finish
     */
    void NotifyToFinishXGC();

    /**
     * Wait until XGC process completed
     * @see NotifyToFinishXGC
     * @see OnDetach
     */
    void WaitForFinishXGC();

    void VerifySharedReferences(ets_proxy::XgcStatus status);

    /// External specific fields ///

    PandaEtsVM *vm_ {nullptr};
    ets_proxy::SharedReferenceStorage *storage_ {nullptr};
    STSVMInterfaceImpl *stsVmIface_ {nullptr};

    /// XGC current state specific fields ///

    os::memory::Mutex finishXgcLock_;
    os::memory::ConditionVariable finishXgcCV_ GUARDED_BY(finishXgcLock_);
    std::atomic_bool isXGcInProgress_ {false};
    bool remarkFinished_ {false};  // GUARDED_BY(mutatorLock)

    /// Trigger specific fields ///

    size_t beforeGCStorageSize_ {0U};
    const size_t minimalThresholdSize_ {0U};
    const size_t increaseThresholdPercent_ {0U};
    // We can load a value of the variable from several threads, so need to use atomic
    std::atomic<size_t> targetThreasholdSize_ {0U};
    const TriggerPolicy treiggerPolicy_ {TriggerPolicy::INVALID};
    const bool enableXgcVerifier_ {false};
};

}  // namespace ark::ets::interop::js

#endif  // PANDA_PLUGINGS_ETS_RUNTIME_INTEROP_JS_HYBRID_XGC_XGC_H
