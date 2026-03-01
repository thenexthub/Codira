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
#ifndef RUNTIME_INCLUDE_TAIHE_COMMON_H_
#define RUNTIME_INCLUDE_TAIHE_COMMON_H_
// NOLINTBEGIN

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define TH_NONNULL __attribute__((nonnull))

#define TH_ASSERT(condition, message)                                          \
    do {                                                                       \
        if (!(condition)) {                                                    \
            fprintf(stderr,                                                    \
                    "Assertion failed: (%s), function %s, file %s, line %d.\n" \
                    "Message: %s\n",                                           \
                    #condition, __FUNCTION__, __FILE__, __LINE__, message);    \
            abort();                                                           \
        }                                                                      \
    } while (0)

#ifdef __cplusplus
#define TH_EXPORT extern "C" __attribute__((visibility("default")))
#else
#define TH_EXPORT extern __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
#define TH_INLINE inline
#else
#define TH_INLINE static inline
#endif

// REFERENCE COUNTING //

typedef uint32_t TRefCount;

// Sets the counter to a fixed value.
TH_INLINE void tref_init(TRefCount *c, TRefCount i)
{
    *c = i;
}

// Increments the refcount and returns the *original* value before add.
TH_INLINE void tref_inc(TRefCount *c)
{
    __atomic_fetch_add(c, 1, __ATOMIC_RELAXED);
}

// Decrements the refcount and returns whether the memory should be freed.
TH_INLINE bool tref_dec(TRefCount *c)
{
    return __atomic_sub_fetch(c, 1, __ATOMIC_ACQ_REL) == 0;
}
// NOLINTEND
#endif  // RUNTIME_INCLUDE_TAIHE_COMMON_H_