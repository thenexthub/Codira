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

#ifndef LIBPANDAFILE_MODIFIERS_H_
#define LIBPANDAFILE_MODIFIERS_H_

#include "libarkbase/utils/bit_utils.h"

#include <cstdint>

namespace ark {

constexpr uint32_t ACC_PUBLIC = 0x0001;        // field, method, class
constexpr uint32_t ACC_PRIVATE = 0x0002;       // field, method, class
constexpr uint32_t ACC_PROTECTED = 0x0004;     // field, method, class
constexpr uint32_t ACC_STATIC = 0x0008;        // field, method
constexpr uint32_t ACC_FINAL = 0x0010;         // field, method, class
constexpr uint32_t ACC_SUPER = 0x0020;         // class
constexpr uint32_t ACC_SYNCHRONIZED = 0x0020;  // method
constexpr uint32_t ACC_BRIDGE = 0x0040;        // method
constexpr uint32_t ACC_VOLATILE = 0x0040;      // field
constexpr uint32_t ACC_TRANSIENT = 0x0080;     // field,
constexpr uint32_t ACC_VARARGS = 0x0080;       // method
constexpr uint32_t ACC_NATIVE = 0x0100;        // method
constexpr uint32_t ACC_INTERFACE = 0x0200;     // class
constexpr uint32_t ACC_ABSTRACT = 0x0400;      // method, class
constexpr uint32_t ACC_STRICT = 0x0800;        // method
constexpr uint32_t ACC_SYNTHETIC = 0x1000;     // field, method, class
constexpr uint32_t ACC_ANNOTATION = 0x2000;    // class
constexpr uint32_t ACC_ENUM = 0x4000;          // field, class
constexpr uint32_t ACC_DESTROYED = 0x8000;     // method
constexpr uint32_t ACC_READONLY = 0x0020;      // field

constexpr uint32_t ACC_FILE_MASK = 0xFFFF;

// Field type
constexpr uint32_t ACC_TYPE = 0x00FF0000;  // field
constexpr uint32_t ACC_TYPE_SHIFT = Ctz(ACC_TYPE);

// Runtime internal modifiers
constexpr uint32_t ACC_HAS_DEFAULT_METHODS = 0x00010000;       // class (runtime)
constexpr uint32_t ACC_CONSTRUCTOR = 0x00010000;               // method (runtime)
constexpr uint32_t ACC_DEFAULT_INTERFACE_METHOD = 0x00020000;  // method (runtime)
constexpr uint32_t ACC_SINGLE_IMPL = 0x00040000;               // method (runtime)
constexpr uint32_t ACC_INTRINSIC = 0x00200000;                 // method (runtime)
constexpr uint32_t ACC_PROFILING = 0x20000000;                 // method (runtime)

constexpr uint32_t INTRINSIC_SHIFT = MinimumBitsToStore(ACC_INTRINSIC);
constexpr uint32_t MAX_INTRINSIC_NUMBER = std::numeric_limits<uint32_t>::max();

// Runtime internal language specific modifiers
constexpr uint32_t ACC_PROXY = 0x00020000;            // class (runtime)
constexpr uint32_t ACC_FAST_NATIVE = 0x00080000;      // method (runtime)
constexpr uint32_t ACC_CRITICAL_NATIVE = 0x00100000;  // method (runtime)

constexpr uint32_t ACC_VERIFICATION_STATUS = 0x00400000;  // method (runtime)
constexpr uint32_t VERIFICATION_STATUS_SHIFT = MinimumBitsToStore(ACC_VERIFICATION_STATUS);
constexpr uint32_t VERIFICATION_STATUS_WIDTH = 0x2;
constexpr uint32_t VERIFICATION_STATUS_MASK = ((1U << VERIFICATION_STATUS_WIDTH) - 1U) << VERIFICATION_STATUS_SHIFT;

constexpr uint32_t ACC_COMPILATION_STATUS = 0x02000000;  // method (runtime)
constexpr uint32_t COMPILATION_STATUS_SHIFT = MinimumBitsToStore(ACC_COMPILATION_STATUS);
constexpr uint32_t COMPILATION_STATUS_MASK = static_cast<uint32_t>(0x7) << COMPILATION_STATUS_SHIFT;
}  // namespace ark

#endif
