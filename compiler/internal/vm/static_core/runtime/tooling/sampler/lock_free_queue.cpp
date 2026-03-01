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

#include "lock_free_queue.h"

namespace ark::tooling::sampler {

void LockFreeQueue::Push(const FileInfo &data)
{
    Node *newNode = new Node(std::make_unique<FileInfo>(data), nullptr);

    for (;;) {
        // Atomic with acquire order reason: to sync with push in other threads
        Node *tail = tail_.load(std::memory_order_acquire);
        ASSERT(tail != nullptr);
        // Atomic with acquire order reason: to sync with push in other threads
        Node *next = tail->next.load(std::memory_order_acquire);
        // Atomic with acquire order reason: to sync with push in other threads
        Node *tail2 = tail_.load(std::memory_order_acquire);
        if (tail != tail2) {
            continue;
        }
        if (next == nullptr) {
            if (tail->next.compare_exchange_weak(next, newNode)) {
                tail_.compare_exchange_strong(tail, newNode);
                ++size_;
                return;
            }
        } else {
            Node *newTail = next;
            tail_.compare_exchange_strong(tail, newTail);
        }
    }
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
void LockFreeQueue::Pop(FileInfo &ret)
{
    for (;;) {
        // Atomic with acquire order reason: to sync with push in other threads
        Node *head = head_.load(std::memory_order_acquire);
        // Atomic with acquire order reason: to sync with push in other threads
        Node *tail = tail_.load(std::memory_order_acquire);
        // Atomic with acquire order reason: to sync with push in other threads
        Node *next = head->next.load(std::memory_order_acquire);
        // Atomic with acquire order reason: to sync with push in other threads
        Node *head2 = head_.load(std::memory_order_acquire);
        if (head != head2) {
            continue;
        }
        if (head == tail) {
            ASSERT(next != nullptr);
            Node *newTail = next;
            tail_.compare_exchange_strong(tail, newTail);
        } else {
            if (next == nullptr) {
                continue;
            }
            ASSERT(next->data != nullptr);
            ret = *(next->data);
            Node *newHead = next;
            if (head_.compare_exchange_weak(head, newHead)) {
                delete head;
                --size_;
                return;
            }
        }
    }
}

bool LockFreeQueue::FindValue(uintptr_t data) const
{
    // Atomic with acquire order reason: to sync with push in other threads
    Node *head = head_.load(std::memory_order_acquire);
    // Atomic with acquire order reason: to sync with push in other threads
    Node *tail = tail_.load(std::memory_order_acquire);

    for (;;) {
        ASSERT(head != nullptr);
        // Should be only dummy Node, but keep that check
        if (head->data == nullptr) {
            if (head == tail) {
                // here all nodes were checked
                break;
            }
            head = head->next;
            continue;
        }

        if (head->data->ptr == data) {
            return true;
        }

        if (head == tail) {
            // here all nodes were checked
            break;
        }
        head = head->next;
    }

    return false;
}

}  // namespace ark::tooling::sampler
