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

#include "scoped_timer.h"

namespace libabckit {

void ScopedTimer::LogTime(const std::string &name, int64_t time)
{
    LIBABCKIT_LOG_NO_FUNC(DEBUG) << "[" << name << "] execution time: " << time << "us\n";
}

ScopedTimer::ScopedTimer(std::string name) : name_(std::move(name))
{
    start_ = std::chrono::high_resolution_clock::now();
}

ScopedTimer::~ScopedTimer()
{
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
    LogTime(name_, duration.count());
}

}  // namespace libabckit
