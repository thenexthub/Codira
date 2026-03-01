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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_PRODUCT_OPTIONS_H
#define PANDA_PLUGINS_ETS_RUNTIME_PRODUCT_OPTIONS_H

#include <array>

namespace ark::ets {

// clang-format off
static constexpr std::array AVAILABLE_RUNTIME_OPTIONS_IN_PRODUCT = {
    "aot-file",
    "aot-files",
    "boot-panda-files",
    "compiler-enable-jit",
    "coroutine-e-workers-limit",
    "coroutine-enable-external-scheduling",
    "coroutine-workers-count",
    "enable-an",
    "heap-size-limit",
    "g1-pause-time-goal",
    "g1-pause-time-goal:max-gc-pause",
    "g1-pause-time-goal:gc-pause-interval",
    "gc-trigger-type",
    "gc-type",
    "native-library-path",
    "panda-files",
    "run-gc-in-place",
    "sampling-profiler-enable",
    "sampling-profiler-output-file",
    "verification-mode",
#ifdef PANDA_ETS_INTEROP_JS
    "xgc-min-trigger-threshold",
#endif  // PANDA_ETS_INTEROP_JS
};
// clang-format on

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_PRODUCT_OPTIONS_H
