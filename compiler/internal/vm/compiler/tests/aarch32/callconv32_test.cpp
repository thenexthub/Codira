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

#include "mem/code_allocator.h"
#include "mem/pool_manager.h"
#include "target/aarch32/target.h"
#include "mem/base_mem_stats.h"

const uint64_t SEED = 0x1234;
#ifndef PANDA_NIGHTLY_TEST_ON
const uint64_t ITERATION = 40;
#else
const uint64_t ITERATION = 0xffffff;
#endif
static inline auto random_gen = std::mt19937_64(SEED);

namespace panda::compiler {
class Callconv32Test : public ::testing::Test {
public:
    Callconv32Test()
    {
        panda::mem::MemConfig::Initialize(64_MB, 64_MB, 64_MB, 32_MB);
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
        encoder_ = Encoder::Create(allocator_, Arch::AARCH32, false);
        encoder_->InitMasm();
        regfile_ = RegistersDescription::Create(allocator_, Arch::AARCH32);
        callconv_ = CallingConvention::Create(allocator_, encoder_, regfile_, Arch::AARCH32);
        mem_stats_ = new BaseMemStats();
        code_alloc_ = new (std::nothrow) CodeAllocator(mem_stats_);
    }
    ~Callconv32Test()
    {
        Logger::Destroy();
        encoder_->~Encoder();
        delete allocator_;
        delete code_alloc_;
        delete mem_stats_;
        PoolManager::Finalize();
        panda::mem::MemConfig::Finalize();
    }

    ArenaAllocator *GetAllocator()
    {
        return allocator_;
    }

    Encoder *GetEncoder()
    {
        return encoder_;
    }

    RegistersDescription *GetRegfile()
    {
        return regfile_;
    }

    CallingConvention *GetCallconv()
    {
        return callconv_;
    }

private:
    ArenaAllocator *allocator_ {nullptr};
    Encoder *encoder_ {nullptr};
    RegistersDescription *regfile_ {nullptr};
    CallingConvention *callconv_ {nullptr};
    CodeAllocator *code_alloc_ {nullptr};
    BaseMemStats *mem_stats_ {nullptr};
};

#if (PANDA_TARGET_ARM32_ABI_HARD)
#define FLOAT_PARAM_TYPE FLOAT32_TYPE
#define DOUBLE_PARAM_TYPE FLOAT64_TYPE
#else
#define FLOAT_PARAM_TYPE INT32_TYPE
#define DOUBLE_PARAM_TYPE INT64_TYPE
#endif

TEST_F(Callconv32Test, NativeParams)
{
    // Test for
    // std::variant<Reg, uint8_t> GetNativeParam(const TypeInfo& type)

    // 4 uint8_t params - in registers
    {
        ArenaVector<TypeInfo> tmp(GetAllocator()->Adapter());
        auto param_info = GetCallconv()->GetParameterInfo(0);
        auto ret = param_info->GetNativeParam(INT8_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, INT8_TYPE));

        for (uint32_t i = 1; i <= 3; ++i) {
            ret = param_info->GetNativeParam(INT8_TYPE);
            EXPECT_TRUE(std::holds_alternative<Reg>(ret));
            EXPECT_EQ(std::get<Reg>(ret).GetId(), i);
            EXPECT_EQ(std::get<Reg>(ret), Reg(i, INT8_TYPE));
        }
    }

    // 4 uint32_t params - in registers
    {
        auto param_info = GetCallconv()->GetParameterInfo(0);
        auto ret = param_info->GetNativeParam(INT32_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, INT32_TYPE));

