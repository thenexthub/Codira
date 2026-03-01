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

#ifndef COMMON_COMPONENTS_COMMON_PAGEALLOCATOR_H
#define COMMON_COMPONENTS_COMMON_PAGEALLOCATOR_H

#include <pthread.h>
#if defined(__linux__) || defined(PANDA_TARGET_OHOS) || defined(__APPLE__)
#include <sys/mman.h>
#endif
#include <atomic>
#include <cstdint>
#include <mutex>

#include "common_components/base/globals.h"
#include "common_components/common/run_type.h"
#include "common_components/common/page_pool.h"
#include "common_components/log/log.h"

namespace common {
// when there is a need to use PageAllocator to manage
// the memory for a specific data structure, please add
// a new type
enum AllocationTag : uint32_t {
    // alllocation type for std container
    FINALIZER_PROCESSOR, // manage the std container in FinalizerProcessor
    ALLOCATOR,           // for Allocator
    MUTATOR_LIST,        // for mutator list
    GC_WORK_STACK,       // for gc mark and write barrier
    GC_TASK_QUEUE,       // for gc task queue
    STACK_PTR,           // for stack in stack_grow
    // more to come
    MAX_ALLOCATION_TAG
};

// constants and utility function
class AllocatorUtils {
public:
    AllocatorUtils() = delete;
    ~AllocatorUtils() = delete;
    NO_COPY_SEMANTIC_CC(AllocatorUtils);
    NO_MOVE_SEMANTIC_CC(AllocatorUtils);
    static const size_t ALLOC_PAGE_SIZE;
    static constexpr uint32_t LOG_ALLOC_ALIGNMENT = 3;
    static constexpr uint32_t ALLOC_ALIGNMENT = 1 << LOG_ALLOC_ALIGNMENT;
};

// Allocator manages page allocation and deallocation
class PageAllocator {
    // slots in a page are managed as a linked list
    struct Slot {
        Slot* next = nullptr;
    };

    // pages are linked to each other as a double-linked list.
    // the free slot list and other infomation are also in
    // page header
    class Page {
    public:
        // get a slot from the free slot list
        inline void* Allocate()
        {
            void* result = nullptr;
            if (header_ != nullptr) {
                result = reinterpret_cast<void*>(header_);
                header_ = header_->next;
                --free_;
            }
            return result;
        }

        // return a slot to the free slot list
        inline void Deallocate(void* slot)
        {
            Slot* cur = reinterpret_cast<Slot*>(slot);
            cur->next = header_;
            header_ = cur;
            ++free_;
        }

        inline bool Available() const { return free_ != 0; }

        inline bool Empty() const { return free_ == total_; }

    private:
        Page* prev_ = nullptr;
        Page* next_ = nullptr;
        Slot* header_ = nullptr;
        uint16_t free_ = 0;
        uint16_t total_ = 0;

        friend class PageAllocator;
    };

public:
    PageAllocator() : nonFull_(nullptr), totalPages_(0), slotSize_(0), slotAlignment_(0) {}

    explicit PageAllocator(uint16_t size) : nonFull_(nullptr), totalPages_(0), slotSize_(size)
    {
        slotAlignment_ = AlignUp<uint16_t>(size, AllocatorUtils::ALLOC_ALIGNMENT);
    }

    ~PageAllocator() = default;

    void Destroy() { DestroyList(nonFull_); }

    void Init(uint16_t size)
    {
        slotSize_ = size;
        slotAlignment_ = AlignUp<uint16_t>(size, AllocatorUtils::ALLOC_ALIGNMENT);
    }

    // allocation entrypoint
    void* Allocate()
    {
        void* result = nullptr;
        {
            std::lock_guard<std::mutex> guard(allocLock_);

            // create page if nonFull_ is nullptr
            if (nonFull_ == nullptr) {
                Page* cur = CreatePage();
                DCHECK_CC(cur != nullptr);
                InitPage(*cur);
                ++totalPages_;
                nonFull_ = cur;
                VLOG(DEBUG, "\ttotal pages mapped: %u, slot_size: %u", totalPages_, slotSize_);
            }

            result = nonFull_->Allocate();

            if (!(nonFull_->Available())) {
                // move from nonFull to full
                Page* cur = nonFull_;
                RemoveFromList(nonFull_, *cur);
            }
        }
        if (result != nullptr) {
            LOGF_CHECK(memset_s(result, slotSize_, 0, slotSize_) == EOK) << "memset_s fail";
        }
        return result;
    }

