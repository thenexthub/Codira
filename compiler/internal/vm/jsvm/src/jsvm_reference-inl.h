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

#ifndef JSVM_REFERENCE_INL_H
#define JSVM_REFERENCE_INL_H
#include "jsvm_env.h"
#include "jsvm_reference.h"

namespace v8impl {
inline bool CanBeHeldWeakly(v8::Local<v8::Value> value)
{
    return value->IsObject() || value->IsSymbol();
}

// RefTracker
inline void RefTracker::Link(RefList* list)
{
    DCHECK(list != nullptr);
    prev = list;
    next = list->next;
    if (next != nullptr) {
        next->prev = this;
    }
    list->next = this;
}

inline void RefTracker::Unlink()
{
    if (prev != nullptr) {
        prev->next = next;
    }
    if (next != nullptr) {
        next->prev = prev;
    }
    prev = nullptr;
    next = nullptr;
}

// UserReference
inline void UserReference::SetWeak()
{
    if (canBeWeak) {
        persistent.SetWeak();
    } else {
        persistent.Reset();
    }
}

inline uint32_t UserReference::Ref()
{
    // If persistent is cleared by GC, return 0 unconditionally.
    if (persistent.IsEmpty()) {
        return 0;
    }

    if (++refcount == 1) {
        // If persistent can not be weak, it will be cleared in SetWeak().
        DCHECK(canBeWeak);
        persistent.ClearWeak();
    }

    return refcount;
}

inline uint32_t UserReference::Unref()
{
    // If persistent is cleared by GC, return 0 unconditionally.
    if (persistent.IsEmpty() || refcount == 0) {
        return 0;
    }

    if (--refcount == 0) {
        SetWeak();
    }

    return refcount;
}

inline v8::Local<v8::Value> UserReference::Get()
{
    DCHECK(isValue);
    if (persistent.IsEmpty()) {
        return v8::Local<v8::Value>();
    } else {
        return v8::Local<v8::Data>::New(env->isolate, persistent).As<v8::Value>();
    }
}

inline v8::Local<v8::Data> UserReference::GetData()
{
    if (persistent.IsEmpty()) {
        return v8::Local<v8::Data>();
    } else {
        return v8::Local<v8::Data>::New(env->isolate, persistent);
    }
}

} // namespace v8impl

#endif