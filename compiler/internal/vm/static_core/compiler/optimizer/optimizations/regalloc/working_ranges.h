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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_WORKING_RANGES_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_WORKING_RANGES_H

#include "compiler/optimizer/analysis/liveness_analyzer.h"
#include "libarkbase/utils/arena_containers.h"

namespace ark::compiler {
using InstructionsRanges = ArenaDeque<LifeIntervals *>;

struct WorkingRanges {
    explicit WorkingRanges(ArenaAllocator *allocator) : regular(allocator->Adapter()), physical(allocator->Adapter()) {}

    InstructionsRanges regular;   // NOLINT(misc-non-private-member-variables-in-classes)
    InstructionsRanges physical;  // NOLINT(misc-non-private-member-variables-in-classes)
};

/*
 * Add range in sorted order
 */
static inline void AddRange(LifeIntervals *interval, InstructionsRanges *dest)
{
    auto iter = std::upper_bound(dest->begin(), dest->end(), interval,
                                 [](const auto &lhs, const auto &rhs) { return lhs->GetBegin() < rhs->GetBegin(); });
    dest->insert(iter, interval);
}
}  // namespace ark::compiler
#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_WORKING_RANGES_H