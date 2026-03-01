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

#include <random>
#include <gtest/gtest.h>

#include "libarkbase/mem/pool_manager.h"
#include "target/aarch64/target.h"
#include "scoped_tmp_reg.h"

namespace ark::compiler {
class Register64Test : public ::testing::Test {
public:
    Register64Test()
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        ark::mem::MemConfig::Initialize(64_MB, 64_MB, 64_MB, 32_MB, 0, 0);
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
    }
    ~Register64Test() override
    {
        delete allocator_;
        PoolManager::Finalize();
        ark::mem::MemConfig::Finalize();
    }

    NO_COPY_SEMANTIC(Register64Test);
    NO_MOVE_SEMANTIC(Register64Test);

    ArenaAllocator *GetAllocator()
    {
        return allocator_;
    }

private:
    ArenaAllocator *allocator_;
};

TEST_F(Register64Test, TempRegisters)
{
    aarch64::Aarch64Encoder encoder(GetAllocator());
    ASSERT_TRUE(encoder.InitMasm());

    auto floatType = FLOAT64_TYPE;

    auto initialCount = encoder.GetScratchRegistersCount();
    auto initialFpCount = encoder.GetScratchFPRegistersCount();
    ASSERT_NE(initialCount, 0);
    ASSERT_NE(initialFpCount, 0);

    std::vector<Reg> regs;
    for (size_t i = 0; i < initialCount; i++) {
        regs.push_back(encoder.AcquireScratchRegister(INT64_TYPE));
    }
    ASSERT_EQ(encoder.GetScratchRegistersCount(), 0);
    ASSERT_EQ(encoder.GetScratchFPRegistersCount(), initialFpCount);
    for (auto reg : regs) {
        encoder.ReleaseScratchRegister(reg);
    }
    ASSERT_EQ(encoder.GetScratchRegistersCount(), initialCount);

    regs.clear();
    for (size_t i = 0; i < initialFpCount; i++) {
        regs.push_back(encoder.AcquireScratchRegister(floatType));
    }

    ASSERT_EQ(encoder.GetScratchRegistersCount(), initialCount);
    ASSERT_EQ(encoder.GetScratchFPRegistersCount(), 0);
    for (auto reg : regs) {
        encoder.ReleaseScratchRegister(reg);
    }
    ASSERT_EQ(encoder.GetScratchFPRegistersCount(), initialFpCount);

    {
        ScopedTmpRegRef reg(&encoder);
        ASSERT_EQ(encoder.GetScratchRegistersCount(), initialCount - 1);
        ASSERT_EQ(encoder.GetScratchFPRegistersCount(), initialFpCount);
        if (encoder.GetScratchRegistersCount() != 0) {
            ScopedTmpRegU32 reg2(&encoder);
            ASSERT_EQ(encoder.GetScratchRegistersCount(), initialCount - 2U);
        }
        {
            ScopedTmpReg reg2(&encoder, floatType);
            ASSERT_EQ(encoder.GetScratchFPRegistersCount(), initialFpCount - 1);
            ASSERT_EQ(encoder.GetScratchRegistersCount(), initialCount - 1);
        }
        ASSERT_EQ(encoder.GetScratchFPRegistersCount(), initialFpCount);
    }
    ASSERT_EQ(encoder.GetScratchRegistersCount(), initialCount);
    ASSERT_EQ(encoder.GetScratchFPRegistersCount(), initialFpCount);
}
}  // namespace ark::compiler
