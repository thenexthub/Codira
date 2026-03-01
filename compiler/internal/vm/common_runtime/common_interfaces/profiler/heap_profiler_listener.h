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

#ifndef COMMON_INTERFACES_HEAP_PROFILER_LISTENER_H
#define COMMON_INTERFACES_HEAP_PROFILER_LISTENER_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <shared_mutex>
#include <mutex>
#include <map>

namespace common {
class HeapProfilerListener {
public:
    static HeapProfilerListener &GetInstance();

    uint32_t RegisterMoveEventCb(const std::function<void(uintptr_t, uintptr_t, size_t)> &cb);
    void UnRegisterMoveEventCb(uint32_t index);
    void OnMoveEvent(uintptr_t fromObj, uintptr_t toObj, size_t size);
    void RegisterOutOfMemoryEventCb(const std::function<void(void *thread)> &cb);
    void OnOutOfMemoryEventCb();

private:
    std::map<uint32_t, std::function<void(uintptr_t, uintptr_t, size_t)>> moveEventCbs_;
    uint32_t moveEventCbId_ {1};
    std::shared_mutex mutex_;
    std::function<void(void *thread)> outOfMemoryEventCb_ = [](void *thread) {};
};
}  // namespace common
#endif  // COMMON_INTERFACES_HEAP_PROFILER_LISTENER_H