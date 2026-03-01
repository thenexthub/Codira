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

#ifndef PANDA_DEBUG_H
#define PANDA_DEBUG_H

#include "../../../libpandabase/include/libpandabase/utils/debug.h"

namespace ark::debug {

[[noreturn]] PANDA_PUBLIC_API void AssertionFail(const char *expr, const char *file, unsigned line,
                                                 const char *function);

#if defined(PANDA_TARGET_MOBILE) || defined(PANDA_TARGET_WINDOWS) || defined(PANDA_TARGET_OHOS)
inline void PrintStackTrace([[maybe_unused]] int skip = 1) {}
#else
void PrintStackTrace(int skip = 1);
#endif

}  // namespace ark::debug

#endif  // PANDA_DEBUG_H
