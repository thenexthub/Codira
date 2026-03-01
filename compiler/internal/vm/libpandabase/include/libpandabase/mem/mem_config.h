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

#ifndef LIBPANDABASE_MEM_MEM_CONFIG_H
#define LIBPANDABASE_MEM_MEM_CONFIG_H

#include <cstddef>

#include "macros.h"

namespace panda::mem {

/**
 * class for global memory parameters
 */
class MemConfig {
public:
    static void Initialize(size_t object_pool_size, size_t internal_size, size_t compiler_size, size_t code_size,
                           size_t initial_object_pool_size)
    {
        ASSERT(!is_initialized);
        initial_heap_size_limit = initial_object_pool_size;
        heap_size_limit = object_pool_size;
        internal_memory_size_limit = internal_size;
        compiler_memory_size_limit = compiler_size;
        code_cache_size_limit = code_size;
        is_initialized = true;
    }

    static void Initialize(size_t object_pool_size, size_t internal_size, size_t compiler_size, size_t code_size)
    {
        Initialize(object_pool_size, internal_size, compiler_size, code_size, object_pool_size);
    }

    static void Finalize()
    {
        is_initialized = false;
        heap_size_limit = 0;
        internal_memory_size_limit = 0;
        code_cache_size_limit = 0;
    }

    static size_t GetInitialHeapSizeLimit()
    {
        ASSERT(is_initialized);
        return initial_heap_size_limit;
    }

    static size_t GetHeapSizeLimit()
    {
        ASSERT(is_initialized);
        return heap_size_limit;
    }

    static size_t GetInternalMemorySizeLimit()
    {
        ASSERT(is_initialized);
        return internal_memory_size_limit;
    }

    static size_t GetCodeCacheSizeLimit()
    {
        ASSERT(is_initialized);
        return code_cache_size_limit;
    }

    static size_t GetCompilerMemorySizeLimit()
    {
        ASSERT(is_initialized);
        return compiler_memory_size_limit;
    }

    MemConfig() = delete;

    ~MemConfig() = delete;

    NO_COPY_SEMANTIC(MemConfig);
    NO_MOVE_SEMANTIC(MemConfig);

private:
    static bool is_initialized;
    static size_t initial_heap_size_limit;     // Initial heap size
    static size_t heap_size_limit;             // Max heap size
    static size_t internal_memory_size_limit;  // Max internal memory used by the VM
    static size_t code_cache_size_limit;       // The limit for compiled code size.
    static size_t compiler_memory_size_limit;  // Max memory used by compiler
};

}  // namespace panda::mem

#endif  // LIBPANDABASE_MEM_MEM_CONFIG_H
