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

#ifndef PANDA_LIBPANDABASE_TESTS_GENMC_COMMON_H
#define PANDA_LIBPANDABASE_TESTS_GENMC_COMMON_H

#include <pthread.h>
#include <cstdlib>
#define MC_ON
#include "platforms/unix/libarkbase/futex/fmutex.cpp"

thread_local pthread_t current_tid;

inline int g_shared;

// The function checks if a data race may be present:
// it writes down the argument to the shared variable
// and checks if the value is not changed
inline void CheckGlobalVar(int index)
{
    g_shared = index;
    int r = g_shared;
    ASSERT(r == index);
}

#endif  // PANDA_LIBPANDABASE_TESTS_GENMC_COMMON_H
