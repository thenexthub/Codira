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


#ifndef MRT_PANIC_H
#define MRT_PANIC_H

#include <cassert>
#include <cstdlib>

#include "CodiraRuntime.h"
#include "StackManager.h"

namespace MapleRuntime {
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
#if (defined(__OHOS__) && (__OHOS__ == 1))
#define MRT_ASSERT(p, msg) \
    do { \
        if (!(p)) { \
            if (OH_LOG_IsLoggable(LOG_DOMAIN, LOG_TAG, LOG_INFO)) {       \
                OH_LOG_INFO(LOG_APP, "%{public}s:%{public}d:%{public}s", __FILE__, __LINE__, msg);                    \
            } \
            abort(); \
        } \
    } while (0)
#else
#define MRT_ASSERT(p, msg) \
    do { \
        if (!(p)) { \
            (void)PRINT_INFO("%s:%d:%s", __FILE__, __LINE__, msg); \
            abort(); \
        } \
    } while (0)
#endif
#define ASSERT(f) assert(f)
#else
#define MRT_ASSERT(p, msg)
#define ASSERT(f)
#endif
} // namespace MapleRuntime

#endif // MRT_PANIC_H
