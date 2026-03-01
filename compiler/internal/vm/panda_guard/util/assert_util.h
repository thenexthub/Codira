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

#ifndef PANDA_GUARD_UTIL_ASSERT_UTIL_H
#define PANDA_GUARD_UTIL_ASSERT_UTIL_H

#include "macros.h"
#include "util/error.h"

#define PANDA_GUARD_ERROR_PRINT(tag, errCode, desc, cause, solutions)              \
    do {                                                                           \
        panda::guard::Error error((errCode), (tag));                               \
        error.GetDescStream() << desc;           /* CC-OFF(G.PRE.02) string arg */ \
        error.GetCauseStream() << cause;         /* CC-OFF(G.PRE.02) string arg */ \
        error.GetSolutionsStream() << solutions; /* CC-OFF(G.PRE.02) string arg */ \
        error.Print();                                                             \
    } while (0)

#define PANDA_GUARD_ASSERT_PRINT(cond, tag, errCode, desc)       \
    do {                                                         \
        if (UNLIKELY((cond))) {                                  \
            PANDA_GUARD_ERROR_PRINT(tag, errCode, desc, "", ""); \
            std::abort();                                        \
        }                                                        \
    } while (0)

#define PANDA_GUARD_ABORT_PRINT(tag, errCode, desc)          \
    do {                                                     \
        PANDA_GUARD_ERROR_PRINT(tag, errCode, desc, "", ""); \
        std::abort();                                        \
    } while (0)

#endif  // PANDA_GUARD_UTIL_ASSERT_UTIL_H
