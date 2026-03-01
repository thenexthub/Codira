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

#ifndef PANDA_GLOBALS_H
#define PANDA_GLOBALS_H

#include <cstdint>

namespace ark {

static constexpr unsigned BITS_PER_BYTE = 8;
static constexpr unsigned BITS_PER_UINT16 = sizeof(uint16_t) * BITS_PER_BYTE;
static constexpr unsigned BITS_PER_UINT32 = sizeof(uint32_t) * BITS_PER_BYTE;
static constexpr unsigned BITS_PER_UINT64 = sizeof(uint64_t) * BITS_PER_BYTE;
static constexpr unsigned BITS_PER_INTPTR = sizeof(intptr_t) * BITS_PER_BYTE;

static constexpr unsigned BITS_PER_BYTE_LOG2 = 3;

static constexpr uint32_t PERCENT_100_U32 = 100U;
static constexpr double PERCENT_100_D = 100.0;

#ifndef NDEBUG
static constexpr bool DEBUG_BUILD = true;
#else
static constexpr bool DEBUG_BUILD = false;
#endif

}  // namespace ark

#endif  // PANDA_GLOBALS_H
