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

#ifndef COMMON_COMPONENTS_COMMON_WORK_STACK_H
#define COMMON_COMPONENTS_COMMON_WORK_STACK_H

#include <mutex>
#include <type_traits>

#include "common_interfaces/base/common.h"
#include "common_components/log/log.h"

namespace common {
template <typename T, size_t capacity>
class StackBase {
public:
    StackBase() = default;
    ~StackBase()
    {
        DCHECK_CC(IsEmpty());
    }

    NO_COPY_SEMANTIC_CC(StackBase);
    NO_MOVE_SEMANTIC_CC(StackBase);

    bool IsEmpty() const;

    bool IsFull() const;

    void Push(T *e);

    void Pop(T **e);

    StackBase *GetNext() const;

    void SetNext(StackBase *next);

private:
    size_t top_ {0};
    StackBase *next_ {nullptr};
    T *data_[capacity];
};

namespace __work_stack_internal_impl {
template <typename T, size_t capacity, typename PushToGlobalNotify>
class LocalStackImpl;
}

template <typename T, size_t capacity>
class StackList {
public:
    using InternalStack = StackBase<T, capacity>;
    StackList() = default;
    ~StackList()
    {
        DCHECK_CC(head_ == nullptr);
    }

    NO_COPY_SEMANTIC_CC(StackList);
    NO_MOVE_SEMANTIC_CC(StackList);

    size_t Count();

private:
    void Push(InternalStack *stack);

    void Pop(InternalStack **stack);

    InternalStack *head_ {nullptr};
    std::mutex mutex_;

    template <typename, size_t, typename>
    friend class __work_stack_internal_impl::LocalStackImpl;
};

namespace __work_stack_internal_impl {

class LocalStackBaseWithoutNotify {
protected:
    LocalStackBaseWithoutNotify() = default;
    ~LocalStackBaseWithoutNotify() = default;

    NO_COPY_SEMANTIC_CC(LocalStackBaseWithoutNotify);
    NO_MOVE_SEMANTIC_CC(LocalStackBaseWithoutNotify);
};

template <typename PushToGlobalNotify>
class LocalStackBaseWithNotify {
protected:
    explicit LocalStackBaseWithNotify(PushToGlobalNotify *pushToGlobalNotify)
        : pushToGlobalNotify_(pushToGlobalNotify)
    {
        DCHECK_CC(pushToGlobalNotify_ != nullptr);
    }
    ~LocalStackBaseWithNotify()
    {
        pushToGlobalNotify_ = nullptr;
    }

    NO_COPY_SEMANTIC_CC(LocalStackBaseWithNotify);
    NO_MOVE_SEMANTIC_CC(LocalStackBaseWithNotify);

    void NotifyPushToGlobal()
    {
        DCHECK_CC(pushToGlobalNotify_ != nullptr);
        (*pushToGlobalNotify_)();
    }
private:
    PushToGlobalNotify *pushToGlobalNotify_ {nullptr};
};

struct DummyNoPushToGlobalNotify {};

template <typename T, size_t capacity, typename PushToGlobalNotify>
class LocalStackImpl final : public std::conditional_t<std::is_same_v<DummyNoPushToGlobalNotify, PushToGlobalNotify>,
                                                       LocalStackBaseWithoutNotify,
                                                       LocalStackBaseWithNotify<PushToGlobalNotify>> {
    using InternalStack = StackBase<T, capacity>;
    using GlobalStack = StackList<T, capacity>;
private:
    static constexpr bool HAS_PUSH_TO_GLOBAL_NOTIFY = !std::is_same_v<DummyNoPushToGlobalNotify, PushToGlobalNotify>;
public:
    template <typename U = PushToGlobalNotify,
              typename = std::enable_if<!std::is_same_v<DummyNoPushToGlobalNotify, U>>>
    LocalStackImpl(GlobalStack *globalStack, PushToGlobalNotify *pushToGlobalNotify)
        : LocalStackBaseWithNotify<PushToGlobalNotify>(pushToGlobalNotify), globalStack_(globalStack)
    {
        inStack_ = new InternalStack();
        outStack_ = new InternalStack();
    }

    template <typename U = PushToGlobalNotify,
              typename = std::enable_if<std::is_same_v<DummyNoPushToGlobalNotify, U>>>
    explicit LocalStackImpl(GlobalStack *globalStack) : globalStack_(globalStack)
    {
        inStack_ = new InternalStack();
        outStack_ = new InternalStack();
    }

    ~LocalStackImpl()
    {
        DCHECK_CC(IsEmpty());
        delete inStack_;
        delete outStack_;
        inStack_ = nullptr;
        outStack_ = nullptr;
    }

    NO_COPY_SEMANTIC_CC(LocalStackImpl);
    NO_MOVE_SEMANTIC_CC(LocalStackImpl);

    void Push(T *e);

    bool Pop(T **e);

    bool IsEmpty() const;

    void Publish();

private:
    void PushInStackToGlobal();

    bool PopOutStackFromGlobal();

    GlobalStack *globalStack_ {nullptr};
    InternalStack *inStack_ {nullptr};
    InternalStack *outStack_ {nullptr};
};
}  // namespace __work_stack_internal_impl

template <typename T, size_t capacity,
          typename PushToGlobalNotify = __work_stack_internal_impl::DummyNoPushToGlobalNotify>
using LocalStack = __work_stack_internal_impl::LocalStackImpl<T, capacity, PushToGlobalNotify>;
}  // namespace common
#endif  // COMMON_COMPONENTS_COMMON_WORK_STACK_H
