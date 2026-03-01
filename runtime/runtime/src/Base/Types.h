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


#ifndef MRT_BASE_TYPES_H
#define MRT_BASE_TYPES_H

#include <cstddef>
#include <cstdint>
#if defined(__linux__) || defined(__APPLE__)
#include <sys/types.h>
#elif defined(hongmeng)
#include <unistd.h>
#endif

// general types (for unmanaged world)
namespace MapleRuntime {
using U1 = uint8_t;
using I1 = int8_t;
using U8 = uint8_t;
using I8 = int8_t;
using U16 = uint16_t;
using I16 = int16_t;
using U32 = uint32_t;
using I32 = int32_t;
using U64 = uint64_t;
using I64 = int64_t;

using F16 = uint16_t;
using F32 = float;
using F64 = double;
// platform dependent types

// a more general set of pointer type
#if defined(__APPLE__)
    using Uptr = uint64_t;
#else
    using Uptr = uintptr_t;
#endif
using Sptr = intptr_t;

// these're related: used for field declaration and parameter.
using Size = ssize_t;
using USize = size_t;
using Index = size_t;
using Offset = size_t;

} // namespace MapleRuntime

#endif // MRT_BASE_TYPES_H
