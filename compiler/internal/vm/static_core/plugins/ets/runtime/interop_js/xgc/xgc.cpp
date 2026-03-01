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

#include "runtime/include/runtime.h"
#include "runtime/mem/gc/gc_scope.h"
#include "runtime/mem/gc/g1/g1-gc.h"
#include "runtime/mem/gc/g1/xgc-extension-data.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/xgc/xgc.h"
#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference_storage_verifier.h"
#ifdef PANDA_JS_ETS_HYBRID_MODE
#include "native_engine/native_reference.h"
#include "interfaces/inner_api/napi/native_node_hybrid_api.h"
#endif  // PANDA_JS_ETS_HYBRID_MODE

namespace ark::ets::interop::js {
// CC-OFFNXT(G.PRE.02) necessary macro
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_XGC(level) LOG(level, ETS_INTEROP_JS) << "[XGC] "

class XGCScope : public mem::GCScope<mem::TRACE_TIMING> {
public:
    XGCScope(std::string_view name, PandaEtsVM *vm)
        : mem::GCScope<mem::TRACE_TIMING>(name, vm->GetGC()), scopeName_(name)
    {
        ASSERT(vm->GetGC()->GetLastGCCause() == GCTaskCause::CROSSREF_CAUSE);
        LOG_XGC(DEBUG) << scopeName_ << ": start";
    }

    NO_COPY_SEMANTIC(XGCScope);
    NO_MOVE_SEMANTIC(XGCScope);

