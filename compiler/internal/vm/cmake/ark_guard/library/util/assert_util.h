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

#ifndef GUARD_UTIL_ASSERT_UTIL_H
#define GUARD_UTIL_ASSERT_UTIL_H

#include "libarkbase/macros.h"

#include "error.h"

#define GUARD_ERROR_PRINT(errCode, desc, cause, solutions) \
    do {                                                       \
        ark::guard::Error error((errCode));                    \
        error.GetDescStream() << (desc);                       \
        error.GetCauseStream() << (cause);                     \
        error.GetSolutionsStream() << (solutions);             \
        error.Print();                                         \
    } while (0)

#define GUARD_ASSERT(cond, errCode, desc)             \
    do {                                                  \
        if (UNLIKELY((cond))) {                           \
            GUARD_ERROR_PRINT(errCode, desc, "", ""); \
            std::abort();                                 \
        }                                                 \
    } while (0)

#define GUARD_ABORT(errCode, desc)                \
    do {                                              \
        GUARD_ERROR_PRINT(errCode, desc, "", ""); \
        std::abort();                                 \
    } while (0)

#endif  // GUARD_UTIL_ASSERT_UTIL_H