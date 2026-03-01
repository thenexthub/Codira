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

#ifndef PANDA_RUNTIME_ASSERT_GC_SCOPE_H
#define PANDA_RUNTIME_ASSERT_GC_SCOPE_H

#include "libarkbase/macros.h"

#include <atomic>

namespace ark {

#ifndef NDEBUG
constexpr bool IS_GC_ALLOW_CHECK = true;
#else
constexpr bool IS_GC_ALLOW_CHECK = false;
#endif

template <bool IS_DEBUG = IS_GC_ALLOW_CHECK>
class AssertGCScopeT {
public:
    static bool IsAllowed()
    {
        return true;
    }
};

template <>
class AssertGCScopeT<true> {
public:
    AssertGCScopeT()
    {
        // Atomic with relaxed order reason: data race with gc_flag with no synchronization or ordering constraints
        // imposed on other reads or writes
        AssertGCScopeT::gcFlag_.fetch_add(1, std::memory_order_relaxed);
    }

    ~AssertGCScopeT()
    {
        // Atomic with relaxed order reason: data race with gc_flag with no synchronization or ordering constraints
        // imposed on other reads or writes
        AssertGCScopeT::gcFlag_.fetch_sub(1, std::memory_order_relaxed);
    }

    static bool IsAllowed();

    NO_COPY_SEMANTIC(AssertGCScopeT);
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(AssertGCScopeT);

private:
    static std::atomic<int> gcFlag_;
};

using DisallowGarbageCollection = AssertGCScopeT<IS_GC_ALLOW_CHECK>;

#if !defined(NDEBUG)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DISALLOW_GARGABE_COLLECTION [[maybe_unused]] DisallowGarbageCollection no_gc
#else
#define DISALLOW_GARGABE_COLLECTION
#endif

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DCHECK_ALLOW_GARBAGE_COLLECTION \
    ASSERT_PRINT(AssertGCScopeT<IS_GC_ALLOW_CHECK>::IsAllowed(), "disallow execute garbage collection.")
}  // namespace ark

#endif  // PANDA_RUNTIME_ASSERT_GC_SCOPE_H