    ~XGCScope()
    {
        LOG_XGC(DEBUG) << scopeName_ << ": end";
    }

private:
    std::string_view scopeName_ {};
};

XGC *XGC::instance_ = nullptr;

static constexpr XGC::TriggerPolicy GetXGCTriggerPolicyByString(std::string_view policyStr)
{
    if (policyStr == "default") {
        return XGC::TriggerPolicy::DEFAULT;
    }
    if (policyStr == "force") {
        return XGC::TriggerPolicy::FORCE;
    }
    if (policyStr == "never") {
        return XGC::TriggerPolicy::NEVER;
    }
    return XGC::TriggerPolicy::INVALID;
}

XGC::XGC(PandaEtsVM *vm, STSVMInterfaceImpl *stsVmIface, ets_proxy::SharedReferenceStorage *storage)
    : vm_(vm),
      storage_(storage),
      stsVmIface_(stsVmIface),
      minimalThresholdSize_(Runtime::GetCurrent()->GetOptions().GetXgcMinTriggerThreshold()),
      increaseThresholdPercent_(
          std::min(PERCENT_100_U32, Runtime::GetCurrent()->GetOptions().GetXgcTriggerPercentThreshold())),
      treiggerPolicy_(GetXGCTriggerPolicyByString(Runtime::GetCurrent()->GetOptions().GetXgcTriggerType())),
      enableXgcVerifier_(Runtime::GetCurrent()->GetOptions().IsEnableXgcVerifier())
{
    ASSERT(minimalThresholdSize_ <= storage->MaxSize());
    ASSERT(treiggerPolicy_ != XGC::TriggerPolicy::INVALID);
    // Atomic with relaxed order reason: data race with targetThreasholdSize_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    targetThreasholdSize_.store(minimalThresholdSize_, std::memory_order_relaxed);
}

ALWAYS_INLINE static void MarkJsObject(ets_proxy::SharedReference *ref, STSVMInterfaceImpl *stsVmIface)
{
    ASSERT(ref->HasJSFlag());
    LOG_XGC(DEBUG) << "napi_mark_from_object for ref " << ref;
    ref->GetCtx()->GetXGCVmAdaptor()->MarkFromObject(ref->GetJsRef());
    LOG_XGC(DEBUG) << "Notify to JS waiters";
    stsVmIface->NotifyWaiters();
}

ALWAYS_INLINE static void MarkEtsObject(ets_proxy::SharedReference *ref, PandaEtsVM *vm)
{
    ASSERT(ref->HasETSFlag());
    EtsObject *etsObj = ref->GetEtsObject();
    auto *gc = reinterpret_cast<mem::G1GC<EtsLanguageConfig> *>(vm->GetGC());
    LOG_XGC(DEBUG) << "Start marking from " << etsObj << " (" << etsObj->GetClass()->GetDescriptor() << ")";
    gc->MarkObjectRecursively(etsObj->GetCoreType());
}

static auto CreateXObjectHandler(ets_proxy::SharedReferenceStorage *storage, STSVMInterfaceImpl *stsVmIface)
{
    return [storage, stsVmIface](ObjectHeader *obj) {
        auto *etsObj = EtsObject::FromCoreType(obj);
        if (!etsObj->HasInteropIndex()) {
            return;
        }
        // NOTE(audovichenko): Handle multithreading issue.
        ets_proxy::SharedReference::Iterator it(storage->GetReference(etsObj));
        ets_proxy::SharedReference::Iterator end;
        do {
            if (it->HasJSFlag() && it->MarkIfNotMarked()) {
                MarkJsObject(*it, stsVmIface);
            }
            ++it;
        } while (it != end);
    };
}

/* static */
bool XGC::Create(PandaEtsVM *vm, ets_proxy::SharedReferenceStorage *storage, STSVMInterfaceImpl *stsVmIface)
{
    if (instance_ != nullptr) {
        return false;
    }
    if (stsVmIface == nullptr) {
        // JS VM is not ArkJS.
        // NOTE(audovichenko): remove this later
        return true;
    }
    auto xobjHandler = CreateXObjectHandler(storage, stsVmIface);
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    auto *extentionGCData = allocator->New<mem::XGCExtensionData>(xobjHandler);
    if (extentionGCData == nullptr) {
        return false;
    }
    instance_ = allocator->New<XGC>(vm, stsVmIface, storage);
    if (instance_ == nullptr) {
        allocator->Delete(extentionGCData);
        return false;
    }
    auto *gc = vm->GetGC();
    // NOTE(audovichenko): Don't like to use extension data.
    gc->SetExtensionData(extentionGCData);
    gc->AddListener(instance_);
    return true;
}

/* static */
XGC *XGC::GetInstance()
{
    ASSERT(instance_ != nullptr);
    return instance_;
}

/* static */
bool XGC::Destroy()
{
    if (instance_ == nullptr) {
        return false;
    }
    auto *mainCoro = EtsCoroutine::GetCurrent();
    ASSERT(mainCoro != nullptr);
    ASSERT(mainCoro == mainCoro->GetCoroutineManager()->GetMainThread());
    mainCoro->GetPandaVM()->GetGC()->RemoveListener(instance_);
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    allocator->Delete(instance_);
    instance_ = nullptr;
    return true;
}

void XGC::OnAttach([[maybe_unused]] const InteropCtx *context) {}

void XGC::NotifyToFinishXGC()
{
    os::memory::LockHolder lh(finishXgcLock_);
    // Atomic with relaxed order reason: data race with isXGcInProgress_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    isXGcInProgress_.store(false, std::memory_order_relaxed);
    finishXgcCV_.SignalAll();
}

void XGC::WaitForFinishXGC()
{
    os::memory::LockHolder lh(finishXgcLock_);
    // Atomic with relaxed order reason: data race with isXGcInProgress_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    while (isXGcInProgress_.load(std::memory_order_relaxed)) {
        finishXgcCV_.Wait(&finishXgcLock_);
    }
}

void XGC::OnDetach(const InteropCtx *context)
{
    WaitForFinishXGC();
    storage_->DeleteAllReferencesWithCtx(context);
}

void XGC::GCStarted(const GCTask &task, [[maybe_unused]] size_t heapSize)
{
    if (task.reason != GCTaskCause::CROSSREF_CAUSE) {
        return;
    }
    XGCScope xgcStartScope("XGC Start", vm_);
    storage_->NotifyXGCStarted();
    vm_->RemoveRootProvider(storage_);
    // Atomic with relaxed order reason: data race with isXGcInProgress_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    isXGcInProgress_.store(true, std::memory_order_relaxed);
    remarkFinished_ = false;
    beforeGCStorageSize_ = storage_->Size();
}

