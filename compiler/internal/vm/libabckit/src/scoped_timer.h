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

#ifndef LIBABCKIT_SCOPED_TIMER_H
#define LIBABCKIT_SCOPED_TIMER_H

#include <chrono>
#include <string>

#include "logger.h"
#include "macros.h"

namespace libabckit {

class ScopedTimer {
public:
    explicit ScopedTimer(std::string name);
    ~ScopedTimer();

    ScopedTimer(const ScopedTimer &) = delete;
    ScopedTimer &operator=(const ScopedTimer &) = delete;
    ScopedTimer(ScopedTimer &&) = delete;
    ScopedTimer &operator=(ScopedTimer &&) = delete;

private:
    std::string name_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
    static void LogTime(const std::string &name, int64_t time);
};

}  // namespace libabckit

// CC-OFFNXT(G.PRE.09) code generation
// CC-OFFNXT(G.PRE.02) necessary macro
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LIBABCKIT_TIME_EXEC ScopedTimer LIBABCKIT_UNIQUE_VAR(timer)(LIBABCKIT_FUNC_NAME)

#endif  // LIBABCKIT_SCOPED_TIMER_H
