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

#ifndef PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLE_READER_H
#define PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLE_READER_H

#include "libarkbase/macros.h"

#include "runtime/tooling/sampler/sample_info.h"

namespace ark::tooling::sampler {

// Reader of .aspt format
class SampleReader final {
public:
    // clang-format off
    static constexpr size_t SAMPLE_THREAD_ID_OFFSET     = 0 * sizeof(uint32_t);
    static constexpr size_t SAMPLE_THREAD_STATUS_OFFSET = 1 * sizeof(uint32_t);
    static constexpr size_t SAMPLE_STACK_SIZE_OFFSET    = 2 * sizeof(uint32_t);
    static constexpr size_t SAMPLE_STACK_OFFSET         = 2 * sizeof(uint32_t) + 1 * sizeof(uintptr_t);

    static constexpr size_t PANDA_FILE_POINTER_OFFSET   = 1 * sizeof(uintptr_t);
    static constexpr size_t PANDA_FILE_CHECKSUM_OFFSET  = 2 * sizeof(uintptr_t);
    static constexpr size_t PANDA_FILE_NAME_SIZE_OFFSET = 2 * sizeof(uintptr_t) + 1 * sizeof(uint32_t);
    static constexpr size_t PANDA_FILE_NAME_OFFSET      = 3 * sizeof(uintptr_t) + 1 * sizeof(uint32_t);
    // clang-format on
    inline explicit SampleReader(const char *filename);
    ~SampleReader() = default;

    inline bool GetNextSample(SampleInfo *sampleOut);
    inline bool GetNextModule(FileInfo *moduleOut);

    NO_COPY_SEMANTIC(SampleReader);
    NO_MOVE_SEMANTIC(SampleReader);

private:
    // Using std::vector instead of PandaVector 'cause it should be used in tool without runtime
    std::vector<char> buffer_;
    std::vector<char *> sampleRowPtrs_;
    std::vector<char *> moduleRowPtrs_;
    size_t sampleRowCounter_ {0};
    size_t moduleRowCounter_ {0};
};

}  // namespace ark::tooling::sampler

#endif  // PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLE_READER_H
