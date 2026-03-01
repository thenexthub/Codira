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
#ifndef PANDA_RUNTIME_MEM_ALLOC_TRACKER_H
#define PANDA_RUNTIME_MEM_ALLOC_TRACKER_H

#include <list>
#include <map>
#include <atomic>
#include <memory>
#include <iostream>
#include <vector>
#include <unordered_map>
#include "space.h"
#include "libarkbase/os/mutex.h"
#include "libarkbase/utils/span.h"

namespace ark {

class AllocTracker {
public:
    AllocTracker() = default;
    virtual ~AllocTracker() = default;

    virtual void TrackAlloc(void *addr, size_t size, SpaceType space) = 0;
    virtual void TrackFree(void *addr) = 0;

    virtual void Dump() {}
    virtual void Dump([[maybe_unused]] std::ostream &out) {}
    virtual void DumpMemLeaks([[maybe_unused]] std::ostream &out) {}

    NO_COPY_SEMANTIC(AllocTracker);
    NO_MOVE_SEMANTIC(AllocTracker);
};

class SimpleAllocTracker final : public AllocTracker {
public:
    void TrackAlloc(void *addr, size_t size, [[maybe_unused]] SpaceType space) override
    {
        os::memory::LockHolder lock(lock_);
        internalAllocCounter_++;
        totalAllocated_ += size;
        currentAllocated_ += size;
        peakAllocated_ = std::max(peakAllocated_, currentAllocated_);
        auto insResult = allocatedAddresses_.insert({addr, AllocInfo(internalAllocCounter_, size)});
        ASSERT(insResult.second);
        static_cast<void>(insResult);  // Fix compilation in release
    }

    void TrackFree(void *addr) override
    {
        os::memory::LockHolder lock(lock_);
        internalFreeCounter_++;
        auto it = allocatedAddresses_.find(addr);
        ASSERT(it != allocatedAddresses_.end());
        size_t size = it->second.GetSize();
        allocatedAddresses_.erase(it);
        currentAllocated_ -= size;
    }

    void Dump() override
    {
        Dump(std::cout);
    }

    void Dump(std::ostream &out) override
    {
        out << "Internal memory allocations:\n";
        out << "allocations count: " << internalAllocCounter_ << "\n";
        out << "  total allocated: " << totalAllocated_ << "\n";
        out << "   peak allocated: " << peakAllocated_ << "\n";
    }

    void DumpMemLeaks(std::ostream &out) override
    {
        out << "=== Allocated Internal Memory: ===" << std::endl;
        for (auto it : allocatedAddresses_) {
            out << std::hex << it.first << ", allocation #" << std::dec << it.second.GetAllocNumber() << std::endl;
        }
        out << "==================================" << std::endl;
    }

private:
    class AllocInfo {
    public:
        AllocInfo(size_t allocNumber, size_t size) : allocNumber_(allocNumber), size_(size) {}

        size_t GetAllocNumber() const
        {
            return allocNumber_;
        }

        size_t GetSize() const
        {
            return size_;
        }

    private:
        size_t allocNumber_;
        size_t size_;
    };

private:
    size_t internalAllocCounter_ = 0;
    size_t internalFreeCounter_ = 0;
    size_t totalAllocated_ = 0;
    size_t currentAllocated_ = 0;
    size_t peakAllocated_ = 0;
    std::unordered_map<void *, AllocInfo> allocatedAddresses_;
    os::memory::Mutex lock_;
};

class PANDA_PUBLIC_API DetailAllocTracker final : public AllocTracker {
public:
    static constexpr uint32_t ALLOC_TAG = 1;
    static constexpr uint32_t FREE_TAG = 2;

    void TrackAlloc(void *addr, size_t size, SpaceType space) override;
    void TrackFree(void *addr) override;

    void Dump() override;
    void Dump(std::ostream &out) override;
    void DumpMemLeaks(std::ostream &out) override;

private:
    using Stacktrace = std::vector<uintptr_t>;

    class AllocInfo {
    public:
        AllocInfo(uint32_t id, uint32_t size, uint32_t space, uint32_t stacktraceId)
            : id_(id), size_(size), space_(space), stacktraceId_(stacktraceId)
        {
        }

        uint32_t GetTag() const
        {
            return tag_;
        }

        uint32_t GetId() const
        {
            return id_;
        }

        uint32_t GetSize() const
        {
            return size_;
        }

        uint32_t GetSpace() const
        {
            return space_;
        }

        uint32_t GetStacktraceId() const
        {
            return stacktraceId_;
        }

    private:
        const uint32_t tag_ = ALLOC_TAG;
        uint32_t id_;
        uint32_t size_;
        uint32_t space_;
        uint32_t stacktraceId_;
    };

    class FreeInfo {
    public:
        explicit FreeInfo(uint32_t allocId) : allocId_(allocId) {}

        uint32_t GetTag() const
        {
            return tag_;
        }

        uint32_t GetAllocId() const
        {
            return allocId_;
        }

    private:
        const uint32_t tag_ = FREE_TAG;
        uint32_t allocId_;
    };

    void AllocArena() REQUIRES(mutex_);
    uint32_t WriteStacks(std::ostream &out, std::map<uint32_t, uint32_t> *idMap) REQUIRES(mutex_);

private:
    std::atomic<size_t> allocCounter_ = 0;
    uint32_t curId_ GUARDED_BY(mutex_) = 0;
    Span<uint8_t> curArena_ GUARDED_BY(mutex_);
    std::list<std::unique_ptr<uint8_t[]>> arenas_ GUARDED_BY(mutex_);  // NOLINT(modernize-avoid-c-arrays)
    std::list<Stacktrace> stacktraces_ GUARDED_BY(mutex_);
    std::map<void *, AllocInfo *> curAllocs_ GUARDED_BY(mutex_);
    os::memory::Mutex mutex_;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_MEM_ALLOC_TRACKER_H
