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

#include "runtime/monitor_pool.h"

#include "runtime/include/object_header.h"
#include "runtime/include/runtime.h"
#include "runtime/mark_word.h"
#include "runtime/monitor.h"

namespace ark {

Monitor *MonitorPool::CreateMonitor(ObjectHeader *obj)
{
    os::memory::LockHolder lock(poolLock_);
    for (Monitor::MonitorId i = 0; i < MAX_MONITOR_ID; i++) {
        lastId_ = (lastId_ + 1) % MAX_MONITOR_ID;
        if (monitors_.count(lastId_) == 0) {
            auto monitor = allocator_->New<Monitor>(lastId_);
            if (monitor == nullptr) {
                return nullptr;
            }
            monitors_[lastId_] = monitor;
            monitor->SetObject(obj);
            return monitor;
        }
    }
    LOG(FATAL, RUNTIME) << "Out of MonitorPool indexes";
    UNREACHABLE();
}

Monitor *MonitorPool::LookupMonitor(Monitor::MonitorId id)
{
    os::memory::LockHolder lock(poolLock_);
    auto it = monitors_.find(id);
    if (it != monitors_.end()) {
        return it->second;
    }
    return nullptr;
}

void MonitorPool::FreeMonitor(Monitor::MonitorId id)
{
    os::memory::LockHolder lock(poolLock_);
    auto it = monitors_.find(id);
    if (it != monitors_.end()) {
        auto *monitor = it->second;
        monitors_.erase(it);
        allocator_->Delete(monitor);
    }
}

void MonitorPool::DeflateMonitors()
{
    os::memory::LockHolder lock(poolLock_);
    for (auto monitorIter = monitors_.begin(); monitorIter != monitors_.end();) {
        auto monitor = monitorIter->second;
        if (monitor->DeflateInternal()) {
            monitorIter = monitors_.erase(monitorIter);
            allocator_->Delete(monitor);
        } else {
            monitorIter++;
        }
    }
}

void MonitorPool::ReleaseMonitors(MTManagedThread *thread)
{
    os::memory::LockHolder lock(poolLock_);
    for (auto &it : monitors_) {
        auto *monitor = it.second;
        // Recursive lock is possible
        while (monitor->GetOwner() == thread) {
            monitor->Release(thread);
        }
    }
}

PandaSet<Monitor::MonitorId> MonitorPool::GetEnteredMonitorsIds(MTManagedThread *thread)
{
    PandaSet<Monitor::MonitorId> enteredMonitorsIds;
    os::memory::LockHolder lock(poolLock_);
    for (auto &it : monitors_) {
        auto *monitor = it.second;
        if (monitor->GetOwner() == thread) {
            enteredMonitorsIds.insert(monitor->GetId());
        }
    }
    return enteredMonitorsIds;
}

}  // namespace ark
