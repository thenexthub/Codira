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

#ifndef PANDA_LIBPANDABASE_PBASE_OS_TIME_H_
#define PANDA_LIBPANDABASE_PBASE_OS_TIME_H_

#ifdef PANDA_TARGET_UNIX
#include "platforms/unix/libarkbase/time.h"
#elif PANDA_TARGET_WINDOWS
#include "platforms/windows/libarkbase/time.h"
#else
#error "Unsupported platform"
#endif  // PANDA_TARGET_UNIX

#include <libarkbase/macros.h>

#include <cstdint>

WEAK_FOR_LTO_START

namespace ark::os::time {

static constexpr uint64_t MICRO_TO_NANO = 1000;
static constexpr uint64_t MILLIS_TO_MICRO = 1000;
static constexpr uint64_t MILLIS_TO_NANO = MILLIS_TO_MICRO * MICRO_TO_NANO;

PANDA_PUBLIC_API uint64_t GetClockTimeInMicro();
PANDA_PUBLIC_API uint64_t GetClockTimeInMilli();
PANDA_PUBLIC_API uint64_t GetClockTimeInThreadCpuTime();

}  // namespace ark::os::time

WEAK_FOR_LTO_END

#endif  // PANDA_LIBPANDABASE_PBASE_OS_TIME_H_