void XGC::VerifySharedReferences(ets_proxy::XgcStatus status)
{
    ets_proxy::SharedReferenceStorageVerifier::TraverseAllItems(storage_, status);
}

void XGC::GCFinished(const GCTask &task, [[maybe_unused]] size_t heapSizeBeforeGc, [[maybe_unused]] size_t heapSize)
{
    if (task.reason != GCTaskCause::CROSSREF_CAUSE) {
        return;
    }
    if (!remarkFinished_) {
        // Remark was interrupted, so XGC did not Finish on remark phase and do it here
        Finish();
    }
    // NOTE(ipetrov, XGC): if table will be cleared in concurrent, then compute the new size should not be based on
    // the current storage size, need storage size without dead references
    auto newTargetThreshold = this->ComputeNewSize();
    LOG(DEBUG, GC_TRIGGER) << "XGC's new target threshold storage size = " << newTargetThreshold;
    // Atomic with relaxed order reason: data race with targetThreasholdSize_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    targetThreasholdSize_.store(newTargetThreshold, std::memory_order_relaxed);
}

void XGC::GCPhaseStarted(mem::GCPhase phase)
{
    // Atomic with relaxed order reason: data race with isXGcInProgress_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    if (!isXGcInProgress_.load(std::memory_order_relaxed)) {
        return;
    }
    switch (phase) {
        case mem::GCPhase::GC_PHASE_INITIAL_MARK: {
            UnmarkAll();
            {
                XGCScope xgcStartBarrierScope("StartXGCBarrier", vm_);
                stsVmIface_->StartXGCBarrier(nullptr);
            }
            break;
        }
        case mem::GCPhase::GC_PHASE_REMARK: {
            {
                XGCScope xgcRemarkStartScope("RemarkStartBarrier", vm_);
                stsVmIface_->RemarkStartBarrier();
            }
            Remark();
            break;
        }
        default: {
            break;
        }
    }
}

void XGC::GCPhaseFinished(mem::GCPhase phase)
{
    // Atomic with relaxed order reason: data race with isXGcInProgress_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    if (!isXGcInProgress_.load(std::memory_order_relaxed)) {
        return;
    }
    switch (phase) {
        case mem::GCPhase::GC_PHASE_MARK: {
            XGCScope xgcWaitForConcurrentMarkScope("WaitForConcurrentMark", vm_);
            stsVmIface_->WaitForConcurrentMark(nullptr);
            break;
        }
        case mem::GCPhase::GC_PHASE_REMARK: {
            {
                XGCScope xgcRemarkFinishScope("WaitForRemark", vm_);
                stsVmIface_->WaitForRemark(nullptr);
                remarkFinished_ = true;
            }
            // All common phases with other JS GC threads are done and concurrent mark was not interrupted,
            // so XGC may finish immediately after common Remark
            Finish();
            break;
        }
        default: {
            break;
        }
    }
}

void XGC::MarkFromObject([[maybe_unused]] void *data)
{
    if (data == nullptr) {
        return;
    }
#if defined(PANDA_JS_ETS_HYBRID_MODE)
    auto *nativeRef = static_cast<NativeReference *>(data);
    auto *refRef = static_cast<ets_proxy::SharedReference **>(nativeRef->GetData());
    // Atomic with acquire order reason: load visibility after shared reference initialization in mutator thread
    auto *sharedRef = AtomicLoad(refRef, std::memory_order_acquire);
    // Reference is not initialized yet, will be processed on Remark phase
    if (sharedRef == nullptr) {
        return;
    }
    LOG_XGC(DEBUG) << "MarkFromObject for " << sharedRef;
    if (sharedRef->MarkIfNotMarked()) {
        MarkEtsObject(sharedRef, vm_);
    }
#endif  // PANDA_JS_ETS_HYBRID_MODE
}

void XGC::UnmarkAll()
{
    XGCScope xgcInitialMarkkScope("XGC UnmarkAll", vm_);
    storage_->UnmarkAll();
}