        for (uint32_t i = 1; i <= 3; ++i) {
            ret = param_info->GetNativeParam(INT32_TYPE);
            EXPECT_TRUE(std::holds_alternative<Reg>(ret));
            EXPECT_EQ(std::get<Reg>(ret).GetId(), i);
            EXPECT_EQ(std::get<Reg>(ret), Reg(i, INT32_TYPE));
        }
    }

    // 2 uint64_t params - in registers
    {
        auto param_info = GetCallconv()->GetParameterInfo(0);
        auto ret = param_info->GetNativeParam(INT64_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, INT64_TYPE));

        ret = param_info->GetNativeParam(INT64_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 2);
        EXPECT_EQ(std::get<Reg>(ret), Reg(2, INT64_TYPE));
    }

    // 4 float params - in registers
    {
        auto param_info = GetCallconv()->GetParameterInfo(0);
        auto ret = param_info->GetNativeParam(FLOAT32_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, FLOAT_PARAM_TYPE));

        for (uint32_t i = 1; i <= 3; ++i) {
            ret = param_info->GetNativeParam(FLOAT32_TYPE);
            EXPECT_TRUE(std::holds_alternative<Reg>(ret));
            EXPECT_EQ(std::get<Reg>(ret).GetId(), i);
            EXPECT_EQ(std::get<Reg>(ret), Reg(i, FLOAT_PARAM_TYPE));
        }
    }

    // 2 double params - in registers
    {
        auto param_info = GetCallconv()->GetParameterInfo(0);
        auto ret = param_info->GetNativeParam(FLOAT64_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, DOUBLE_PARAM_TYPE));

        ret = param_info->GetNativeParam(FLOAT64_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 2);
        EXPECT_EQ(std::get<Reg>(ret), Reg(2, DOUBLE_PARAM_TYPE));
    }

    // int8_t uin64_t int8_t  int64_t         int8_t int8_t in32_t int64_t
    //   r0   r2+r3   stack0  stack2(align)   stack4 stack5 stack6 stack8(align)
    //      r1            stack1                                 stack7 - missed by align
    {
        auto param_info = GetCallconv()->GetParameterInfo(0);
        auto ret = param_info->GetNativeParam(INT8_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, INT8_TYPE));

        ret = param_info->GetNativeParam(INT64_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 2);
        EXPECT_EQ(std::get<Reg>(ret), Reg(2, INT64_TYPE));

        ret = param_info->GetNativeParam(INT8_TYPE);
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 0);

        ret = param_info->GetNativeParam(INT64_TYPE);
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 2);

        ret = param_info->GetNativeParam(INT8_TYPE);
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 4);

        ret = param_info->GetNativeParam(INT8_TYPE);
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 5);

        ret = param_info->GetNativeParam(INT32_TYPE);
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 6);

        ret = param_info->GetNativeParam(INT64_TYPE);
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 8);
    }

    //        int32_t float  int64_t    double     float
    // hfloat   r0     s0!    r2+r3    d1(s2+s3)!   s4!
    // sfloat   r0     r1!    r2+r3    stack0+1 !  stack2!
    {
        auto param_info = GetCallconv()->GetParameterInfo(0);
        auto ret = param_info->GetNativeParam(INT32_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, INT32_TYPE));

        ret = param_info->GetNativeParam(FLOAT32_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
#if (PANDA_TARGET_ARM32_ABI_HARD)
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, FLOAT32_TYPE));
#else
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 1);
        EXPECT_EQ(std::get<Reg>(ret), Reg(1, INT32_TYPE));
#endif

        ret = param_info->GetNativeParam(INT64_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 2);
        EXPECT_EQ(std::get<Reg>(ret), Reg(2, INT64_TYPE));

        ret = param_info->GetNativeParam(FLOAT64_TYPE);
#if (PANDA_TARGET_ARM32_ABI_HARD)
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 2);
        EXPECT_EQ(std::get<Reg>(ret), Reg(2, FLOAT64_TYPE));
#else
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 0);
#endif

        ret = param_info->GetNativeParam(FLOAT32_TYPE);
#if (PANDA_TARGET_ARM32_ABI_HARD)
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 4);
        EXPECT_EQ(std::get<Reg>(ret), Reg(4, FLOAT32_TYPE));
#else
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 2);
#endif
    }

    //         float int64_t float int32_t float double int8_t
    // hfloat   s0    r0+r1   s1     r2     s2    s4(d2) r3
    // sfloat   r0    r2+r3  slot0  slot1  slot2 slot4+5 slot6
    {
        auto param_info = GetCallconv()->GetParameterInfo(0);
        auto ret = param_info->GetNativeParam(FLOAT32_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, FLOAT_PARAM_TYPE));

        ret = param_info->GetNativeParam(INT64_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
#if (PANDA_TARGET_ARM32_ABI_HARD)
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, INT64_TYPE));
#else
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 2);
        EXPECT_EQ(std::get<Reg>(ret), Reg(2, INT64_TYPE));
#endif

        ret = param_info->GetNativeParam(FLOAT32_TYPE);
#if (PANDA_TARGET_ARM32_ABI_HARD)
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 1);
        EXPECT_EQ(std::get<Reg>(ret), Reg(1, FLOAT32_TYPE));
#else
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 0);
#endif

        ret = param_info->GetNativeParam(INT32_TYPE);
#if (PANDA_TARGET_ARM32_ABI_HARD)
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 2);
        EXPECT_EQ(std::get<Reg>(ret), Reg(2, INT32_TYPE));
#else
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 1);
#endif

        ret = param_info->GetNativeParam(FLOAT32_TYPE);
#if (PANDA_TARGET_ARM32_ABI_HARD)
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 2);
        EXPECT_EQ(std::get<Reg>(ret), Reg(2, FLOAT32_TYPE));
#else
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 2);
#endif

        ret = param_info->GetNativeParam(FLOAT64_TYPE);
#if (PANDA_TARGET_ARM32_ABI_HARD)
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 4);
        EXPECT_EQ(std::get<Reg>(ret), Reg(4, FLOAT64_TYPE));
#else
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 4);
#endif

        ret = param_info->GetNativeParam(INT8_TYPE);
#if (PANDA_TARGET_ARM32_ABI_HARD)
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 3);
        EXPECT_EQ(std::get<Reg>(ret), Reg(3, INT8_TYPE));
#else
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 6);
#endif
    }
}
}  // namespace panda::compiler
