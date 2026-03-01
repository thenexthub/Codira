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

#ifndef COMMON_COMPONENTS_COMMON_NEW_MARK_STACK_H
#define COMMON_COMPONENTS_COMMON_NEW_MARK_STACK_H

#include <mutex>
#include "common_components/base/time_utils.h"

namespace common {
template<class T>
class MarkStackBuffer {
constexpr static size_t MAX = 64;

public:
    MarkStackBuffer<T>* next = nullptr;
    MarkStackBuffer<T>* pre = nullptr;
    bool full() { return count == MAX; }

    bool empty() { return count == 0; }

    void push_back(T t)
    {
        ASSERT_LOGF(!full(), "Mark stack buffer can not be full when push back");
        stack_[count] = t;
        count++;
    }

    T back()
    {
        ASSERT_LOGF(!empty(), "Mark stack buffer can not be empty when get back");
        return stack_[count - 1];
    }

    void pop_back()
    {
        ASSERT_LOGF(!empty(), "Mark stack buffer can not be empty when pop back");
        count--;
    }
    size_t count = 0;

private:
    T stack_[MAX];
};

template<class T>
class MarkStack {
public:
    MarkStack() = default;

    MarkStack(MarkStack&& stack)
    {
        this->h_ = stack.head();
        this->t_ = stack.tail();
        this->s_ = stack.size();
        stack.clean();
    }

    MarkStack(MarkStackBuffer<T>* buf) : h_(buf)
    {
        MarkStackBuffer<T>* tmp = buf;
        while (tmp != nullptr) {
            if (tmp->next == nullptr) {
                this->t_ = tmp;
            }
            this->s_++;
            tmp = tmp->next;
        }
    }

    ~MarkStack() { clear(); }

    MarkStackBuffer<T>* head() { return this->h_; }

    MarkStackBuffer<T>* tail() { return this->t_; }

    size_t size() { return this->s_; }

    size_t count()
    {
        MarkStackBuffer<T> *current = this->h_;
        size_t ret = 0;
        while (current!=nullptr) {
            ret+=current->count;
            current = current->next;
        }

        return ret;
    }

    bool empty()
    {
        return this->s_ == 0 && this->h_ == nullptr && this->t_ == nullptr;
    }

    void clear()
    {
        while (!empty()) {
            MarkStackBuffer<T>* tmp = this->h_;
            this->h_ = this->h_->next;
            if (this->h_ != nullptr) {
                this->h_->pre = nullptr;
            }
            delete tmp;
            tmp = nullptr;
            this->s_--;
        }
        this->t_ = nullptr;
    }

    void push_back(T value)
    {
        if (this->t_ == nullptr || this->t_->full()) {
            push(new MarkStackBuffer<T>());
        }
        this->t_->push_back(value);
    }

    T back()
    {
        if (this->t_ == nullptr) {
            return nullptr;
        }
        return this->t_->back();
    }

    void pop_back()
    {
        ASSERT_LOGF(!empty(), "Mark stack can not be empty when pop back");
        this->t_->pop_back();
        if (this->t_->empty()) {
            auto tmp = pop();
            delete tmp;
            tmp = nullptr;
        }
    }

    void insert(MarkStack<T>& stack)
    {
        if (stack.empty()) {
            return;
        }
        if (this->h_ == nullptr) {
            this->h_ = stack.head();
            this->t_ = stack.tail();
            this->s_ = stack.size();
            stack.clean();
            return;
        }
        this->t_->next = stack.head();
        stack.head()->pre = this->t_;
        this->t_ = stack.tail();
        this->s_ += stack.size();
        stack.clean();
    }

    MarkStackBuffer<T>* split(size_t splitNum)
    {
        if (splitNum == 0 || s_ == 0) {
            return nullptr;
        }
        auto res = this->h_;
        size_t num = 0;
        while (num < splitNum) {
            if (num == splitNum - 1) {
                auto tmp = this->h_;
                this->h_ = this->h_->next;
                if (this->h_ != nullptr) {
                    this->h_->pre = nullptr;
                }
                tmp->next = nullptr;
            } else {
                this->h_ = this->h_->next;
            }
            num++;
            this->s_--;
            if (this->h_ == nullptr) {
                this->t_ = nullptr;
                return res;
            }
        }
        return res;
    }
private:
    void push(MarkStackBuffer<T>* buf)
    {
        if (empty()) {
            this->h_ = buf;
            this->t_ = buf;
            this->s_++;
            return;
        }
        buf->pre = this->t_;
        this->t_->next = buf;
        this->t_ = buf;
        this->s_++;
    }

    MarkStackBuffer<T>* pop()
    {
        if (empty()) {
            return nullptr;
        }
        MarkStackBuffer<T>* res = this->t_;
        this->t_ = this->t_->pre;
        if (this->t_ != nullptr) {
            this->t_->next = nullptr;
        } else {
            this->h_ = nullptr;
        }
        res->pre = nullptr;
        this->s_--;
        return res;
    }

    void clean()
    {
        if (empty()) {
            return;
        }
        this->h_ = nullptr;
        this->t_ = nullptr;
        this->s_ = 0;
    }

    MarkStackBuffer<T>* h_ = nullptr;
    MarkStackBuffer<T>* t_ = nullptr;
    size_t s_ = 0;
};
}
#endif // COMMON_COMPONENTS_COMMON_NEW_MARK_STACK_H
