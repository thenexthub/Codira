/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#include "runtime/monitor_object_lock.h"

#include "libarkbase/os/thread.h"
#include "runtime/include/thread.h"
#include "runtime/handle_scope-inl.h"

namespace ark {
ObjectLock::ObjectLock(ObjectHeader *obj)
    : scope_(HandleScope<ObjectHeader *>(ManagedThread::GetCurrent())),
      objHandler_(VMHandle<ObjectHeader>(ManagedThread::GetCurrent(), obj))
{
    [[maybe_unused]] auto res = Monitor::MonitorEnter(objHandler_.GetPtr());
    ASSERT(res == Monitor::State::OK);
}

bool ObjectLock::Wait(bool ignoreInterruption)
{
    Monitor::State state = Monitor::Wait(objHandler_.GetPtr(), ThreadStatus::IS_WAITING, 0, 0, ignoreInterruption);
    LOG_IF(state == Monitor::State::ILLEGAL, FATAL, RUNTIME) << "Monitor::Wait() failed";
    return state != Monitor::State::INTERRUPTED;
}

bool ObjectLock::TimedWait(uint64_t timeout)
{
    Monitor::State state = Monitor::Wait(objHandler_.GetPtr(), ThreadStatus::IS_TIMED_WAITING, timeout, 0);
    LOG_IF(state == Monitor::State::ILLEGAL, FATAL, RUNTIME) << "Monitor::Wait() failed";
    return state != Monitor::State::INTERRUPTED;
}

void ObjectLock::Notify()
{
    Monitor::State state = Monitor::Notify(objHandler_.GetPtr());
    LOG_IF(state != Monitor::State::OK, FATAL, RUNTIME) << "Monitor::Notify() failed";
}

void ObjectLock::NotifyAll()
{
    Monitor::State state = Monitor::NotifyAll(objHandler_.GetPtr());
    LOG_IF(state != Monitor::State::OK, FATAL, RUNTIME) << "Monitor::NotifyAll() failed";
}

ObjectLock::~ObjectLock()
{
    [[maybe_unused]] auto res = Monitor::MonitorExit(objHandler_.GetPtr());
    ASSERT(res == Monitor::State::OK);
}
}  // namespace ark
