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


#ifndef MRT_BASE_MACROS_H
#define MRT_BASE_MACROS_H

// An abstraction layer to hide compiler-specific attributes/builtins

#include <cstddef>
#include <unistd.h>

// Macro definitions for attribute
#define ATTR_NO_INLINE __attribute__((noinline))

#define ATTR_PACKED(x) __attribute__((__aligned__(x), __packed__))

#define ATTR_UNUSED __attribute__((__unused__))

#define ATTR_WARN_UNUSED __attribute__((warn_unused_result))

#ifndef MRT_EXPORT
#define MRT_EXPORT __attribute__((visibility("default")))
#endif

#if defined(__APPLE__)
#define ATTR_HOT
#define ATTR_COLD
#else // not __APPLE__
#define ATTR_HOT __attribute__((hot))
#define ATTR_COLD __attribute__((cold))
#endif // __APPLE__

#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
#define ALWAYS_INLINE
#else
#define ALWAYS_INLINE __attribute__((always_inline))
#endif // MRT_DEBUG

#if defined(__clang__)
#define ATTR_NO_SANITIZE_ADDRESS __attribute__((no_sanitize("address")))
#else
#define ATTR_NO_SANITIZE_ADDRESS
#endif // __clang__

// Macro definitions for builtin
#define BUILTIN_UNREACHABLE __builtin_unreachable

#define LIKELY(exp) (__builtin_expect(static_cast<long>(!!(exp)), 1))

#define UNLIKELY(exp) (__builtin_expect(static_cast<long>(!!(exp)), 0))

// Macro definitions for class operator
#define DISABLE_CLASS_DEFAULT_CONSTRUCTOR(ClassName) ClassName() = delete

#define DISABLE_CLASS_COPY_CONSTRUCTOR(ClassName) ClassName(const ClassName&) = delete

#define DISABLE_CLASS_ASSIGN_OPERATOR(ClassName) ClassName& operator=(const ClassName&) = delete

#define DISABLE_CLASS_IMPLICIT_DESTRUCTION(ClassName) ~ClassName() = delete

#define DISABLE_CLASS_COPY_AND_ASSIGN(ClassName) \
    DISABLE_CLASS_COPY_CONSTRUCTOR(ClassName); \
    DISABLE_CLASS_ASSIGN_OPERATOR(ClassName)

#define DISABLE_CLASS_IMPLICIT_CONSTRUCTORS(ClassName) \
    DISABLE_CLASS_DEFAULT_CONSTRUCTOR(ClassName); \
    DISABLE_CLASS_COPY_CONSTRUCTOR(ClassName); \
    DISABLE_CLASS_ASSIGN_OPERATOR(ClassName)

#define NO_RETURN [[noreturn]]

#ifndef FALLTHROUGH_INTENDED
#define FALLTHROUGH_INTENDED [[clang::fallthrough]]
#endif

#if defined(__LP64__) || defined(_WIN64)
#define POINTER_IS_64BIT
#endif

#endif // MRT_BASE_MACROS_H
