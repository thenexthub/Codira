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
#ifndef PANDA_RUNTIME_ARCH_MEMORY_HELPERS_H_
#define PANDA_RUNTIME_ARCH_MEMORY_HELPERS_H_

#if defined(PANDA_TARGET_ARM64)
#include "aarch64/memory.h"
#elif defined(PANDA_TARGET_ARM32)
#include "arm/memory.h"
#elif defined(PANDA_TARGET_X86)
#include "x86/memory.h"
#elif defined(PANDA_TARGET_AMD64)
#include "amd64/memory.h"
#else
#error "Unsupported target"
#endif

#include "libarkbase/macros.h"

namespace ark::arch {

// Forces system-wide full memory synchronization
// NB! It is assumed all panda targets provide such synchronization, which might not be true on all CPU architectures
// Architecture-agnostic C++ memory order provides no reordering guarantees in case just one thread contains memory
// fence while FullMemoryBarrier callers expect all previous reads and writes to be visible in all threads
inline void FullMemoryBarrier()
{
    arch_specific::FullMemoryBarrier();
}

// Actually only full memory barrier guarantees StoreLoad order but separate implementation is used because of #16489
ALWAYS_INLINE inline void StoreLoadBarrier()
{
    __sync_synchronize();
}

}  // namespace ark::arch

#endif  // PANDA_RUNTIME_ARCH_HELPERS_H_
