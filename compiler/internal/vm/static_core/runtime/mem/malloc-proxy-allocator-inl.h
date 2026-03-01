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
#ifndef PANDA_RUNTIME_MEM_MALLOC_PROXY_ALLOCATOR_INL_H
#define PANDA_RUNTIME_MEM_MALLOC_PROXY_ALLOCATOR_INL_H

#include <cstring>

#include "libarkbase/utils/logger.h"
#include "libarkbase/os/mem.h"
#include "runtime/mem/alloc_config.h"
#include "libarkbase/utils/asan_interface.h"
#include "runtime/mem/malloc-proxy-allocator.h"

namespace ark::mem {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_MALLOCPROXY_ALLOCATOR(level) LOG(level, ALLOC) << "MallocProxyAllocator: "

template <typename AllocConfigT>
MallocProxyAllocator<AllocConfigT>::~MallocProxyAllocator()
{
    LOG_MALLOCPROXY_ALLOCATOR(INFO) << "Destroying MallocProxyAllocator";
}

template <typename AllocConfigT>
void *MallocProxyAllocator<AllocConfigT>::Alloc(size_t size, Alignment align)
{
    if (size == 0) {
        return nullptr;
    }

    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (!DUMMY_ALLOC_CONFIG) {
        lock_.Lock();
    }
    size_t alignmentInBytes = GetAlignmentInBytes(align);
    void *ret = os::mem::AlignedAlloc(alignmentInBytes, size);
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (!DUMMY_ALLOC_CONFIG) {
        ASSERT(allocatedMemory_.find(ret) == allocatedMemory_.end());
        allocatedMemory_.insert({ret, size});
        AllocConfigT::OnAlloc(size, typeAllocation_, memStats_);
        AllocConfigT::MemoryInit(ret);
        lock_.Unlock();
    }
    LOG_MALLOCPROXY_ALLOCATOR(DEBUG) << "Allocate memory with size " << std::dec << size << " at addr " << std::hex
                                     << ret;
    return ret;
}

template <typename AllocConfigT>
void MallocProxyAllocator<AllocConfigT>::Free(void *mem)
{
    if (mem == nullptr) {
        return;
    }
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (!DUMMY_ALLOC_CONFIG) {
        lock_.Lock();
    }
    os::mem::AlignedFree(mem);
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (!DUMMY_ALLOC_CONFIG) {
        auto iterator = allocatedMemory_.find(mem);
        ASSERT(iterator != allocatedMemory_.end());
        size_t size = iterator->second;
        allocatedMemory_.erase(mem);
        AllocConfigT::OnFree(size, typeAllocation_, memStats_);
        lock_.Unlock();
    }
    LOG_MALLOCPROXY_ALLOCATOR(DEBUG) << "Free memory at " << mem;
}

#undef LOG_MALLOCPROXY_ALLOCATOR

}  // namespace ark::mem
#endif  // PANDA_RUNTIME_MEM_MALLOC_PROXY_ALLOCATOR_INL_H