void XGC::Remark()
{
    XGCScope remarkScope("SharedRefsRemark", vm_);
    auto *ref = storage_->ExtractRefAllocatedDuringXGC();
    while (ref != nullptr) {
        if (ref->MarkIfNotMarked()) {
            if (ref->HasJSFlag()) {
                MarkJsObject(ref, stsVmIface_);
            }
            if (ref->HasETSFlag()) {
                MarkEtsObject(ref, vm_);
            }
        }
        ref = storage_->ExtractRefAllocatedDuringXGC();
    }
}

void XGC::Sweep()
{
    XGCScope xgcSweepScope("XGC Sweep", vm_);
    storage_->SweepUnmarkedRefs();
}

void XGC::Finish()
{
    XGCScope xgcFinishScope("XGC Finish", vm_);
    vm_->AddRootProvider(storage_);
    if (remarkFinished_) {
        // XGC was not interrupted
        Sweep();
    }
    if (enableXgcVerifier_) {
        VerifySharedReferences(ets_proxy::XgcStatus::XGC_FINISHED);
    }
    storage_->NotifyXGCFinished();
    // Sweep should be done on common STW, so it's critical to have the barrier here
    stsVmIface_->FinishXGCBarrier();
    NotifyToFinishXGC();
}

size_t XGC::ComputeNewSize()
{
    size_t currentStorageSize = storage_->Size();
    size_t delta = (currentStorageSize / PERCENT_100_D) * increaseThresholdPercent_;

    // NOTE(ipetrov, 20146): maybe use an adaptive trigger?
    if (beforeGCStorageSize_ > currentStorageSize) {
        delta = std::max(delta, static_cast<size_t>((beforeGCStorageSize_ - currentStorageSize) *
                                                    (increaseThresholdPercent_ / PERCENT_100_D)));
    }
    return std::min(std::max(currentStorageSize + delta, minimalThresholdSize_), storage_->MaxSize());
}

bool XGC::Trigger([[maybe_unused]] mem::GC *gc, [[maybe_unused]] PandaUniquePtr<GCTask> task)
{
    ASSERT_MANAGED_CODE();
    LOG(DEBUG, GC_TRIGGER) << "Trigger XGC. Current storage size = " << storage_->Size();
    // NOTE(ipetrov, #20146): Iterate over all contexts
    auto *coro = EtsCoroutine::GetCurrent();
    auto *ctx = InteropCtx::Current(coro);
    ASSERT(ctx != nullptr);
    // NOTE(audovichenko): Handle the situation when the function create several equal tasks
    // NOTE(audovichenko): Handle the situation when GC is triggered in one VM but cannot be triggered in another VM.
    if (!ctx->GetXGCVmAdaptor()->StartXRefMarking()) {
        return false;
    }
    if (!gc->Trigger(std::move(task))) {
        ctx->GetXGCVmAdaptor()->NotifyXGCInterruption();
        stsVmIface_->NotifyWaiters();
        return false;
    }
    return true;
}

ALWAYS_INLINE bool XGC::NeedToTriggerXGC([[maybe_unused]] const mem::GC *gc) const
{
    switch (treiggerPolicy_) {
        case TriggerPolicy::FORCE:
            return true;
        case TriggerPolicy::NEVER:
            return false;
        case TriggerPolicy::DEFAULT:
            [[fallthrough]];
        default:
            break;
    }
    // Atomic with relaxed order reason: data race with isXGcInProgress_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    if (isXGcInProgress_.load(std::memory_order_relaxed)) {
        return false;
    }
    // Atomic with relaxed order reason: data race with targetThreasholdSize_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    if (storage_->Size() < targetThreasholdSize_.load(std::memory_order_relaxed)) {
        return false;
    }
#if defined(PANDA_TARGET_OHOS)
    // Don't trigger XGC in high sensitive case
    if (AppStateManager::GetCurrent()->GetAppState().GetState() == AppState::State::SENSITIVE_START) {
        return false;
    }
#endif  // PANDA_TARGET_OHOS
    return true;
}

void XGC::TriggerGcIfNeeded(mem::GC *gc)
{
    if (!NeedToTriggerXGC(gc)) {
        return;
    }
    this->Trigger(gc, MakePandaUnique<GCTask>(GCTaskCause::CROSSREF_CAUSE, time::GetCurrentTimeInNanos()));
}

}  // namespace ark::ets::interop::js
