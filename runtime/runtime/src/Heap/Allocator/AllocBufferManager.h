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


#ifndef MRT_ALLOC_BUFFER_MANAGER
#define MRT_ALLOC_BUFFER_MANAGER

#include <atomic>
#include <functional>
#include <unordered_set>

#include "AllocBuffer.h"
#include "Base/AtomicSpinLock.h"
#include "Common/PageAllocator.h"
#include "Common/RunType.h"
namespace MapleRuntime {
using AllocBufferVisitor = std::function<void(AllocBuffer& buffer)>;
class AllocBufferManager {
public:
    using AllocBuffersSet = std::unordered_set<AllocBuffer*, std::hash<AllocBuffer*>, std::equal_to<AllocBuffer*>,
                                               StdContainerAllocator<AllocBuffer*, ALLOCATOR>>;
    using HungryBuffers = AllocBuffersSet;
    AllocBufferManager() {}
    ~AllocBufferManager()
    {
        for (auto* buffer : allocBuffers) {
            if (buffer != nullptr) {
                delete buffer;
            }
        }
        allocBuffers.clear();
    };

    void RegisterAllocBuffer(AllocBuffer& buffer)
    {
        allocBufferLock.Lock();
        allocBuffers.insert(&buffer);
        allocBufferLock.Unlock();
    }

    void RemoveAllocBuffer(AllocBuffer& buffer)
    {
        allocBufferLock.Lock();
        if (allocBuffers.find(&buffer) != allocBuffers.end()) {
            allocBuffers.erase(&buffer);
        }
        allocBufferLock.Unlock();
    }

    void VisitAllocBuffers(const AllocBufferVisitor& visitor)
    {
        allocBufferLock.Lock();
        for (auto* buffer : allocBuffers) {
            visitor(*buffer);
        }
        allocBufferLock.Unlock();
    }

    void AddHungryBuffer(AllocBuffer& buffer)
    {
        std::lock_guard<std::mutex> lg(hungryBuffersLock);
        hungryBuffers.insert(&buffer);
    }
    void SwapHungryBuffers(HungryBuffers& getBufferSet)
    {
        std::lock_guard<std::mutex> lg(hungryBuffersLock);
        hungryBuffers.swap(getBufferSet);
    }
    size_t GetAllocBufersCount()
    {
        return allocBuffers.size();
    }

private:
    AllocBuffersSet allocBuffers;
    HungryBuffers hungryBuffers;
    std::mutex hungryBuffersLock;
    AtomicSpinLock allocBufferLock;
};
} // namespace MapleRuntime
#endif // MRT_ALLOC_BUFFER_MANAGER
