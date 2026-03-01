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

#ifndef COMMON_COMPONENTS_COMMON_WORK_STACK_INL_H
#define COMMON_COMPONENTS_COMMON_WORK_STACK_INL_H

#include "common_components/common/work_stack.h"

#include <errno.h>

namespace common {

template <typename T, size_t capacity>
bool StackBase<T, capacity>::IsEmpty() const
{
    DCHECK_CC(top_ <= capacity);
    return top_ == 0;
}

template <typename T, size_t capacity>
bool StackBase<T, capacity>::IsFull() const
{
    DCHECK_CC(top_ <= capacity);
    return top_ == capacity;
}

template <typename T, size_t capacity>
void StackBase<T, capacity>::Push(T *e)
{
    DCHECK_CC(top_ <= capacity);
    DCHECK_CC(!IsFull());
    DCHECK_CC(e != nullptr);
    data_[top_++] = e;
}

template <typename T, size_t capacity>
void StackBase<T, capacity>::Pop(T **e)
{
    DCHECK_CC(top_ <= capacity);
    DCHECK_CC(!IsEmpty());
    T *result = data_[--top_];
    DCHECK_CC(result != nullptr);
    *e = result;
}

template <typename T, size_t capacity>
StackBase<T, capacity> *StackBase<T, capacity>::GetNext() const
{
    return next_;
}

template <typename T, size_t capacity>
void StackBase<T, capacity>::SetNext(StackBase *next)
{
    next_ = next;
}

template <typename T, size_t capacity>
void StackList<T, capacity>::Push(InternalStack *stack)
{
    DCHECK_CC(stack != nullptr);
    DCHECK_CC(!stack->IsEmpty());
    std::lock_guard<std::mutex> guard(mutex_);
    stack->SetNext(head_);
    head_ = stack;
}

template <typename T, size_t capacity>
void StackList<T, capacity>::Pop(InternalStack **stack)
{
    std::lock_guard<std::mutex> guard(mutex_);
    *stack = head_;
    if (head_ != nullptr) {
        head_ = head_->GetNext();
    }
}

template <typename T, size_t capacity>
size_t StackList<T, capacity>::Count()
{
    size_t cnt = 0;
    std::lock_guard<std::mutex> guard(mutex_);
    InternalStack *current = head_;
    while (current != nullptr) {
        ++cnt;
        current = current->GetNext();
    }
    return cnt;
}

namespace __work_stack_internal_impl {

template <typename T, size_t capacity, typename PushToGlobalNotify>
void LocalStackImpl<T, capacity, PushToGlobalNotify>::Push(T *e)
{
    DCHECK_CC(e != nullptr);
    if (UNLIKELY_CC(inStack_->IsFull())) {
        PushInStackToGlobal();
    }
    DCHECK_CC(!inStack_->IsFull());
    inStack_->Push(e);
}

template <typename T, size_t capacity, typename PushToGlobalNotify>
bool LocalStackImpl<T, capacity, PushToGlobalNotify>::Pop(T **e)
{
    if (UNLIKELY_CC(outStack_->IsEmpty())) {
        if (UNLIKELY_CC(!inStack_->IsEmpty())) {
            std::swap(inStack_, outStack_);
        } else if (!PopOutStackFromGlobal()) {
            return false;
        }
    }
    DCHECK_CC(!outStack_->IsEmpty());
    outStack_->Pop(e);
    return true;
}

template <typename T, size_t capacity, typename PushToGlobalNotify>
bool LocalStackImpl<T, capacity, PushToGlobalNotify>::IsEmpty() const
{
    return inStack_->IsEmpty() && outStack_->IsEmpty();
}

template <typename T, size_t capacity, typename PushToGlobalNotify>
void LocalStackImpl<T, capacity, PushToGlobalNotify>::Publish()
{
    if (!inStack_->IsEmpty()) {
        PushInStackToGlobal();
    }
    std::swap(inStack_, outStack_);
    if (!inStack_->IsEmpty()) {
        PushInStackToGlobal();
    }
}

template <typename T, size_t capacity, typename PushToGlobalNotify>
void LocalStackImpl<T, capacity, PushToGlobalNotify>::PushInStackToGlobal()
{
    DCHECK_CC(!inStack_->IsEmpty());
    globalStack_->Push(inStack_);
    if constexpr (HAS_PUSH_TO_GLOBAL_NOTIFY) {
        this->NotifyPushToGlobal();
    }
    inStack_ = new InternalStack();
}

template <typename T, size_t capacity, typename PushToGlobalNotify>
bool LocalStackImpl<T, capacity, PushToGlobalNotify>::PopOutStackFromGlobal()
{
    DCHECK_CC(outStack_->IsEmpty());
    InternalStack *newStack = nullptr;
    globalStack_->Pop(&newStack);
    if (LIKELY_CC(newStack != nullptr)) {
        delete outStack_;
        outStack_ = newStack;
        return true;
    }
    return false;
}
}  // namespace __work_stack_internal_impl
}  // namespace common
#endif  // COMMON_COMPONENTS_COMMON_WORK_STACK_INL_H
