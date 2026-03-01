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
#ifndef LIBABCKIT_MACROS_H
#define LIBABCKIT_MACROS_H

#include <iostream>
#include "libabckit/src/statuses_impl.h"

#include "libabckit/c/statuses.h"
#include "libabckit/src/logger.h"

/*
 * NOTE: __FUNCTION__ and __PRETTY_FUNCTION__ are non-standard extensions. Use __func__ for portability.
 */

#define LIBABCKIT_CLEAR_LAST_ERROR statuses::SetLastError(ABCKIT_STATUS_NO_ERROR)

#define LIBABCKIT_RETURN_VOID
// CC-OFFNXT(G.PRE.02) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_BAD_ARGUMENT(argument, returnvalue)       \
    if ((argument) == nullptr) {                            \
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT); \
        /* CC-OFFNXT(G.PRE.05) code generation */           \
        return returnvalue;                                 \
    }
// CC-OFFNXT(G.PRE.02) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_BAD_ARGUMENT_VOID(argument)               \
    if ((argument) == nullptr) {                            \
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT); \
        /* CC-OFFNXT(G.PRE.05) code generation */           \
        return;                                             \
    }
// CC-OFFNXT(G.PRE.02) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_INTERNAL_ERROR(argument, returnValue)       \
    if ((argument) == nullptr) {                              \
        statuses::SetLastError(ABCKIT_STATUS_INTERNAL_ERROR); \
        /* CC-OFFNXT(G.PRE.05) code generation */             \
        return returnValue;                                   \
    }
// CC-OFFNXT(G.PRE.02) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_INTERNAL_ERROR_VOID(argument)               \
    if ((argument) == nullptr) {                              \
        statuses::SetLastError(ABCKIT_STATUS_INTERNAL_ERROR); \
        /* CC-OFFNXT(G.PRE.05) code generation */             \
        return;                                               \
    }
// CC-OFFNXT(G.PRE.02) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_WRONG_CTX(lhs, rhs, returnvalue)       \
    if ((lhs) != (rhs)) {                                \
        statuses::SetLastError(ABCKIT_STATUS_WRONG_CTX); \
        /* CC-OFFNXT(G.PRE.05) code generation */        \
        return returnvalue;                              \
    }
// CC-OFFNXT(G.PRE.02) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_WRONG_CTX_VOID(lhs, rhs)               \
    if ((lhs) != (rhs)) {                                \
        statuses::SetLastError(ABCKIT_STATUS_WRONG_CTX); \
        /* CC-OFFNXT(G.PRE.05) code generation */        \
        return;                                          \
    }
// CC-OFFNXT(G.PRE.02) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_WRONG_MODE(graph, expectedMode, returnValue)       \
    if ((graph)->file->frontend != (expectedMode)) {                 \
        libabckit::statuses::SetLastError(ABCKIT_STATUS_WRONG_MODE); \
        /* CC-OFFNXT(G.PRE.05) code generation */                    \
        return returnValue;                                          \
    }
// CC-OFFNXT(G.PRE.02) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_ZERO_ARGUMENT(argument, returnvalue)      \
    if ((argument) == 0) {                                  \
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT); \
        /* CC-OFFNXT(G.PRE.05) code generation */           \
        return returnvalue;                                 \
    }
// CC-OFFNXT(G.PRE.02) code readability
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_ZERO_ARGUMENT_VOID(argument)              \
    if ((argument) == 0) {                                  \
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT); \
        /* CC-OFFNXT(G.PRE.05) code generation */           \
        return;                                             \
    }
// CC-OFFNXT(G.PRE.09) code generation
// CC-OFFNXT(G.PRE.02) necessary macro
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_CONCAT_INNER(x, y) x##y
// CC-OFFNXT(G.PRE.09) code generation
// CC-OFFNXT(G.PRE.02) necessary macro
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_CONCAT(x, y) LIBABCKIT_CONCAT_INNER(x, y)
// CC-OFFNXT(G.PRE.09) code generation
// CC-OFFNXT(G.PRE.02) necessary macro
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_UNIQUE_VAR(name) LIBABCKIT_CONCAT(_##name##_, LIBABCKIT_LINE)
#endif
