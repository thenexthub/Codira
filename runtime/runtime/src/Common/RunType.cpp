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


#include "RunType.h"

namespace MapleRuntime {
// REMEMBER TO CHANGE NUM_OF_RUN_TYPES WHEN YOU ADD/REMOVE CONFIGS
// this stores a config for each kind of run (represented by an index)
const RunType RunType::TYPES[NUM_OF_RUN_TYPES] = {
    { true, DEFAULT_PAGE_PER_RUN, 16 },

    { true, DEFAULT_PAGE_PER_RUN, 24 },
    { true, DEFAULT_PAGE_PER_RUN, 32 },
    { true, DEFAULT_PAGE_PER_RUN, 40 },
    { true, DEFAULT_PAGE_PER_RUN, 48 },
    { true, DEFAULT_PAGE_PER_RUN, 56 },
    { true, DEFAULT_PAGE_PER_RUN, 64 },
    { true, DEFAULT_PAGE_PER_RUN, 72 },
    { true, DEFAULT_PAGE_PER_RUN, 80 },
    { true, DEFAULT_PAGE_PER_RUN, 88 },
    { true, DEFAULT_PAGE_PER_RUN, 96 },
    // all sizes smaller than RUN_ALLOC_SMALL_SIZE may use cache run to speed up allocation.
    // all sizes smaller than RUN_ALLOC_SMALL_SIZE must be increased by 8 bytes.
    // (these are optimisations, rather than restrictions.)
    { true, DEFAULT_PAGE_PER_RUN, RUN_ALLOC_SMALL_SIZE },

    { false, DEFAULT_PAGE_PER_RUN, 112 },
    { false, DEFAULT_PAGE_PER_RUN, 120 },
    { false, DEFAULT_PAGE_PER_RUN, 128 },
    { false, DEFAULT_PAGE_PER_RUN, 136 },
    { false, DEFAULT_PAGE_PER_RUN, 144 },
    { false, DEFAULT_PAGE_PER_RUN, 152 },
    { false, DEFAULT_PAGE_PER_RUN, 160 },

    { false, DEFAULT_PAGE_PER_RUN, 168 },
    { false, DEFAULT_PAGE_PER_RUN, 176 },

    { false, DEFAULT_PAGE_PER_RUN, 184 },
    { false, DEFAULT_PAGE_PER_RUN, 192 },
    { false, DEFAULT_PAGE_PER_RUN, 200 },
    { false, DEFAULT_PAGE_PER_RUN, 208 },
    { false, DEFAULT_PAGE_PER_RUN, 216 },
    { false, DEFAULT_PAGE_PER_RUN, 224 },
    { false, DEFAULT_PAGE_PER_RUN, 232 },
    { false, DEFAULT_PAGE_PER_RUN, 240 },
    { false, DEFAULT_PAGE_PER_RUN, 248 },
    { false, DEFAULT_PAGE_PER_RUN, 256 },
    { false, DEFAULT_PAGE_PER_RUN, 264 },
    { false, DEFAULT_PAGE_PER_RUN, 272 },
    { false, DEFAULT_PAGE_PER_RUN, 280 },
    { false, DEFAULT_PAGE_PER_RUN, 288 },
    { false, DEFAULT_PAGE_PER_RUN, 296 },
    { false, DEFAULT_PAGE_PER_RUN, 304 },
    { false, DEFAULT_PAGE_PER_RUN, 312 },
    { false, DEFAULT_PAGE_PER_RUN, 320 },
    { false, DEFAULT_PAGE_PER_RUN, 328 },
    { false, DEFAULT_PAGE_PER_RUN, 336 },
    { false, DEFAULT_PAGE_PER_RUN, 344 },
    { false, DEFAULT_PAGE_PER_RUN, 352 },
    { false, DEFAULT_PAGE_PER_RUN, 360 },

    { false, DEFAULT_PAGE_PER_RUN, 400 },
    { false, DEFAULT_PAGE_PER_RUN, 448 },
    { false, DEFAULT_PAGE_PER_RUN, 504 },
    { false, DEFAULT_PAGE_PER_RUN, 576 },
    { false, DEFAULT_PAGE_PER_RUN, 672 },
    { false, DEFAULT_PAGE_PER_RUN, 800 },
    { false, DEFAULT_PAGE_PER_RUN, 1008 },
    { false, DEFAULT_PAGE_PER_RUN, 1344 },
    { false, DEFAULT_PAGE_PER_RUN, RUN_ALLOC_LARGE_SIZE }
};

// this map maps a size ((size >> 3 - 1) to be precise) to a run config
// this map takes 4 * MAX_NUM_OF_RUN_TYPES == 1k
uint32_t RunType::g_size2Idx[MAX_NUM_OF_RUN_TYPES] = { 0 }; // all zero-initialised

// this function inits the config map using the configs above
// so that for any size there is a config for it
// if the size doesn't match any of the configs exactly, we choose
// the closest config with a greater size, e.g.,
// g_size2Idx[456 >> 3 - 1] == g_size2Idx[464 >> 3 - 1] == .. == g_size2Idx[504 >> 3 - 1]
void RunType::InitRunTypeMap()
{
    constexpr uint32_t runSizeShift = 3;
    MRT_ASSERT(RunType::NUM_OF_RUN_TYPES <= RunType::MAX_NUM_OF_RUN_TYPES, "too many configs");
    uint32_t idx = RunType::NUM_OF_RUN_TYPES;
    uint32_t nextSize = RunType::TYPES[RunType::NUM_OF_RUN_TYPES - 1].size;
    MRT_ASSERT(nextSize <= (RunType::MAX_NUM_OF_RUN_TYPES << runSizeShift), "size too big in config");
    uint32_t i = (RunType::MAX_NUM_OF_RUN_TYPES - 1);
    while (true) {
        if (((i + 1) << runSizeShift) > nextSize) {
            if (idx < RunType::NUM_OF_RUN_TYPES) {
                RunType::g_size2Idx[i] = idx;
            }
        } else {
            MRT_ASSERT(((i + 1) << runSizeShift) == nextSize, "init run config error");
            MRT_ASSERT(idx > 0, "init run config error");
            RunType::g_size2Idx[i] = --idx;
            if (idx > 0 && idx < RunType::NUM_OF_RUN_TYPES) {
                MRT_ASSERT(static_cast<size_t>(RunType::TYPES[idx - 1].size) < nextSize, "not in ascending order");
                nextSize = RunType::TYPES[idx - 1].size;
            } else {
                nextSize = 0;
            }
        }

        if (i == 0) {
            break;
        } else {
            --i;
        }
    }

    MRT_ASSERT(RUNTYPE_SIZE_TO_RUN_IDX(RUN_ALLOC_LARGE_SIZE) + 1 == RunType::NUM_OF_RUN_TYPES,
               "run config inconsistent: large size");
}
} // namespace MapleRuntime
