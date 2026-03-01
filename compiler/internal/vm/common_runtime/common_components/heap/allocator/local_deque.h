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

#ifndef COMMON_COMPONENTS_HEAP_ALLOCATOR_ROSALLOC_DEQUE_H
#define COMMON_COMPONENTS_HEAP_ALLOCATOR_ROSALLOC_DEQUE_H

#include <cstddef>
#include <cstdint>

#include "common_components/heap/allocator/memory_map.h"
#include "common_components/log/log.h"

#define DEBUG_DEQUE false
#if DEBUG_DEQUE
#define DEQUE_ASSERT(cond, msg) ASSERT_LOGF(cond, msg)
#else
#define DEQUE_ASSERT(cond, msg) (void(0))
#endif

namespace common {
// Designed for single-use, single-purpose operations
// Supports both queue-like and stack-like operations
// Memory is efficiently cleared in O(1) time by simply resetting pointers
// No need to explicitly free memory, allowing for quick reuse
// Optimized for scenarios where content is discarded after use
template<class ValType>
class SingleUseDeque {
public:
    static constexpr size_t VAL_SIZE = sizeof(ValType);
    void Init(size_t mapSize)
    {
        static_assert(VAL_SIZE == sizeof(void*), "invalid val type");
        MemoryMap::Option opt = MemoryMap::DEFAULT_OPTIONS;
        opt.tag = "arkcommon_alloc_ros_sud";
        opt.reqBase = nullptr;
        memMap_ = MemoryMap::MapMemory(mapSize, mapSize, opt);
#ifdef _WIN64
        MemoryMap::CommitMemory(memMap_->GetBaseAddr(), mapSize);
#endif
        beginAddr_ = reinterpret_cast<HeapAddress>(memMap_->GetBaseAddr());
        endAddr_ = reinterpret_cast<HeapAddress>(memMap_->GetCurrEnd());
        Clear();
    }
    void Init(MemoryMap& other)
    {
        // init from another sud, that is, the two suds share the same mem map
        static_assert(VAL_SIZE == sizeof(void*), "invalid val type");
        memMap_ = &other;
        beginAddr_ = reinterpret_cast<HeapAddress>(memMap_->GetBaseAddr());
        endAddr_ = reinterpret_cast<HeapAddress>(memMap_->GetCurrEnd());
        Clear();
    }
    void Fini() noexcept { MemoryMap::DestroyMemoryMap(memMap_); }
    MemoryMap& GetMemMap() { return *memMap_; }
    bool Empty() const { return topAddr_ < frontAddr_; }
    void Push(ValType v)
    {
        topAddr_ += VAL_SIZE;
        DEQUE_ASSERT(topAddr < endAddr, "not enough memory");
        *reinterpret_cast<ValType*>(topAddr_) = v;
    }
    ValType Top()
    {
        DEQUE_ASSERT(topAddr >= frontAddr, "read empty queue");
        return *reinterpret_cast<ValType*>(topAddr_);
    }
    void Pop()
    {
        DEQUE_ASSERT(topAddr >= frontAddr, "pop empty queue");
        topAddr_ -= VAL_SIZE;
    }
    ValType Front()
    {
        DEQUE_ASSERT(frontAddr <= topAddr, "front reach end");
        return *reinterpret_cast<ValType*>(frontAddr_);
    }
    void PopFront()
    {
        DEQUE_ASSERT(frontAddr <= topAddr, "pop front empty queue");
        frontAddr_ += VAL_SIZE;
    }
    void Clear()
    {
        frontAddr_ = beginAddr_;
        topAddr_ = beginAddr_ - VAL_SIZE;
    }

private:
    MemoryMap* memMap_ = nullptr;
    HeapAddress beginAddr_ = 0;
    HeapAddress frontAddr_ = 0;
    HeapAddress topAddr_ = 0;
    HeapAddress endAddr_ = 0;
};

// This stack-based deque optimizes memory usage by avoiding heap allocations,
// which prevents unnecessary RAM access and eliminates the risk of memory leaks
// from non-releasable memory. However, its capacity is limited by the stack size.
template<class Type>
class LocalDeque {
public:
    static_assert(sizeof(Type) == sizeof(void*), "invalid val type");
    static constexpr int LOCAL_LENGTH = ALLOC_UTIL_PAGE_SIZE / sizeof(Type);
    explicit LocalDeque(SingleUseDeque<Type>& singleUseDeque) : sud_(&singleUseDeque) {}
    ~LocalDeque() = default;
    bool Empty() const { return (top_ < front_) || (front_ == LOCAL_LENGTH && sud_->Empty()); }
    void Push(Type v)
    {
        if (LIKELY_CC(top_ < LOCAL_LENGTH - 1)) {
            array_[++top_] = v;
            return;
        } else if (top_ == LOCAL_LENGTH - 1) {
            ++top_;
            sud_->Clear();
        }
        sud_->Push(v);
    }
    Type Top()
    {
        if (LIKELY_CC(top_ < LOCAL_LENGTH)) {
            DEQUE_ASSERT(top >= front, "read empty queue");
            return array_[top_];
        }
        return sud_->Top();
    }
    void Pop()
    {
        if (LIKELY_CC(top_ < LOCAL_LENGTH)) {
            DEQUE_ASSERT(top >= front, "pop empty queue");
            --top_;
            return;
        }
        DEQUE_ASSERT(top == LOCAL_LENGTH, "pop error");
        sud_->Pop();
        if (sud_->Empty()) {
            // if local array is empty, reuse loacl array
            if (front_ == LOCAL_LENGTH) {
                front_ = 0;
                top_ = -1;
            } else if (front_ < LOCAL_LENGTH) {
                --top_;
                return;
            }
        }
    }
    Type Front()
    {
        if (LIKELY_CC(front_ < LOCAL_LENGTH)) {
            DEQUE_ASSERT(front <= top, "read empty queue front");
            return array_[front_];
        }
        DEQUE_ASSERT(top == LOCAL_LENGTH, "queue front error");
        return sud_->Front();
    }
    void PopFront()
    {
        if (LIKELY_CC(front_ < LOCAL_LENGTH)) {
            DEQUE_ASSERT(front <= top, "pop front empty queue");
            ++front_;
            return;
        }
        DEQUE_ASSERT(front == LOCAL_LENGTH, "pop front error");
        sud_->PopFront();
    }

private:
    int front_ = 0;
    int top_ = -1;
    SingleUseDeque<Type>* sud_;
    Type array_[LOCAL_LENGTH] = { 0 };
};

// this allocator allocates for a certain-sized native data structure
// it is very lightweight but doesn't recycle pages.
template<size_t allocSize, size_t align>
class RTAllocatorT {
    struct Slot {
        Slot* next = nullptr;
    };

public:
    void Init(size_t mapSize)
    {
        static_assert(allocSize >= sizeof(Slot), "invalid alloc size");
        static_assert(align >= alignof(Slot), "invalid align");
        static_assert(allocSize % align == 0, "size not aligned");

        MemoryMap::Option opt = MemoryMap::DEFAULT_OPTIONS;
        opt.tag = "arkcommon_alloc";
        opt.reqBase = nullptr;
        memMap_ = MemoryMap::MapMemory(mapSize, mapSize, opt);
#ifdef _WIN64
        MemoryMap::CommitMemory(memMap_->GetBaseAddr(), mapSize);
#endif
        currAddr_ = reinterpret_cast<HeapAddress>(memMap_->GetBaseAddr());
        endAddr_ = reinterpret_cast<HeapAddress>(memMap_->GetCurrEnd());
        head_ = nullptr;
    }

    void Fini() noexcept { MemoryMap::DestroyMemoryMap(memMap_); }

    void* Allocate()
    {
        void* result = nullptr;
        if (UNLIKELY_CC(this->head_ == nullptr)) {
            DEQUE_ASSERT(this->currAddr + allocSize <= this->endAddr, "not enough memory");
            result = reinterpret_cast<void*>(this->currAddr_);
            this->currAddr_ += allocSize;
        } else {
            result = reinterpret_cast<void*>(this->head_);
            this->head_ = this->head_->next;
        }
        // no zeroing
        return result;
    }

    void Deallocate(void* addr)
    {
        reinterpret_cast<Slot*>(addr)->next = this->head_;
        this->head_ = reinterpret_cast<Slot*>(addr);
    }

private:
    Slot* head_ = nullptr;
    HeapAddress currAddr_ = 0;
    HeapAddress endAddr_ = 0;
    MemoryMap* memMap_ = nullptr;
};
} // namespace common

#endif // COMMON_COMPONENTS_HEAP_ALLOCATOR_ROSALLOC_DEQUE_H