    // deallocation entrypoint
    void Deallocate(void* slot)
    {
        Page* current = reinterpret_cast<Page*>(
            AlignDown<uintptr_t>(reinterpret_cast<uintptr_t>(slot), AllocatorUtils::ALLOC_PAGE_SIZE));

        std::lock_guard<std::mutex> lockGuard(allocLock_);
        // Transition from full to non-full state if the current resource is unavailable.
        if (!current->Available()) {
            AddToList(nonFull_, *current);
        }
        current->Deallocate(slot);
        if (current->Empty()) {
            RemoveFromList(nonFull_, *current);
            DestroyPage(*current);
            --totalPages_;
            VLOG(DEBUG, "\ttotal pages mapped: %u, slot_size: %u", totalPages_, slotSize_);
        }
    }

private:
    // get a page from os
    static inline Page* CreatePage() { return reinterpret_cast<Page*>(PagePool::Instance().GetPage()); }

    // return the page to os
    static inline void DestroyPage(Page& page)
    {
        LOGF_CHECK(page.free_ == page.total_) << "\t destroy page in use: total = " <<
        page.total_ << ", free = " << page.free_;
        DLOG(ALLOC, "\t destroy page %p total = %u, free = %u", &page, page.total_, page.free_);
        PagePool::Instance().ReturnPage(reinterpret_cast<uint8_t*>(&page));
    }

    // construct the data structure of a new allocated page
    void InitPage(Page& page)
    {
        page.prev_ = nullptr;
        page.next_ = nullptr;
        constexpr uint32_t offset = AlignUp<uint32_t>(sizeof(Page), AllocatorUtils::ALLOC_ALIGNMENT);
        page.free_ = (AllocatorUtils::ALLOC_PAGE_SIZE - offset) / slotAlignment_;
        page.total_ = page.free_;
        LOGF_CHECK(page.free_ >= 1) << "use the wrong allocator! slot size = " << slotAlignment_;

        char* start{ reinterpret_cast<char*>(&page) };
        char* slot{ start + offset };
        page.header_ = reinterpret_cast<Slot*>(slot);
        Slot* prevSlot{ page.header_ };
        char* end{ start + AllocatorUtils::ALLOC_PAGE_SIZE - 1 };
        while (true) {
            slot += slotAlignment_;
            char* slotEnd{ slot + slotAlignment_ - 1 };
            if (slotEnd > end) {
                break;
            }

            Slot* cur{ reinterpret_cast<Slot*>(slot) };
            prevSlot->next = cur;
            prevSlot = cur;
        }

        DLOG(ALLOC,
             "new page start = %p, end = %p, slot header = %p, total slots = %u, slot size = %u, sizeof(Page) = %u",
             start, end, page.header_, page.total_, slotAlignment_, sizeof(Page));
    }

    // linked-list management
    inline void AddToList(Page*& list, Page& page) const
    {
        if (list != nullptr) {
            list->prev_ = &page;
        }
        page.next_ = list;
        list = &page;
    }

    inline void RemoveFromList(Page*& list, Page& page) const
    {
        Page* prev = page.prev_;
        Page* next = page.next_;
        if (&page == list) {
            list = next;
            if (next != nullptr) {
                next->prev_ = nullptr;
            }
        } else {
            prev->next_ = next;
            if (next != nullptr) {
                next->prev_ = prev;
            }
        }
        page.next_ = nullptr;
        page.prev_ = nullptr;
    }

    inline void DestroyList(Page*& linkedList)
    {
        Page* current{ nullptr };
        while (linkedList != nullptr) {
            current = linkedList;
            linkedList = linkedList->next_;
            DestroyPage(*current);
        }
    }

    Page* nonFull_;
    std::mutex allocLock_;
    uint32_t totalPages_;
    uint16_t slotSize_;
    uint16_t slotAlignment_;
};

// Utility class used for StdContainerAllocator
// It has lots of PageAllocators, each for different slot size,
// so all allocation sizes can be handled by this bridge class.
class AggregateAllocator {
public:
    static constexpr uint32_t MAX_ALLOCATORS = 53;

