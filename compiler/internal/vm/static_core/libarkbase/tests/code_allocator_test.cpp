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

#include "libarkbase/mem/code_allocator.h"
#include "libarkbase/mem/pool_manager.h"
#include "libarkbase/mem/base_mem_stats.h"

#include "gtest/gtest.h"
#include "libarkbase/utils/logger.h"

namespace ark {

// NOLINTBEGIN(readability-magic-numbers)

class CodeAllocatorTest : public testing::Test {
public:
    CodeAllocatorTest() = default;

    ~CodeAllocatorTest() override
    {
        Logger::Destroy();
    }
    NO_COPY_SEMANTIC(CodeAllocatorTest);
    NO_MOVE_SEMANTIC(CodeAllocatorTest);

protected:
    static constexpr size_t K1_K = 1024;
    static constexpr size_t TEST_HEAP_SIZE = K1_K * K1_K;

    static bool IsAligned(const void *ptr, size_t alignment)
    {
        return reinterpret_cast<uintptr_t>(ptr) % alignment == 0;
    }

    void SetUp() override
    {
        ark::mem::MemConfig::Initialize(0U, 32_MB, 0U, 32_MB, 0U, 0U);
        PoolManager::Initialize();
    }

    void TearDown() override
    {
        PoolManager::Finalize();
        ark::mem::MemConfig::Finalize();
    }
};

TEST_F(CodeAllocatorTest, AllocateBuffTest)
{
    BaseMemStats stats;
    CodeAllocator ca(&stats);
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    uint8_t buff[] = {0xCCU, 0xCCU};
    void *codeBuff = ca.AllocateCode(sizeof(buff), static_cast<void *>(&buff[0U]));
    for (size_t i = 0; i < sizeof(buff); i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        ASSERT_EQ(static_cast<uint8_t *>(codeBuff)[i], 0xCCU);
    }
    ASSERT_TRUE(IsAligned(codeBuff, 4U * SIZE_1K));
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark
