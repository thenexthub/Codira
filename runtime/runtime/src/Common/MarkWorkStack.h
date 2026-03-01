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


#ifndef MRT_NEW_MARK_STACK_H
#define MRT_NEW_MARK_STACK_H

#include <mutex>
#include "Base/TimeUtils.h"

namespace MapleRuntime {
template<class T>
class MarkStackBuf {
constexpr static size_t MAX = 64;

public:
    MarkStackBuf<T>* next = nullptr;
    MarkStackBuf<T>* pre = nullptr;
    bool full() { return count == MAX; }

    bool empty() { return count == 0; }

    void push_back(T t)
    {
        CHECK_DETAIL(!full(), "Mark stack buffer can not be full when push back");
        stack[count] = t;
        count++;
    }

    T back()
    {
        CHECK_DETAIL(!empty(), "Mark stack buffer can not be empty when get back");
        return stack[count - 1];
    }

    void pop_back()
    {
        CHECK_DETAIL(!empty(), "Mark stack buffer can not be empty when pop back");
        count--;
    }
private:
    size_t count = 0;
    T stack[MAX];
};

template<class T>
class MarkStack {
public:
    MarkStack() = default;

    MarkStack(MarkStack&& stack)
    {
        this->h = stack.head();
        this->t = stack.tail();
        this->s = stack.size();
        stack.clean();
    }

    MarkStack(MarkStackBuf<T>* buf) : h(buf)
    {
        MarkStackBuf<T>* tmp = buf;
        while (tmp != nullptr) {
            if (tmp->next == nullptr) {
                this->t = tmp;
            }
            this->s++;
            tmp = tmp->next;
        }
    }

    ~MarkStack() { clear(); }

    MarkStackBuf<T>* head() { return this->h; }

    MarkStackBuf<T>* tail() { return this->t; }

    size_t size() { return this->s; }

    bool empty()
    {
        return this->s == 0 && this->h == nullptr && this->t == nullptr;
    }

    void clear()
    {
        while (!empty()) {
            MarkStackBuf<T>* tmp = this->h;
            this->h = this->h->next;
            if (this->h != nullptr) {
                this->h->pre = nullptr;
            }
            delete tmp;
            tmp = nullptr;
            this->s--;
        }
        this->t = nullptr;
    }

    void push_back(T value)
    {
        if (this->t == nullptr || this->t->full()) {
            push(new MarkStackBuf<T>());
        }
        this->t->push_back(value);
    }

    T back()
    {
        if (this->t == nullptr) {
            return nullptr;
        }
        return this->t->back();
    }

    void pop_back()
    {
        CHECK_DETAIL(!empty(), "Mark stack can not be empty when pop back");
        this->t->pop_back();
        if (this->t->empty()) {
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
        if (this->h == nullptr) {
            this->h = stack.head();
            this->t = stack.tail();
            this->s = stack.size();
            stack.clean();
            return;
        }
        this->t->next = stack.head();
        stack.head()->pre = this->t;
        this->t = stack.tail();
        this->s += stack.size();
        stack.clean();
    }

    MarkStackBuf<T>* split(size_t splitNum)
    {
        if (splitNum == 0 || s == 0) {
            return nullptr;
        }
        auto res = this->h;
        size_t num = 0;
        while (num < splitNum) {
            if (num == splitNum - 1) {
                auto tmp = this->h;
                this->h = this->h->next;
                if (this->h != nullptr) {
                    this->h->pre = nullptr;
                }
                tmp->next = nullptr;
            } else {
                this->h = this->h->next;
            }
            num++;
            this->s--;
            if (this->h == nullptr) {
                this->t = nullptr;
                return res;
            }
        }
        return res;
    }
private:
    void push(MarkStackBuf<T>* buf)
    {
        if (empty()) {
            this->h = buf;
            this->t = buf;
            this->s++;
            return;
        }
        buf->pre = this->t;
        this->t->next = buf;
        this->t = buf;
        this->s++;
    }

    MarkStackBuf<T>* pop()
    {
        if (empty()) {
            return nullptr;
        }
        MarkStackBuf<T>* res = this->t;
        this->t = this->t->pre;
        if (this->t != nullptr) {
            this->t->next = nullptr;
        } else {
            this->h = nullptr;
        }
        res->pre = nullptr;
        this->s--;
        return res;
    }

    void clean()
    {
        if (empty()) {
            return;
        }
        this->h = nullptr;
        this->t = nullptr;
        this->s = 0;
    }

    MarkStackBuf<T>* h = nullptr;
    MarkStackBuf<T>* t = nullptr;
    size_t s = 0;
};
}
#endif // MRT_NEW_MARK_STACK_H
