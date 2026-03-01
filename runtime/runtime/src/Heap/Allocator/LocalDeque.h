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


#ifndef MRT_ROSALLOC_DEQUE_H
#define MRT_ROSALLOC_DEQUE_H

#include <cstddef>
#include <cstdint>

#include "Base/Panic.h"
#include "MemMap.h"

#define DEBUG_DEQUE false
#if DEBUG_DEQUE
#define DEQUE_ASSERT(cond, msg) MRT_ASSERT(cond, msg)
#else
#define DEQUE_ASSERT(cond, msg) (void(0))
#endif

namespace MapleRuntime {
// this deque is single-use, meaning we can use it, then after a while,
// we can discard its whole content
// under this assumption, clearing this data structure is really fast
// (each clear takes O(1) time)
// also, this assumes that the underlying memory does not need to be freed
// because we can reuse it after each clear
//
// it can be used like a queue or a stack
template<class ValType>
class SingleUseDeque {
public:
    static constexpr size_t VAL_SIZE = sizeof(ValType);
    void Init(size_t mapSize)
    {
        static_assert(VAL_SIZE == sizeof(void*), "invalid val type");
        MemMap::Option opt = MemMap::DEFAULT_OPTIONS;
        opt.tag = "maple_alloc_ros_sud";
        opt.reqBase = nullptr;
        memMap = MemMap::MapMemory(mapSize, mapSize, opt);
#ifdef _WIN64
        MemMap::CommitMemory(memMap->GetBaseAddr(), mapSize);
#endif
        beginAddr = reinterpret_cast<MAddress>(memMap->GetBaseAddr());
        endAddr = reinterpret_cast<MAddress>(memMap->GetCurrEnd());
        Clear();
    }
    void Init(MemMap& other)
    {
        // init from another sud, that is, the two suds share the same mem map
        static_assert(VAL_SIZE == sizeof(void*), "invalid val type");
        memMap = &other;
        beginAddr = reinterpret_cast<MAddress>(memMap->GetBaseAddr());
        endAddr = reinterpret_cast<MAddress>(memMap->GetCurrEnd());
        Clear();
    }
    void Fini() noexcept { MemMap::DestroyMemMap(memMap); }
    MemMap& GetMemMap() { return *memMap; }
    bool Empty() const { return topAddr < frontAddr; }
    void Push(ValType v)
    {
        topAddr += VAL_SIZE;
        DEQUE_ASSERT(topAddr < endAddr, "not enough memory");
        *reinterpret_cast<ValType*>(topAddr) = v;
    }
    ValType Top()
    {
        DEQUE_ASSERT(topAddr >= frontAddr, "read empty queue");
        return *reinterpret_cast<ValType*>(topAddr);
    }
    void Pop()
    {
        DEQUE_ASSERT(topAddr >= frontAddr, "pop empty queue");
        topAddr -= VAL_SIZE;
    }
    ValType Front()
    {
        DEQUE_ASSERT(frontAddr <= topAddr, "front reach end");
        return *reinterpret_cast<ValType*>(frontAddr);
    }
    void PopFront()
    {
        DEQUE_ASSERT(frontAddr <= topAddr, "pop front empty queue");
        frontAddr += VAL_SIZE;
    }
    void Clear()
    {
        frontAddr = beginAddr;
        topAddr = beginAddr - VAL_SIZE;
    }

private:
    MemMap* memMap = nullptr;
    MAddress beginAddr = 0;
    MAddress frontAddr = 0;
    MAddress topAddr = 0;
    MAddress endAddr = 0;
};

// this deque lives on the stack, hence local
// this is better than the above deque because it avoids visiting ram
// and it also avoids using unfreeable memory
// however, its capacity is limited
template<class ValType>
class LocalDeque {
public:
    static_assert(sizeof(ValType) == sizeof(void*), "invalid val type");
    static constexpr int LOCAL_LENGTH = ALLOC_UTIL_PAGE_SIZE / sizeof(ValType);
    explicit LocalDeque(SingleUseDeque<ValType>& singleUseDeque) : sud(&singleUseDeque) {}
    ~LocalDeque() = default;
    bool Empty() const { return (top < front) || (front == LOCAL_LENGTH && sud->Empty()); }
    void Push(ValType v)
    {
        if (LIKELY(top < LOCAL_LENGTH - 1)) {
            array[++top] = v;
            return;
        } else if (top == LOCAL_LENGTH - 1) {
            ++top;
            sud->Clear();
        }
        sud->Push(v);
    }
    ValType Top()
    {
        if (LIKELY(top < LOCAL_LENGTH)) {
            DEQUE_ASSERT(top >= front, "read empty queue");
            return array[top];
        }
        return sud->Top();
    }
    void Pop()
    {
        if (LIKELY(top < LOCAL_LENGTH)) {
            DEQUE_ASSERT(top >= front, "pop empty queue");
            --top;
            return;
        }
        DEQUE_ASSERT(top == LOCAL_LENGTH, "pop error");
        sud->Pop();
        if (sud->Empty()) {
            // if local array is empty, reuse loacl array
            if (front == LOCAL_LENGTH) {
                front = 0;
                top = -1;
            } else if (front < LOCAL_LENGTH) {
                --top;
                return;
            }
        }
    }
    ValType Front()
    {
        if (LIKELY(front < LOCAL_LENGTH)) {
            DEQUE_ASSERT(front <= top, "read empty queue front");
            return array[front];
        }
        DEQUE_ASSERT(top == LOCAL_LENGTH, "queue front error");
        return sud->Front();
    }
    void PopFront()
    {
        if (LIKELY(front < LOCAL_LENGTH)) {
            DEQUE_ASSERT(front <= top, "pop front empty queue");
            ++front;
            return;
        }
        DEQUE_ASSERT(front == LOCAL_LENGTH, "pop front error");
        sud->PopFront();
    }

private:
    int front = 0;
    int top = -1;
    SingleUseDeque<ValType>* sud;
    ValType array[LOCAL_LENGTH] = { 0 };
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

        MemMap::Option opt = MemMap::DEFAULT_OPTIONS;
        opt.tag = "maplert_alloc";
        opt.reqBase = nullptr;
        memMap = MemMap::MapMemory(mapSize, mapSize, opt);
#ifdef _WIN64
        MemMap::CommitMemory(memMap->GetBaseAddr(), mapSize);
#endif
        currAddr = reinterpret_cast<MAddress>(memMap->GetBaseAddr());
        endAddr = reinterpret_cast<MAddress>(memMap->GetCurrEnd());
    }

    void Fini() noexcept { MemMap::DestroyMemMap(memMap); }

    void* Allocate()
    {
        void* result = nullptr;
        if (UNLIKELY(this->head == nullptr)) {
            DEQUE_ASSERT(this->currAddr + allocSize <= this->endAddr, "not enough memory");
            result = reinterpret_cast<void*>(this->currAddr);
            this->currAddr += allocSize;
        } else {
            result = reinterpret_cast<void*>(this->head);
            this->head = this->head->next;
        }
        // no zeroing
        return result;
    }

    void Deallocate(void* addr)
    {
        reinterpret_cast<Slot*>(addr)->next = this->head;
        this->head = reinterpret_cast<Slot*>(addr);
    }

private:
    Slot* head = nullptr;
    MAddress currAddr = 0;
    MAddress endAddr = 0;
    MemMap* memMap = nullptr;
};
} // namespace MapleRuntime

#endif // MRT_ROSALLOC_DEQUE_H
