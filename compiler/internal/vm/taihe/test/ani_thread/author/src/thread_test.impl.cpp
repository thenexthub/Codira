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
#include "thread_test.impl.hpp"
#include "stdexcept"
#include "taihe/runtime.hpp"
#include "thread_test.proj.hpp"

#include <chrono>
#include <iostream>
#include <thread>

using namespace taihe;

namespace {
static constexpr int32_t THOUSAND = 1000;

void invokeFromOtherThreadAfter(double sec, callback_view<void()> cb)
{
    std::cerr << "-- begin invokeFromOtherThreadAfter --" << std::endl;
    std::thread thread([sec, cb = callback<void()>(cb)]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sec * THOUSAND)));
        std::cerr << "invokeFromOtherThreadAfter: " << sec << " seconds" << std::endl;
        cb();
    });
    thread.detach();
    std::cerr << "-- end invokeFromOtherThreadAfter --" << std::endl;
}
}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_invokeFromOtherThreadAfter(invokeFromOtherThreadAfter);
// NOLINTEND