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

#ifndef COMMON_COMPONENTS_BASE_TIME_UTILS_H
#define COMMON_COMPONENTS_BASE_TIME_UTILS_H

#include <cinttypes>

#include "common_components/base/c_string.h"
#include "common_interfaces/base/common.h"

namespace common {
namespace TimeUtil {
// returns the monotonic time since epoch starting point in milliseconds
uint64_t MilliSeconds();

// returns the monotonic time since epoch starting point in nanoseconds
PUBLIC_API uint64_t NanoSeconds();

// returns the monotonic time since epoch starting point in microseconds
uint64_t MicroSeconds() noexcept;

// sleep for the given count of nanoseconds
void SleepForNano(uint64_t ns);

#ifndef NDEBUG
// returns the current date in ISO yyymmdd.hhmmss format
CString GetDigitDate();
#endif

// returns the current date in ISO yyyy-mm-dd hh:mm::ss.ms format
PUBLIC_API CString GetTimestamp();
} // namespace TimeUtil
} // namespace common

#endif // COMMON_COMPONENTS_BASE_TIME_UTILS_H
