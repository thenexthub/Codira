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

#ifndef PANDA_RUNTIME_MEM_MEM_CONFIG_H
#define PANDA_RUNTIME_MEM_MEM_CONFIG_H

#include "libarkbase/macros.h"
#include "libarkbase/mem/mem.h"
#include "libarkbase/utils/asan_interface.h"

#include <cstddef>

namespace ark::mem {

/// class for global memory parameters
class MemConfig {
public:
    static void Initialize(size_t objectPoolSize, size_t internalSize, size_t compilerSize, size_t codeSize,
                           size_t framesSize, size_t stacksSize, size_t initialObjectPoolSize)
    {
        ASSERT(!isInitialized_);
        initialHeapSizeLimit_ = initialObjectPoolSize;
        heapSizeLimit_ = objectPoolSize;
        internalMemorySizeLimit_ = internalSize;
        compilerMemorySizeLimit_ = compilerSize;
        codeCacheSizeLimit_ = codeSize;
        framesMemorySizeLimit_ = framesSize;
        nativeStacksMemorySizeLimit_ = stacksSize;
        isInitialized_ = true;
    }

    static void Initialize(size_t objectPoolSize, size_t internalSize, size_t compilerSize, size_t codeSize,
                           size_t framesSize, size_t stacksSize)
    {
        Initialize(objectPoolSize, internalSize, compilerSize, codeSize, framesSize, stacksSize, objectPoolSize);
    }

    static void Finalize()
    {
        isInitialized_ = false;
        heapSizeLimit_ = 0;
        internalMemorySizeLimit_ = 0;
        codeCacheSizeLimit_ = 0;
    }

    static size_t GetInitialHeapSizeLimit()
    {
        ASSERT(isInitialized_);
        return initialHeapSizeLimit_;
    }

    static size_t GetHeapSizeLimit()
    {
        ASSERT(isInitialized_);
        return heapSizeLimit_;
    }

    static size_t GetInternalMemorySizeLimit()
    {
        ASSERT(isInitialized_);
        return internalMemorySizeLimit_;
    }

    static size_t GetCodeCacheSizeLimit()
    {
        ASSERT(isInitialized_);
        return codeCacheSizeLimit_;
    }

    static size_t GetCompilerMemorySizeLimit()
    {
        ASSERT(isInitialized_);
        return compilerMemorySizeLimit_;
    }

    static size_t GetFramesMemorySizeLimit()
    {
        ASSERT(isInitialized_);
        return framesMemorySizeLimit_;
    }

    static size_t GetNativeStacksMemorySizeLimit()
    {
        ASSERT(isInitialized_);
        return nativeStacksMemorySizeLimit_;
    }

    MemConfig() = delete;

    ~MemConfig() = delete;

    NO_COPY_SEMANTIC(MemConfig);
    NO_MOVE_SEMANTIC(MemConfig);

    static bool IsInitialized()
    {
        return isInitialized_;
    }

private:
    PANDA_PUBLIC_API static bool isInitialized_;
    PANDA_PUBLIC_API static size_t initialHeapSizeLimit_;         // Initial heap size
    PANDA_PUBLIC_API static size_t heapSizeLimit_;                // Max heap size
    PANDA_PUBLIC_API static size_t internalMemorySizeLimit_;      // Max internal memory used by the VM
    PANDA_PUBLIC_API static size_t codeCacheSizeLimit_;           // The limit for compiled code size.
    PANDA_PUBLIC_API static size_t compilerMemorySizeLimit_;      // Max memory used by compiler
    PANDA_PUBLIC_API static size_t framesMemorySizeLimit_;        // Max memory used for frames
    PANDA_PUBLIC_API static size_t nativeStacksMemorySizeLimit_;  // Limit for manually (i.e. not by OS means on
                                                                  // thread creation) allocated native stacks
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_MEM_CONFIG_H
