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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_WAITERS_LIST_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_WAITERS_LIST_H

#include "libarkbase/macros.h"
#include "runtime/coroutines/coroutine_events.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_primitives.h"

#include <atomic>
#include <utility>

namespace ark::ets {

namespace test {
class EtsSyncPrimitivesTest;
}  // namespace test

/// @brief Managed intrusive MPSC list containing native nodes
class EtsWaitersList : public EtsObject {
public:
    class Node {
    public:
        explicit Node(CoroutineManager *coroManager) : event_(coroManager) {}

        GenericEvent &GetEvent()
        {
            return event_;
        }

    private:
        friend class EtsWaitersList;
        Node *next_ = nullptr;
        GenericEvent event_;
    };

    void PushBack(Node *node)
    {
        // Atomic with relaxed order reason: sync is not needed here because CAS has acq_rel order
        node->next_ = tail_.load(std::memory_order_relaxed);
        auto spinWait = SpinWait();
        while (!TryPushBack(node)) {
            spinWait();
        }
    }

    Node *PopFront()
    {
        Node *node = nullptr;
        auto spinWait = SpinWait();
        while (!TryPopFront(&node)) {
            spinWait();
        }
        return node;
    }

    static EtsWaitersList *FromCoreType(ObjectHeader *waitersList)
    {
        return reinterpret_cast<EtsWaitersList *>(waitersList);
    }

    static EtsWaitersList *Create(EtsCoroutine *coro);

private:
    class SpinWait {
    public:
        void operator()() {}
    };

    bool TryPushBack(Node *node)
    {
        // Atomic with acq_rel order reason: sync Push/Pop in other threads
        return tail_.compare_exchange_weak(node->next_, node, std::memory_order_acq_rel, std::memory_order_relaxed);
    }

    bool TryPopFront(Node **node)
    {
        if (head_ == nullptr && !RestockQueue()) {
            return false;
        }
        *node = std::exchange(head_, head_->next_);
        return true;
    }

    bool RestockQueue()
    {
        ASSERT(head_ == nullptr);
        // Atomic with acq_rel order reason: sync Push/Pop in other threads
        auto *tail = tail_.exchange(nullptr, std::memory_order_acq_rel);
        while (tail != nullptr) {
            auto *newHead = std::exchange(tail, tail->next_);
            newHead->next_ = std::exchange(head_, newHead);
        }
        return head_ != nullptr;
    }

    /// alignment is necessary for arm32 to ensure the same field layout as in the ETS class
    alignas(alignof(EtsLong)) Node *head_;
    alignas(alignof(EtsLong)) std::atomic<Node *> tail_;

    friend class test::EtsSyncPrimitivesTest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_WAITERS_LIST_H
