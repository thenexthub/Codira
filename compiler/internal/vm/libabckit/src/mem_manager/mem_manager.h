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

#ifndef MEM_MANAGER_H
#define MEM_MANAGER_H

#include <cstdint>
#include <cstddef>

namespace libabckit {

class MemManager {
public:
    [[maybe_unused]] static MemManager *Initialize(size_t compilerMemSize = 0)
    {
        MemManager::compilerMemSize_ = compilerMemSize;
        static auto memManager = libabckit::MemManager();
        if (!isInitialized_) {
            memManager.InitializeMemory();
        }
        return &memManager;
    };

    static void Finalize();

private:
    void InitializeMemory();
    static size_t compilerMemSize_;
    static bool isInitialized_;
};

constexpr uint64_t SHIFT_KB = 10ULL;
constexpr uint64_t SHIFT_MB = 20ULL;
constexpr uint64_t SHIFT_GB = 30ULL;

constexpr uint64_t operator"" _KB(long double count)
{
    return static_cast<uint64_t>(count * (1ULL << SHIFT_KB));
}

// NOLINTNEXTLINE(google-runtime-int)
constexpr uint64_t operator"" _KB(unsigned long long count)
{
    return count * (1ULL << SHIFT_KB);
}

constexpr uint64_t operator"" _MB(long double count)
{
    return static_cast<uint64_t>(count * (1ULL << SHIFT_MB));
}

// NOLINTNEXTLINE(google-runtime-int)
constexpr uint64_t operator"" _MB(unsigned long long count)
{
    return count * (1ULL << SHIFT_MB);
}

constexpr uint64_t operator"" _GB(long double count)
{
    return static_cast<uint64_t>(count * (1ULL << SHIFT_GB));
}

// CC-OFFNXT(WordsTool.95) sensitive word conflict
// NOLINTNEXTLINE(google-runtime-int)
constexpr uint64_t operator"" _GB(unsigned long long count)
{
    return count * (1ULL << SHIFT_GB);
}

}  // namespace libabckit

#endif
