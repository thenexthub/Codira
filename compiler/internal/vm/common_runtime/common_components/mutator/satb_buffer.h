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

#ifndef COMMON_COMPONENTS_MUTATOR_SATB_BUFFER_H
#define COMMON_COMPONENTS_MUTATOR_SATB_BUFFER_H

#include "common_components/common/page_pool.h"
#include "common_components/common/mark_work_stack.h"
#include "common_components/log/log.h"

namespace common {
// snapshot at the beginning buffer
// mainly used to buffer modified field of mutator write
class SatbBuffer {
public:
    static constexpr size_t INITIAL_PAGES = 64;    // 64 pages of initial satb buffer
    static constexpr size_t CACHE_LINE_ALIGN = 64; // for most hardware platfrom, the cache line is 64-byte aigned.
    static SatbBuffer& Instance() noexcept;

    static bool ShouldEnqueue(const BaseObject* obj);

    class TreapNode {
        friend class SatbBuffer;

    public:
        TreapNode() : top_(container_), next_(nullptr) {}
        ~TreapNode() = default;
        bool IsEmpty() const { return reinterpret_cast<size_t>(top_) == reinterpret_cast<size_t>(container_); }
        bool IsFull() const
        {
            static_assert((sizeof(TreapNode) % sizeof(BaseObject**)) == 0, "Satb node must be aligned");
            return top_ == &container_[CONTAINER_CAPACITY_];
        }
        void Clear()
        {
            size_t size = reinterpret_cast<uintptr_t>(top_) - reinterpret_cast<uintptr_t>(container_);
            LOGF_CHECK((memset_s(container_, sizeof(container_), 0, size) == EOK)) << "memset fail\n";
            top_ = container_;
        }
        void Push(const BaseObject* obj)
        {
            std::lock_guard<std::mutex> lg(syncLock_);
            *top_ = const_cast<BaseObject*>(obj);
            top_++;
        }
        template<typename T>
        void GetObjects(T& stack)
        {
            ASSERT_LOGF(top_ <= &container_[CONTAINER_CAPACITY_], "invalid node");
            std::lock_guard<std::mutex> lg(syncLock_);
            BaseObject** head = container_;
            while (head != top_) {
                stack.push_back(*head);
                head++;
            }
            Clear();
        }

    private:
#if defined(_WIN64)
        static constexpr size_t CONTAINER_CAPACITY_ = 69;
#elif defined(__aarch64__)
        static constexpr size_t CONTAINER_CAPACITY_ = 64;
#else
        static constexpr size_t CONTAINER_CAPACITY_ = 65;
#endif
        std::mutex syncLock_;
        BaseObject** top_;
        TreapNode* next_;
        BaseObject* container_[CONTAINER_CAPACITY_] = { nullptr };
    };

    static constexpr size_t NODE_SIZE_ = sizeof(TreapNode);
    struct Page {
        Page(Page* n, size_t bytes) : next_(n), length_(bytes) {}
        Page* next_ = nullptr;
        size_t length_ = 0;
    };

    // there is no need to use LL/SC to avoid ABA problem, becasue Nodes are all unique.
    template<typename T>
    class LockFreeList {
        friend class SatbBuffer;

    public:
        LockFreeList() : head_(nullptr) {}
        ~LockFreeList() = default;

        void Reset() { head_ = nullptr; }

        void Push(T* n)
        {
            T* old = head_.load(std::memory_order_relaxed);
            do {
                n->next = old;
            } while (!head_.compare_exchange_weak(old, n, std::memory_order_release, std::memory_order_relaxed));
        }

        T* Pop()
        {
            T* old = head_.load(std::memory_order_relaxed);
            do {
                if (old == nullptr) {
                    return nullptr;
                }
            } while (!head_.compare_exchange_weak(old, old->next, std::memory_order_release,
                                                  std::memory_order_relaxed));
            old->next = nullptr;
            return old;
        }

        T* PopAll()
        {
            T* old = head_.load(std::memory_order_relaxed);
            while (!head_.compare_exchange_weak(old, nullptr, std::memory_order_release, std::memory_order_relaxed)) {
            };
            return old;
        }

    private:
        std::atomic<T*> head_;
    };

    template<typename T>
    class LockedList {
        friend class SatbBuffer;

    public:
        LockedList() : head_(nullptr) {}
        ~LockedList() = default;

        void Reset()
        {
            std::lock_guard<std::mutex> lg(safeLock_);
            head_ = nullptr;
        }

        void Push(T* n)
        {
            std::lock_guard<std::mutex> lg(safeLock_);
            n->next_ = head_;
            head_ = n;
        }

        T* Pop()
        {
            std::lock_guard<std::mutex> lg(safeLock_);
            if (head_ == nullptr) {
                return nullptr;
            }
            T* old = head_;
            T* next = head_->next_;
            head_ = next;
            return old;
        }

