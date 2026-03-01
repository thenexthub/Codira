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

#ifndef COMMON_COMPONENTS_BASE_MEM_UTILS_H
#define COMMON_COMPONENTS_BASE_MEM_UTILS_H

namespace common {
// memset_s wrapper for the situation that memset size is greater than SECUREC_MEM_MAX_LEN(2GB).
void MemorySet(uintptr_t dest, size_t destMax, int c, size_t count);
// memcpy_s wrapper for the situation that memcpy size is greater than SECUREC_MEM_MAX_LEN(2GB).
void MemoryCopy(uintptr_t dest, size_t destMax, const uintptr_t src, size_t count);
} // namespace common

#endif // COMMON_COMPONENTS_BASE_MEM_UTILS_H