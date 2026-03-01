/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "plugins/ets/runtime/interop_js/sts_vm_interface_impl.h"
#include "plugins/ets/runtime/interop_js/xgc/xgc.h"

namespace ark::ets::interop::js {

thread_local STSVMInterfaceImpl::XGCSyncState STSVMInterfaceImpl::xgcSyncState_ =
    STSVMInterfaceImpl::XGCSyncState::NONE;

void STSVMInterfaceImpl::MarkFromObject(void *obj)
{
    XGC::GetInstance()->MarkFromObject(obj);
}

STSVMInterfaceImpl::STSVMInterfaceImpl() : xgcBarrier_(STSVMInterface::DEFAULT_XGC_EXECUTORS_COUNT) {}

void STSVMInterfaceImpl::OnVMAttach()
{
    xgcBarrier_.Increment();
}

void STSVMInterfaceImpl::OnVMDetach()
{
    xgcBarrier_.Decrement();
}

bool STSVMInterfaceImpl::StartXGCBarrier(const NoWorkPred &func)
{
    ASSERT(xgcSyncState_ == XGCSyncState::NONE);
    auto res = xgcBarrier_.InitialWait(func);
    if (res) {
        xgcSyncState_ = XGCSyncState::CONCURRENT_PHASE;
    }
    return res;
}

bool STSVMInterfaceImpl::WaitForConcurrentMark(const NoWorkPred &func)
{
    ASSERT(xgcSyncState_ == XGCSyncState::CONCURRENT_PHASE);
    auto res = xgcBarrier_.Wait(func);
    if (res) {
        xgcSyncState_ = XGCSyncState::CONCURRENT_FINISHED;
    }
    return res;
}

void STSVMInterfaceImpl::RemarkStartBarrier()
{
    ASSERT(xgcSyncState_ == XGCSyncState::CONCURRENT_FINISHED);
    xgcBarrier_.Wait();
    xgcSyncState_ = XGCSyncState::REMARK_PHASE;
}

bool STSVMInterfaceImpl::WaitForRemark(const NoWorkPred &func)
{
    ASSERT(xgcSyncState_ == XGCSyncState::REMARK_PHASE);
    auto res = xgcBarrier_.Wait(func);
    if (res) {
        xgcSyncState_ = XGCSyncState::NONE;
    }
    return res;
}

void STSVMInterfaceImpl::FinishXGCBarrier()
{
    xgcBarrier_.Wait();
    xgcSyncState_ = XGCSyncState::NONE;
}

void STSVMInterfaceImpl::NotifyWaiters()
{
    xgcBarrier_.Signal();
}

STSVMInterfaceImpl::VMBarrier::VMBarrier(size_t vmsCount)
{
    os::memory::LockHolder lh(barrierMutex_);
    vmsCount_ = vmsCount;
    currentVMsCount_ = 0U;
    epochCount_ = 0U;
    currentWaitersCount_ = 0U;
    weakCount_ = 0U;
}

void STSVMInterfaceImpl::VMBarrier::Increment()
{
    os::memory::LockHolder lh(barrierMutex_);
    vmsCount_++;
}

void STSVMInterfaceImpl::VMBarrier::Decrement()
{
    os::memory::LockHolder lh(barrierMutex_);
    ASSERT(vmsCount_ > 1);  // after this Decrement, barrier is broken and will not wait correctly
    vmsCount_--;
    Wake();
}

bool STSVMInterfaceImpl::VMBarrier::InitialWait(const NoWorkPred &noWorkPred)
{
    return Wait(noWorkPred, true);
}

bool STSVMInterfaceImpl::VMBarrier::Wait(const NoWorkPred &noWorkPred)
{
    return Wait(noWorkPred, false);
}

bool STSVMInterfaceImpl::VMBarrier::Wait(const NoWorkPred &noWorkPred, bool isInitial)
{
    os::memory::LockHolder lh(barrierMutex_);
    // Need check if noWorkPred exist and if yes, check if it's true. This prevent situation with lost Signal.
    if (CheckNoWorkPred(noWorkPred)) {
        return false;
    }
    size_t epochCount = 0;
    do {
        // Save current epoch to look at it in future.
        epochCount = epochCount_;
        // No we can try to pass the barrier. First we increment count of waiters.
        auto currentWaitersCount = IncrementCurrentWaitersCount();
        // For Inintial barrier we use variable vmsCount_, otherwise we use currentVMsCount_ that can not be checked
        // between 2 Initial barriers.
        size_t waitersCount = 0;
        if (isInitial) {
            waitersCount = vmsCount_;
        } else {
            waitersCount = currentVMsCount_;
        }
        // Next we check if this waiter is the last, if it true, we increase epoch and go out
        if (currentWaitersCount == waitersCount) {
            // for initial barrier we also should set new currentVMsCount_;
            if (isInitial) {
                currentVMsCount_ = vmsCount_;
            }
            epochCount_++;
            Wake();
            return true;
        }
        // We go to wait, it will return true if noWorkPred() returns true
        if (WaitNextEpochOrSignal(noWorkPred)) {
            return false;
        }
    } while (epochCount == epochCount_);
    return true;
}

void STSVMInterfaceImpl::VMBarrier::Signal()
{
    os::memory::LockHolder lh(barrierMutex_);
    Wake();
}

size_t STSVMInterfaceImpl::VMBarrier::IncrementCurrentWaitersCount()
{
    return ++currentWaitersCount_;
}

bool STSVMInterfaceImpl::VMBarrier::WaitNextEpochOrSignal(const NoWorkPred &noWorkPred)
{
    size_t weakCount = weakCount_;
    do {
        condVar_.Wait(&barrierMutex_);
        // This while is needed only to avoid problems with spurious wakeups
    } while (weakCount == weakCount_);
    return CheckNoWorkPred(noWorkPred);
}

void STSVMInterfaceImpl::VMBarrier::Wake()
{
    currentWaitersCount_ = 0;
    weakCount_++;
    condVar_.SignalAll();
}

}  // namespace ark::ets::interop::js
