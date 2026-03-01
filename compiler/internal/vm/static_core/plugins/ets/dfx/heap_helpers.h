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

#ifndef PANDA_RUNTIME_ETS_DFX_HEAP_HELPERS_H
#define PANDA_RUNTIME_ETS_DFX_HEAP_HELPERS_H

#include <ani.h>

namespace ark::dfx {

__attribute__((visibility("default"))) uint64_t GetHeapTotalSize(ani_vm *vm);    // total heap size
__attribute__((visibility("default"))) uint64_t GetHeapUsedSize(ani_vm *vm);     // total obj size in heap
__attribute__((visibility("default"))) uint64_t GetArrayBufferSize(ani_vm *vm);  // total size of all array buffer obj
__attribute__((visibility("default"))) uint64_t GetProcessHeapLimitSize(ani_vm *vm);     // heap config setting
__attribute__((visibility("default"))) uint64_t GetHeapLimitSize(ani_vm *vm);            // heap config setting
__attribute__((visibility("default"))) uint64_t GetFullGCLongTimeCount(ani_vm *vm);      // long time gc count
__attribute__((visibility("default"))) uint64_t GetAccumulatedFreeSize(ani_vm *vm);      // accumulate free size
__attribute__((visibility("default"))) uint64_t GetAccumulatedAllocateSize(ani_vm *vm);  // accumulate alloc size
__attribute__((visibility("default"))) uint64_t GetGCCount(ani_vm *vm);     // total number of GC cycles triggered
__attribute__((visibility("default"))) uint64_t GetGCDuration(ani_vm *vm);  // total time spent in last GC

}  // namespace ark::dfx
#endif  // PANDA_RUNTIME_ETS_DFX_HEAP_HELPERS_H
