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

#ifndef COMMON_COMPONENTS_HEAP_ALLOCATOR_ALLOC_MEM_MAP_H
#define COMMON_COMPONENTS_HEAP_ALLOCATOR_ALLOC_MEM_MAP_H

#include "common_interfaces/base/common.h"
#ifdef _WIN64
#include <handleapi.h>
#include <memoryapi.h>
#else
#include <sys/mman.h>
#endif
#include "common_components/heap/allocator/alloc_util.h"
#include "common_components/common/type_def.h"

namespace common {
class MemoryMap {
public:
#ifdef _WIN64
    static constexpr int MAP_PRIVATE = 2;
    static constexpr int MAP_FIXED = 0x10;
    static constexpr int MAP_ANONYMOUS = 0x20;
    static constexpr int PROT_NONE = 0;
    static constexpr int PROT_READ = 1;
    static constexpr int PROT_WRITE = 2;
    static constexpr int PROT_EXEC = 4;
    static constexpr int DEFAULT_MEM_FLAGS = MAP_PRIVATE | MAP_ANONYMOUS;
#else
    static constexpr int DEFAULT_MEM_FLAGS = MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE;
#endif
    static constexpr int DEFAULT_MEM_PROT = PROT_READ | PROT_WRITE;

    struct Option {         // optional args for mem map
        const char* tag;    // name to identify the mapped memory
        void* reqBase;      // a hint to mmap about start addr, not guaranteed
        unsigned int flags; // mmap flags
        int prot;           // initial access flags
        bool protAll;       // applying prot to all pages in range
    };
    // by default, it tries to map memory in low addr space, with a random start
    static constexpr Option DEFAULT_OPTIONS = { "arkcommon_unnamed", nullptr, DEFAULT_MEM_FLAGS,
                                                DEFAULT_MEM_PROT, false };

    // the only way to get a MemoryMap
    static MemoryMap* MapMemory(size_t reqSize, size_t initSize, const Option& opt = DEFAULT_OPTIONS);

    static MemoryMap* MapMemoryAlignInner4G(uint64_t reqSize, uint64_t initSize, const Option& opt = DEFAULT_OPTIONS);

#ifdef _WIN64
    static void CommitMemory(void* addr, size_t size);
#endif

    // destroy a MemoryMap
    static void DestroyMemoryMap(MemoryMap*& memMap) noexcept
    {
        if (memMap != nullptr) {
            delete memMap;
            memMap = nullptr;
        }
    }

    void* GetBaseAddr() const { return memBaseAddr_; }
    void* GetCurrEnd() const { return memCurrEndAddr_; }
    void* GetMappedEndAddr() const { return memMappedEndAddr_; }
    size_t GetCurrSize() const { return memCurrSize_; }
    size_t GetMappedSize() const { return memMappedSize_; }

    ~MemoryMap();
    MemoryMap(const MemoryMap& that) = delete;
    MemoryMap(MemoryMap&& that) = delete;
    MemoryMap& operator=(const MemoryMap& that) = delete;
    MemoryMap& operator=(MemoryMap&& that) = delete;

private:
    static bool ProtectMemInternal(void* addr, size_t size, int prot);

    void* memBaseAddr_;      // start of the mapped memory
    void* memCurrEndAddr_;   // end of the memory **in use**
    void* memMappedEndAddr_; // end of the mapped memory, always >= currEndAddr
    size_t memCurrSize_;     // size of the memory **in use**
    size_t memMappedSize_;   // size of the mapped memory, always >= currSize

    // MemoryMap is created via factory method
    MemoryMap(void* baseAddr, size_t initSize, size_t mappedSize);
}; // class MemoryMap
} // namespace common

#endif // COMMON_COMPONENTS_HEAP_ALLOCATOR_ALLOC_MEM_MAP_H
