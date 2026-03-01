/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_THREAD_INL_H_
#define PANDA_RUNTIME_THREAD_INL_H_

#include "runtime/handle_base.h"
#include "runtime/global_handle_storage-inl.h"
#include "runtime/handle_storage-inl.h"
#include "runtime/include/thread.h"
#include "runtime/include/panda_vm.h"

namespace ark {

template <>
inline void ManagedThread::PushHandleScope<coretypes::TaggedType>(HandleScope<coretypes::TaggedType> *handleScope)
{
    taggedHandleScopes_.push_back(handleScope);
}

template <>
inline HandleScope<coretypes::TaggedType> *ManagedThread::PopHandleScope<coretypes::TaggedType>()
{
    HandleScope<coretypes::TaggedType> *scope = taggedHandleScopes_.back();
    taggedHandleScopes_.pop_back();
    return scope;
}

template <>
inline HandleScope<coretypes::TaggedType> *ManagedThread::GetTopScope<coretypes::TaggedType>() const
{
    if (taggedHandleScopes_.empty()) {
        return nullptr;
    }
    return taggedHandleScopes_.back();
}

template <>
inline HandleStorage<coretypes::TaggedType> *ManagedThread::GetHandleStorage<coretypes::TaggedType>() const
{
    return taggedHandleStorage_;
}

template <>
inline GlobalHandleStorage<coretypes::TaggedType> *ManagedThread::GetGlobalHandleStorage<coretypes::TaggedType>() const
{
    return taggedGlobalHandleStorage_;
}

template <>
inline void ManagedThread::PushHandleScope<ObjectHeader *>(HandleScope<ObjectHeader *> *handleScope)
{
    objectHeaderHandleScopes_.push_back(handleScope);
}

template <>
inline HandleScope<ObjectHeader *> *ManagedThread::PopHandleScope<ObjectHeader *>()
{
    HandleScope<ObjectHeader *> *scope = objectHeaderHandleScopes_.back();
    objectHeaderHandleScopes_.pop_back();
    return scope;
}

template <>
inline HandleScope<ObjectHeader *> *ManagedThread::GetTopScope<ObjectHeader *>() const
{
    if (objectHeaderHandleScopes_.empty()) {
        return nullptr;
    }
    return objectHeaderHandleScopes_.back();
}

template <>
inline HandleStorage<ObjectHeader *> *ManagedThread::GetHandleStorage<ObjectHeader *>() const
{
    return objectHeaderHandleStorage_;
}

template <bool CHECK_NATIVE_STACK, bool CHECK_IFRAME_STACK>
ALWAYS_INLINE inline bool ManagedThread::StackOverflowCheck()
{
    if (!StackOverflowCheckResult<CHECK_NATIVE_STACK, CHECK_IFRAME_STACK>()) {
        // we're going to throw exception that will use the reserved stack space, so disable check
        DisableStackOverflowCheck();
        ark::ThrowStackOverflowException(this);
        // after finish throw exception, restore overflow check
        EnableStackOverflowCheck();
        return false;
    }
    return true;
}

ALWAYS_INLINE inline MonitorPool *MTManagedThread::GetMonitorPool()
{
    return GetVM()->GetMonitorPool();
}

ALWAYS_INLINE inline int32_t MTManagedThread::GetMonitorCount()
{
    // Atomic with relaxed order reason: data race with monitor_count_ with no synchronization or ordering constraints
    // imposed on other reads or writes
    return monitorCount_.load(std::memory_order_relaxed);
}

ALWAYS_INLINE inline void MTManagedThread::AddMonitor(Monitor *monitor)
{
    // Atomic with relaxed order reason: data race with monitor_count_ with no synchronization or ordering constraints
    // imposed on other reads or writes
    monitorCount_.fetch_add(1, std::memory_order_relaxed);
    LOG(DEBUG, RUNTIME) << "Adding monitor " << monitor->GetId();
}

ALWAYS_INLINE inline void MTManagedThread::RemoveMonitor(Monitor *monitor)
{
    // Atomic with relaxed order reason: data race with monitor_count_ with no synchronization or ordering constraints
    // imposed on other reads or writes
    monitorCount_.fetch_sub(1, std::memory_order_relaxed);
    LOG(DEBUG, RUNTIME) << "Removing monitor " << monitor->GetId();
}

ALWAYS_INLINE inline void MTManagedThread::ReleaseMonitors()
{
    if (GetMonitorCount() > 0) {
        GetMonitorPool()->ReleaseMonitors(this);
    }
    ASSERT(GetMonitorCount() == 0);
}

}  // namespace ark

#endif  // PANDA_RUNTIME_THREAD_INL_H_
