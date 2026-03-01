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
#ifndef PANDA_LIBPANDABASE_OS_UNIX_MEM_HOOKS_H_
#define PANDA_LIBPANDABASE_OS_UNIX_MEM_HOOKS_H_

#include "libarkbase/mem/mem.h"
#include "libarkbase/macros.h"

namespace ark::os::unix::mem_hooks {

class PandaHooks {
public:
    PANDA_PUBLIC_API static void Initialize();
    PANDA_PUBLIC_API static void Enable();

    PANDA_PUBLIC_API static void Disable();

    static size_t GetAllocViaStandard() noexcept
    {
        return allocViaStandard_;
    }

    static bool IsActive() noexcept
    {
        return isActive_;
    }

    static void *MallocHook(size_t size, const void *caller);
    static void *MemalignHook(size_t alignment, size_t size, const void *caller);
    static void FreeHook(void *ptr, const void *caller);

    static void SaveRealFunctions();
    static void *(*realMalloc_)(size_t);
    static void *(*realMemalign_)(size_t, size_t);
    static void (*realFree_)(void *);

    class AddrRange {
    public:
        AddrRange() = default;
        ~AddrRange() = default;
        DEFAULT_COPY_SEMANTIC(AddrRange);
        DEFAULT_MOVE_SEMANTIC(AddrRange);

        AddrRange(uintptr_t base, size_t size) : base_(base), size_(size) {}

        bool Contains(uintptr_t addr) const
        {
            return base_ <= addr && addr < base_ + size_;
        }

    private:
        uintptr_t base_ {0};
        size_t size_ {0};
    };

private:
    static constexpr size_t MAX_ALLOC_VIA_STANDARD = 4_MB;

    static bool ShouldCountAllocation(const void *caller);

    static size_t allocViaStandard_;
    static AddrRange ignoreCodeRange_;
    static bool isActive_;
};

}  // namespace ark::os::unix::mem_hooks

namespace ark::os::mem_hooks {
using PandaHooks = ark::os::unix::mem_hooks::PandaHooks;
}  // namespace ark::os::mem_hooks

// Don't use for musl and mobile
// Sanitizers hook malloc functions, so we don't use memory hooks
#if !defined(__MUSL__) && !defined(PANDA_TARGET_ARM64) && !defined(PANDA_TARGET_ARM32) && \
    !defined(USE_ADDRESS_SANITIZER) && !defined(USE_THREAD_SANITIZER)
#define PANDA_USE_MEMORY_HOOKS
#endif

#ifdef PANDA_USE_MEMORY_HOOKS

void *malloc(size_t size) noexcept;                      // NOLINT(readability-redundant-declaration)
void *Memalign(size_t alignment, size_t size) noexcept;  // NOLINT(readability-redundant-declaration)
void free(void *ptr) noexcept;                           // NOLINT(readability-redundant-declaration)

#endif  // PANDA_USE_MEMORY_HOOKS

#endif  // PANDA_LIBPANDABASE_OS_UNIX_MEM_HOOKS_H_
