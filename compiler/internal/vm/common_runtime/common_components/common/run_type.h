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

#ifndef COMMON_COMPONENTS_COMMON_RUN_TYPE_H
#define COMMON_COMPONENTS_COMMON_RUN_TYPE_H

#include "common_components/log/log.h"

namespace common {
// slot with size no more than RUN_ALLOC_SMALL_SIZE is small slot.
// small slot is allocated from cache run in thread-local buffer first.
// slot with size (RUN_ALLOC_SMALL_SIZE, RUN_ALLOC_LARGE_SIZE] is allocated from global buffer.
constexpr size_t RUN_ALLOC_SMALL_SIZE = 104;
// slot with size larger than RUN_ALLOC_LARGE_SIZE is large slot.
constexpr size_t RUN_ALLOC_LARGE_SIZE = 2016;

// this is a short cut of RUNTYPE_SIZE_TO_RUN_IDX, only works under certain configs, see TYPES def
#define RUNTYPE_FAST_RUN_IDX(size) (((size) >> 3) - 2)

struct RunType {
public:
    // this supports a maximum of (256 * 8 == 2048 byte) run
    // we need to extend this if we want to config multiple-page run
    static constexpr uint32_t MAX_NUM_OF_RUN_TYPES = 256;

    // REMEMBER TO CHANGE THIS WHEN YOU ADD/REMOVE CONFIGS
    static constexpr uint32_t NUM_OF_RUN_TYPES = 53;

    static constexpr uint32_t NUM_OF_LOCAL_TYPES = RUNTYPE_FAST_RUN_IDX(RUN_ALLOC_SMALL_SIZE) + 1;

    // REMEMBER TO CHANGE NUM_OF_RUN_TYPES WHEN YOU ADD/REMOVE CONFIGS
    // this stores a config for each kind of run (represented by an index)
    static const RunType TYPES[NUM_OF_RUN_TYPES];

    // this map maps a size ((size >> 3 - 1) to be precise) to a run config
    // this map takes 4 * MAX_NUM_OF_RUN_TYPES == 1k
    static uint32_t g_size2Idx[MAX_NUM_OF_RUN_TYPES]; // all zero-initialised

    const bool isSmall; // this kind of run is composed of small-sized slots.

    const uint8_t numPagesPerRun; // pages per run

    const uint32_t size; // slot size of this kind of run

    static void InitRunTypeMap();
};

// assume(size <= (TYPES[N_RUN_CONFIGS - 1].size << 3))
#define RUNTYPE_SIZE_TO_RUN_IDX(size) RunType::g_size2Idx[((size) >> 3) - 1]

#define RUNTYPE_RUN_IDX_TO_SIZE(idx) (RunType::TYPES[(idx)].size)

constexpr int DEFAULT_PAGE_PER_RUN = 1;
} // namespace common

#endif // COMMON_COMPONENTS_COMMON_RUN_TYPE_H
