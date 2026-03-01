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


#ifndef MRT_MEM_UTILS_H
#define MRT_MEM_UTILS_H

namespace MapleRuntime {
// memset_s wrapper for the situation that memset size is greater than SECUREC_MEM_MAX_LEN(2GB).
void MemorySet(uintptr_t dest, size_t destMax, int c, size_t count);
// memcpy_s wrapper for the situation that memcpy size is greater than SECUREC_MEM_MAX_LEN(2GB).
void MemoryCopy(uintptr_t dest, size_t destMax, const uintptr_t src, size_t count);
#if defined(ENABLE_BACKWARD_PTRAUTH_CFI) || defined(ENABLE_FORWARD_PTRAUTH_CFI)
__attribute__((always_inline))uintptr_t PtrauthStripInstPointer(uintptr_t ptr);
__attribute__((always_inline))uintptr_t PtrauthAuthWithInstAkey(uintptr_t ptr, uintptr_t mod);
__attribute__((always_inline))uintptr_t PtrauthSignWithInstAkey(uintptr_t ptr, uintptr_t mod);
#endif
} // namespace MapleRuntime
#endif // MRT_MEM_UTILS_H