        T* PopAll()
        {
            std::lock_guard<std::mutex> lg(safeLock_);
            T* old = head_;
            head_ = nullptr;
            return old;
        }

    private:
        std::mutex safeLock_;
        T* head_;
    };

    void EnsureGoodNode(TreapNode*& node)
    {
        if (node == nullptr) {
            node = freeNodes_.Pop();
        } else if (node->IsFull()) {
            // means current node is full
            retiredNodes_.Push(node);
            node = freeNodes_.Pop();
        } else {
            // not null & have slots
            return;
        }
        if (node == nullptr) {
            // there is no free nodes in the freeNodes list
            Page* page = GetPages(COMMON_PAGE_SIZE);
            TreapNode* list = ConstructFreeNodeList(page, COMMON_PAGE_SIZE);
            if (list == nullptr) {
                return;
            }
            node = list;
            TreapNode* cur = list->next_;
            node->next_ = nullptr;
            LOGF_CHECK(node->IsEmpty()) << "Get an unempty node from new page";
            while (cur != nullptr) {
                TreapNode* next = cur->next_;
                freeNodes_.Push(cur);
                cur = next;
            }
        } else {
            LOGF_CHECK(node->IsEmpty()) << "get an unempty node from free nodes";
        }
    }

    // must not have thread racing
    void Init()
    {
        TreapNode* head = retiredNodes_.PopAll();
        while (head != nullptr) {
            TreapNode* oldHead = head;
            head = head->next_;
            oldHead->Clear();
            freeNodes_.Push(oldHead);
        }

        if (freeNodes_.head_ == nullptr) {
            size_t initalBytes = INITIAL_PAGES * COMMON_PAGE_SIZE;
            Page* page = GetPages(initalBytes);
            TreapNode* list = ConstructFreeNodeList(page, initalBytes);
            freeNodes_.head_ = list;
        }
    }

    void RetireNode(TreapNode* node) noexcept { retiredNodes_.Push(node); }

    void Fini() { ReclaimALLPages(); }

    template<typename T>
    void GetRetiredObjects(T& stack)
    {
        TreapNode* head = retiredNodes_.PopAll();
        while (head != nullptr) {
            head->GetObjects(stack);
            TreapNode* oldHead = head;
            head = head->next_;
            oldHead->Clear();
            freeNodes_.Push(oldHead);
        }
    }

    template<typename T>
    void TryFetchOneRetiredNode(T& stack)
    {
        TreapNode* node = retiredNodes_.Pop();
        if (!node) {
            return;
        }
        node->GetObjects(stack);
        node->Clear();
        freeNodes_.Push(node);
    }

    void ClearBuffer()
    {
        TreapNode* head = retiredNodes_.PopAll();
        while (head != nullptr) {
            TreapNode* oldHead = head;
            head = head->next_;
            oldHead->Clear();
            freeNodes_.Push(oldHead);
        }
    }

    // it can be invoked only if no mutator points to any node.
    void ReclaimALLPages()
    {
        freeNodes_.Reset();
        retiredNodes_.Reset();
        Page* list = arena_.PopAll();
        if (list == nullptr) {
            return;
        }
        while (list != nullptr) {
            Page* next = list->next_;
            PagePool::Instance().ReturnPage(reinterpret_cast<uint8_t*>(list), list->length_);
            list = next;
        }
    }

private:
    Page* GetPages(size_t bytes)
    {
        Page* page = new (PagePool::Instance().GetPage(bytes)) Page(nullptr, bytes);
        page->next_ = nullptr;
        arena_.Push(page);
        return page;
    }

    TreapNode* ConstructFreeNodeList(const Page* page, size_t bytes) const
    {
        HeapAddress start = reinterpret_cast<HeapAddress>(page) + RoundUp(sizeof(Page), CACHE_LINE_ALIGN);
        HeapAddress end = reinterpret_cast<HeapAddress>(page) + bytes;
        TreapNode* cur = nullptr;
        TreapNode* head = nullptr;
        while (start <= (end - NODE_SIZE_)) {
            TreapNode* node = new (reinterpret_cast<void*>(start)) TreapNode();
            if (cur == nullptr) {
                cur = node;
                head = node;
            } else {
                cur->next_ = node;
                cur = node;
            }
            start += NODE_SIZE_;
        }
        return head;
    }

    LockedList<Page> arena_;        // arena of allocatable area, first area is 64 * 4k = 256k, the rest is 4k
    // free nodes, mutator will acquire nodes from this list to record old value writes
    LockedList<TreapNode> freeNodes_;
    LockedList<TreapNode> retiredNodes_; // has been filled by mutator, ready for scan
};
} // namespace common

#endif // COMMON_COMPONENTS_MUTATOR_SATB_BUFFER_H
