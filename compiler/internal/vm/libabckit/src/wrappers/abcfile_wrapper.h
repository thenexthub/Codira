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

#ifndef LIBABCKIT_SRC_WRAPPERS_ABCFILE_WRAPPER_H
#define LIBABCKIT_SRC_WRAPPERS_ABCFILE_WRAPPER_H

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <functional>

namespace libabckit {

// NOTE: Fix after discussion with author
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class FileWrapper {
public:
    explicit FileWrapper(void *abcFile) : abcFile_(abcFile) {}
    explicit FileWrapper(const void *abcFile) : abcFile_(abcFile) {}
    ~FileWrapper();

    size_t GetMethodTotalArgumentsCount(void *method) const;
    size_t GetMethodArgumentsCount([[maybe_unused]] void *caller, uint32_t id) const;
    size_t GetMethodRegistersCount(void *method) const;
    const uint8_t *GetMethodCode(void *method) const;
    size_t GetMethodCodeSize(void *method) const;
    uint8_t GetMethodSourceLanguage(void *method) const;
    size_t GetClassIdForMethod(void *method) const;
    std::string GetClassNameFromMethod(void *method) const;
    std::string GetMethodName(void *method) const;
    uint32_t ResolveOffsetByIndex(void *method, uint16_t index) const;
    void EnumerateTryBlocks(void *method, const std::function<void(void *)> &cb) const;
    void EnumerateCatchBlocksForTryBlock(void *tryBlock, const std::function<void(void *)> &cb) const;
    std::pair<int, int> GetTryBlockBoundaries(void *tryBlock) const;
    uint32_t GetCatchBlockHandlerPc(void *catchBlock) const;

private:
    const void *abcFile_;
};

}  // namespace libabckit

#endif  // LIBABCKIT_SRC_WRAPPERS_ABCFILE_WRAPPER_H
