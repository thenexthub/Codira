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

#include "mem/pool_manager.h"
#include "target/aarch32/target.h"

const uint64_t SEED = 0x1234;
#ifndef PANDA_NIGHTLY_TEST_ON
const uint64_t ITERATION = 20;
#else
const uint64_t ITERATION = 0xffffff;
#endif
static inline auto random_gen = std::mt19937_64(SEED);

namespace panda::compiler {
class Register32Test : public ::testing::Test {
public:
    Register32Test()
    {
        panda::mem::MemConfig::Initialize(64_MB, 64_MB, 64_MB, 32_MB);
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
    }
    ~Register32Test()
    {
        delete allocator_;
        PoolManager::Finalize();
        panda::mem::MemConfig::Finalize();
    }

    ArenaAllocator *GetAllocator()
    {
        return allocator_;
    }

private:
    ArenaAllocator *allocator_;
};

TEST_F(Register32Test, TmpReg)
{
    aarch32::Aarch32Encoder encoder(GetAllocator());
    encoder.InitMasm();

    auto FloatType = FLOAT32_TYPE;

    if (encoder.GetScratchFPRegistersCount() == 0) {
        encoder.GetMasm()->GetScratchVRegisterList()->Combine(vixl::aarch32::SRegister(1));
    }

    auto initial_count = encoder.GetScratchRegistersCount();
    auto initial_fp_count = encoder.GetScratchFPRegistersCount();
    ASSERT_NE(initial_count, 0);
    ASSERT_NE(initial_fp_count, 0);

    std::vector<Reg> regs;
    for (size_t i = 0; i < initial_count; i++) {
        regs.push_back(encoder.AcquireScratchRegister(INT64_TYPE));
    }
    ASSERT_EQ(encoder.GetScratchRegistersCount(), 0);
    ASSERT_EQ(encoder.GetScratchFPRegistersCount(), initial_fp_count);
    for (auto reg : regs) {
        encoder.ReleaseScratchRegister(reg);
    }
    ASSERT_EQ(encoder.GetScratchRegistersCount(), initial_count);

    regs.clear();
    for (size_t i = 0; i < initial_fp_count; i++) {
        regs.push_back(encoder.AcquireScratchRegister(FloatType));
    }

    ASSERT_EQ(encoder.GetScratchRegistersCount(), initial_count);
    ASSERT_EQ(encoder.GetScratchFPRegistersCount(), 0);
    for (auto reg : regs) {
        encoder.ReleaseScratchRegister(reg);
    }
    ASSERT_EQ(encoder.GetScratchFPRegistersCount(), initial_fp_count);

    {
        ScopedTmpRegRef reg(&encoder);
        ASSERT_EQ(encoder.GetScratchRegistersCount(), initial_count - 1);
        ASSERT_EQ(encoder.GetScratchFPRegistersCount(), initial_fp_count);
        if (encoder.GetScratchRegistersCount() != 0) {
            ScopedTmpRegU32 reg2(&encoder);
            ASSERT_EQ(encoder.GetScratchRegistersCount(), initial_count - 2);
        }
        {
            ScopedTmpReg reg2(&encoder, FloatType);
            ASSERT_EQ(encoder.GetScratchFPRegistersCount(), initial_fp_count - 1);
            ASSERT_EQ(encoder.GetScratchRegistersCount(), initial_count - 1);
        }
        ASSERT_EQ(encoder.GetScratchFPRegistersCount(), initial_fp_count);
    }
    ASSERT_EQ(encoder.GetScratchRegistersCount(), initial_count);
    ASSERT_EQ(encoder.GetScratchFPRegistersCount(), initial_fp_count);
}
}  // namespace panda::compiler
