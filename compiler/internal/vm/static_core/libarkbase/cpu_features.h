/**
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef PANDA_CPU_FEATURES_H
#define PANDA_CPU_FEATURES_H

#include <cstddef>
#include "libarkbase/macros.h"

namespace ark::compiler {
PANDA_PUBLIC_API bool CpuFeaturesHasCrc32();
PANDA_PUBLIC_API bool CpuFeaturesHasJscvt();
PANDA_PUBLIC_API bool CpuFeaturesHasAtomics();
}  // namespace ark::compiler

namespace ark {
#if defined(PANDA_TARGET_AMD64) || defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_ARM32)
#if defined(PANDA_TARGET_MACOS) && defined(PANDA_TARGET_ARM64)
// Apple M-family processors has a wider cache line.
static constexpr size_t CACHE_LINE_SIZE = 128;
#else
static constexpr size_t CACHE_LINE_SIZE = 64;
#endif
#else
static constexpr size_t CACHE_LINE_SIZE = 0;
static_assert(CACHE_LINE_SIZE, "Undefined cacheline size");
#endif
}  // namespace ark

#endif  // PANDA_CPU_FEATURES_H
