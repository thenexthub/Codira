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

#ifndef PANDA_CODEALLOCATOR_H
#define PANDA_CODEALLOCATOR_H

#include "libarkbase/utils/arena_containers.h"
#include "libarkbase/os/mem.h"
#include "libarkbase/os/mutex.h"

namespace ark {

class BaseMemStats;

class CodeAllocator {
public:
    PANDA_PUBLIC_API explicit CodeAllocator(BaseMemStats *memStats);
    PANDA_PUBLIC_API ~CodeAllocator();
    NO_COPY_SEMANTIC(CodeAllocator);
    NO_MOVE_SEMANTIC(CodeAllocator);

    /**
     * @brief Allocates @param size bytes, copies @param codeBuff to allocated memory and make this memory executable
     * @param size
     * @param codeBuff
     * @return
     */
    [[nodiscard]] PANDA_PUBLIC_API void *AllocateCode(size_t size, const void *codeBuff);

    /**
     * @brief Allocates @param size bytes of non-protected memory
     * @param size to be allocated
     * @return MapRange of the allocated code
     */
    [[nodiscard]] PANDA_PUBLIC_API os::mem::MapRange<std::byte> AllocateCodeUnprotected(size_t size);

    /**
     * Make memory \mem_range executable
     * @param mem_range
     */
    PANDA_PUBLIC_API static void ProtectCode(os::mem::MapRange<std::byte> memRange);

    /// Fast check if the given program counter belongs to JIT code
    PANDA_PUBLIC_API bool InAllocatedCodeRange(const void *pc);

private:
    void CodeRangeUpdate(void *ptr, size_t size);

private:
    static const Alignment PAGE_LOG_ALIGN;

    // NOTE(dtrubenkov): Remove when some CodeCache space will be implemented, currently used for avoid memleak noise
    ArenaAllocator arenaAllocator_;
    BaseMemStats *memStats_;
    os::memory::RWLock codeRangeLock_;
    void *codeRangeStart_ {nullptr};
    void *codeRangeEnd_ {nullptr};
};

}  // namespace ark

#endif  // PANDA_CODEALLOCATOR_H
