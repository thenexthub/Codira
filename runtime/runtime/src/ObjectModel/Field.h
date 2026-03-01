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


#ifndef MRT_FIELD_H
#define MRT_FIELD_H

#include <atomic>
#if defined(CODIRA_TSAN_SUPPORT)
#include "Sanitizer/SanitizerInterface.h"
#endif

namespace MapleRuntime {
class BaseObject;
using MemoryOrder = std::memory_order;
// T is primitive field
template<typename T, bool isAtomic = false>
class Field {
public:
    T GetFieldValue(std::memory_order order = std::memory_order_relaxed) const
    {
        if (isAtomic) {
            // compiling failure: std::atomic_load_explicit(&value, order)
            T ret;
    #if defined(CODIRA_TSAN_SUPPORT)
            ret = Sanitizer::TsanAtomicLoad(&value, order);
    #else
            __atomic_load(&value, &ret, order);
    #endif
            return ret;
        } else {
    #if defined(CODIRA_TSAN_SUPPORT)
            Sanitizer::TsanReadMemory(&value, sizeof(T));
    #endif
            return value;
        }
    }

    void SetFieldValue(const BaseObject* obj, T v, std::memory_order order = std::memory_order_relaxed);

    // the interfaces fellow are only used in atomic operation.
    bool CompareExchange(T expectedValue, T newValue, std::memory_order succOrder = std::memory_order_relaxed,
                         MemoryOrder failOrder = std::memory_order_relaxed)
    {
        static_assert(isAtomic, "this interface must be used in atomic operation");
    #if defined(CODIRA_TSAN_SUPPORT)
        // tsan will get expectedValue's address for us, just pass the real value
        auto ret = Sanitizer::TsanAtomicCompareExchange(&value, expectedValue, newValue, succOrder, failOrder);
        return (ret == expectedValue);
    #else
        // compiling failure:
        //     std::atomic_compare_exchange_strong_explicit(&value, &expectedValue, newValue, order, order)
        return __atomic_compare_exchange(&value, &expectedValue, &newValue, false, succOrder, failOrder);
    #endif
    }

    T Exchange(T newValue, std::memory_order order = std::memory_order_relaxed)
    {
        static_assert(isAtomic, "this interface must be used in atomic operation");
        T ret;
    #if defined(CODIRA_TSAN_SUPPORT)
        ret = Sanitizer::TsanAtomicExchange(&value, newValue, order);
    #else
        __atomic_exchange(&value, &newValue, &ret, order);
    #endif
        return ret;
    }

    T FetchAdd(T val, std::memory_order order = std::memory_order_relaxed)
    {
        static_assert(isAtomic, "this interface must be used in atomic operation");
    #if defined(CODIRA_TSAN_SUPPORT)
        return Sanitizer::TsanAtomicFetchAdd(&value, val, order);
    #else
        return __atomic_fetch_add(&value, val, order);
    #endif
    }

    T FetchSub(T val, std::memory_order order = std::memory_order_relaxed)
    {
        static_assert(isAtomic, "this interface must be used in atomic operation");
    #if defined(CODIRA_TSAN_SUPPORT)
        return Sanitizer::TsanAtomicFetchSub(&value, val, order);
    #else
        return __atomic_fetch_sub(&value, val, order);
    #endif
    }

    T FetchAnd(T val, std::memory_order order = std::memory_order_relaxed)
    {
        static_assert(isAtomic, "this interface must be used in atomic operation");
    #if defined(CODIRA_TSAN_SUPPORT)
        return Sanitizer::TsanAtomicFetchAnd(&value, val, order);
    #else
        return __atomic_fetch_and(&value, val, order);
    #endif
    }

    T FetchOr(T val, std::memory_order order = std::memory_order_relaxed)
    {
        static_assert(isAtomic, "this interface must be used in atomic operation");
    #if defined(CODIRA_TSAN_SUPPORT)
        return Sanitizer::TsanAtomicFetchOr(&value, val, order);
    #else
        return __atomic_fetch_or(&value, val, order);
    #endif
    }

    T FetchXor(T val, std::memory_order order = std::memory_order_relaxed)
    {
        static_assert(isAtomic, "this interface must be used in atomic operation");
    #if defined(CODIRA_TSAN_SUPPORT)
        return Sanitizer::TsanAtomicFetchXor(&value, val, order);
    #else
        return __atomic_fetch_xor(&value, val, order);
    #endif
    }

private:
    T value;
};
} // namespace MapleRuntime
#endif // MRT_FIELD_H