    NO_INLINE_CC PUBLIC_API static AggregateAllocator& Instance(AllocationTag tag);

    AggregateAllocator()
    {
        for (uint32_t i = 0; i < MAX_ALLOCATORS; ++i) {
            allocator_[i].Init(static_cast<uint16_t>(RUNTYPE_RUN_IDX_TO_SIZE(i)));
        }
    }
    ~AggregateAllocator() = default;

    // choose appropriate allocation to allocate
    void* Allocate(size_t size)
    {
        uint32_t alignedSize = AlignUp(static_cast<uint32_t>(size), AllocatorUtils::ALLOC_ALIGNMENT);
        if (alignedSize <= RUN_ALLOC_LARGE_SIZE) {
            uint32_t index = RUNTYPE_SIZE_TO_RUN_IDX(alignedSize);
            return allocator_[index].Allocate();
        } else {
            return PagePool::Instance().GetPage(size);
        }
    }

    NO_INLINE_CC void Deallocate(void* p, size_t size)
    {
        uint32_t alignedSize = AlignUp(static_cast<uint32_t>(size), AllocatorUtils::ALLOC_ALIGNMENT);
        if (alignedSize <= RUN_ALLOC_LARGE_SIZE) {
            uint32_t index = RUNTYPE_SIZE_TO_RUN_IDX(alignedSize);
            allocator_[index].Deallocate(p);
        } else {
            PagePool::Instance().ReturnPage(reinterpret_cast<uint8_t*>(p), size);
        }
    }

private:
    PageAllocator allocator_[MAX_ALLOCATORS];
};

// Allocator is used to take control of memory allocation for std containers.
// It uses AggregateAllocator to dispatch the memory operation to appropriate PageAllocator.
template<class T, AllocationTag Cat>
class StdContainerAllocator {
public:
    using const_pointer = const T*;
    using const_reference = const T&;
    using difference_type = ptrdiff_t;
    using pointer = T*;
    using reference = T&;
    using size_type = size_t;
    using value_type = T;

    using propagate_on_container_swap = std::true_type;
    using propagate_on_container_copy_assignment = std::false_type;
    using propagate_on_container_move_assignment = std::true_type;

    template<class U>
    struct rebind {
        using other = StdContainerAllocator<U, Cat>;
    };

    StdContainerAllocator() = default;
    ~StdContainerAllocator() = default;

    template<class U>
    StdContainerAllocator(const StdContainerAllocator<U, Cat>&)
    {}

    StdContainerAllocator(const StdContainerAllocator<T, Cat>&) {}

    StdContainerAllocator(StdContainerAllocator<T, Cat>&&) {}

    StdContainerAllocator<T, Cat>& operator=(const StdContainerAllocator<T, Cat>&) { return *this; }

    StdContainerAllocator<T, Cat>& operator=(StdContainerAllocator<T, Cat>&&) { return *this; }

    pointer address(reference x) const { return std::addressof(x); }

    const_pointer address(const_reference x) const { return std::addressof(x); }

    pointer allocate(size_type n, const void* hint __attribute__((unused)) = 0)
    {
        pointer result = static_cast<pointer>(AggregateAllocator::Instance(Cat).Allocate(sizeof(T) * n));
        return result;
    }

    void deallocate(pointer p, size_type n) { AggregateAllocator::Instance(Cat).Deallocate(p, sizeof(T) * n); }

    size_type max_size() const { return static_cast<size_type>(~0) / sizeof(value_type); }

    void construct(pointer p, const_reference val) { ::new (reinterpret_cast<void*>(p)) value_type(val); }

    template<class Up, class... Args>
    void construct(Up* p, Args&&... args)
    {
        ::new (reinterpret_cast<void*>(p)) Up(std::forward<Args>(args)...);
    }

    void destroy(pointer p) { p->~value_type(); }
};
// vector::swap requires that the allocator defined by ourselves should be comparable during compiling period,
// so we overload operator == and return true.
template<typename Tp, AllocationTag tag>
inline bool operator==(StdContainerAllocator<Tp, tag>&, StdContainerAllocator<Tp, tag>&) noexcept
{
    return true;
}
} // namespace common
#endif
