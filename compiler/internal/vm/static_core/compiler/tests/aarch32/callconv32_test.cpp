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

#include "libarkbase/mem/code_allocator.h"
#include "libarkbase/mem/pool_manager.h"
#include "target/aarch32/target.h"
#include "libarkbase/mem/base_mem_stats.h"

namespace ark::compiler {
class Callconv32Test : public ::testing::Test {
public:
    Callconv32Test()
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        ark::mem::MemConfig::Initialize(64_MB, 64_MB, 64_MB, 32_MB, 0, 0);
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
        encoder_ = Encoder::Create(allocator_, Arch::AARCH32, false);
        encoder_->InitMasm();
        regfile_ = RegistersDescription::Create(allocator_, Arch::AARCH32);
        callconv_ = CallingConvention::Create(allocator_, encoder_, regfile_, Arch::AARCH32);
        memStats_ = new BaseMemStats();
        codeAlloc_ = new (std::nothrow) CodeAllocator(memStats_);
    }
    ~Callconv32Test() override
    {
        Logger::Destroy();
        encoder_->~Encoder();
        delete allocator_;
        delete codeAlloc_;
        delete memStats_;
        PoolManager::Finalize();
        ark::mem::MemConfig::Finalize();
    }

    NO_COPY_SEMANTIC(Callconv32Test);
    NO_MOVE_SEMANTIC(Callconv32Test);

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

#if (PANDA_TARGET_ARM32_ABI_HARD)
#define FLOAT_PARAM_TYPE FLOAT32_TYPE
#define DOUBLE_PARAM_TYPE FLOAT64_TYPE
#else
#define FLOAT_PARAM_TYPE INT32_TYPE
#define DOUBLE_PARAM_TYPE INT64_TYPE
#endif

    // int8_t uin64_t int8_t  int64_t         int8_t int8_t in32_t int64_t
    //   r0   r2+r3   stack0  stack2(align)   stack4 stack5 stack6 stack8(align)
    //      r1            stack1                                 stack7 - missed by align
    void CheckMissesDueAlign()
    {
        auto paramInfo = GetCallconv()->GetParameterInfo(0);
        auto ret = paramInfo->GetNativeParam(INT8_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, INT8_TYPE));

        ret = paramInfo->GetNativeParam(INT64_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 2U);
        EXPECT_EQ(std::get<Reg>(ret), Reg(2U, INT64_TYPE));

        ret = paramInfo->GetNativeParam(INT8_TYPE);
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 0);

        ret = paramInfo->GetNativeParam(INT64_TYPE);
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 2U);

        ret = paramInfo->GetNativeParam(INT8_TYPE);
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 4U);

        ret = paramInfo->GetNativeParam(INT8_TYPE);
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 5U);

        ret = paramInfo->GetNativeParam(INT32_TYPE);
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 6U);

        ret = paramInfo->GetNativeParam(INT64_TYPE);
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 8U);
    }

    //        int32_t float  int64_t    double     float
    // hfloat   r0     s0!    r2+r3    d1(s2+s3)!   s4!
    // sfloat   r0     r1!    r2+r3    stack0+1 !  stack2!
    void CheckMixHfloatSfloat1()
    {
        auto paramInfo = GetCallconv()->GetParameterInfo(0);
        auto ret = paramInfo->GetNativeParam(INT32_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, INT32_TYPE));

        ret = paramInfo->GetNativeParam(FLOAT32_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
#if (PANDA_TARGET_ARM32_ABI_HARD)
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, FLOAT32_TYPE));
#else
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 1);
        EXPECT_EQ(std::get<Reg>(ret), Reg(1, INT32_TYPE));
#endif

        ret = paramInfo->GetNativeParam(INT64_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 2U);
        EXPECT_EQ(std::get<Reg>(ret), Reg(2U, INT64_TYPE));

        ret = paramInfo->GetNativeParam(FLOAT64_TYPE);
#if (PANDA_TARGET_ARM32_ABI_HARD)
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 2U);
        EXPECT_EQ(std::get<Reg>(ret), Reg(2U, FLOAT64_TYPE));
#else
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 0);
#endif

        ret = paramInfo->GetNativeParam(FLOAT32_TYPE);
#if (PANDA_TARGET_ARM32_ABI_HARD)
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 4U);
        EXPECT_EQ(std::get<Reg>(ret), Reg(4U, FLOAT32_TYPE));
#else
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 2U);
#endif
    }

    static void CheckMixHfloatSfloatSlotsPart1(ParameterInfo *paramInfo)
    {
        auto ret = paramInfo->GetNativeParam(FLOAT32_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, FLOAT_PARAM_TYPE));

        ret = paramInfo->GetNativeParam(INT64_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
#if (PANDA_TARGET_ARM32_ABI_HARD)
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, INT64_TYPE));
#else
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 2U);
        EXPECT_EQ(std::get<Reg>(ret), Reg(2U, INT64_TYPE));
#endif

        ret = paramInfo->GetNativeParam(FLOAT32_TYPE);
#if (PANDA_TARGET_ARM32_ABI_HARD)
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 1);
        EXPECT_EQ(std::get<Reg>(ret), Reg(1, FLOAT32_TYPE));
#else
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 0);
#endif

        ret = paramInfo->GetNativeParam(INT32_TYPE);
#if (PANDA_TARGET_ARM32_ABI_HARD)
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 2U);
        EXPECT_EQ(std::get<Reg>(ret), Reg(2U, INT32_TYPE));
#else
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 1);
#endif
    }

    //         float int64_t float int32_t float double int8_t
    // hfloat   s0    r0+r1   s1     r2     s2    s4(d2) r3
    // sfloat   r0    r2+r3  slot0  slot1  slot2 slot4+5 slot6
    void BigCheckMixHfloatSfloatSlots()
    {
        auto paramInfo = GetCallconv()->GetParameterInfo(0);
        CheckMixHfloatSfloatSlotsPart1(paramInfo);

        // Check MixHfloatSfloatSlots part 2
        auto ret = paramInfo->GetNativeParam(FLOAT32_TYPE);
#if (PANDA_TARGET_ARM32_ABI_HARD)
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 2U);
        EXPECT_EQ(std::get<Reg>(ret), Reg(2U, FLOAT32_TYPE));
#else
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 2U);
#endif

        ret = paramInfo->GetNativeParam(FLOAT64_TYPE);
#if (PANDA_TARGET_ARM32_ABI_HARD)
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 4U);
        EXPECT_EQ(std::get<Reg>(ret), Reg(4U, FLOAT64_TYPE));
#else
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 4U);
#endif

        ret = paramInfo->GetNativeParam(INT8_TYPE);
#if (PANDA_TARGET_ARM32_ABI_HARD)
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 3U);
        EXPECT_EQ(std::get<Reg>(ret), Reg(3U, INT8_TYPE));
#else
        EXPECT_TRUE(std::holds_alternative<uint8_t>(ret));
        EXPECT_EQ(std::get<uint8_t>(ret), 6U);
#endif
    }

    void CheckUintParams()
    {
        // 4 uint8_t params - in registers
        {
            ArenaVector<TypeInfo> tmp(GetAllocator()->Adapter());
            auto paramInfo = GetCallconv()->GetParameterInfo(0);
            auto ret = paramInfo->GetNativeParam(INT8_TYPE);
            EXPECT_TRUE(std::holds_alternative<Reg>(ret));
            EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
            EXPECT_EQ(std::get<Reg>(ret), Reg(0, INT8_TYPE));

            for (uint32_t i = 1; i <= 3U; ++i) {
                ret = paramInfo->GetNativeParam(INT8_TYPE);
                EXPECT_TRUE(std::holds_alternative<Reg>(ret));
                EXPECT_EQ(std::get<Reg>(ret).GetId(), i);
                EXPECT_EQ(std::get<Reg>(ret), Reg(i, INT8_TYPE));
            }
        }

        // 4 uint32_t params - in registers
        {
            auto paramInfo = GetCallconv()->GetParameterInfo(0);
            auto ret = paramInfo->GetNativeParam(INT32_TYPE);
            EXPECT_TRUE(std::holds_alternative<Reg>(ret));
            EXPECT_EQ(std::get<Reg>(ret), Reg(0, INT32_TYPE));
            EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);

            for (uint32_t i = 1; i <= 3U; ++i) {
                ret = paramInfo->GetNativeParam(INT32_TYPE);
                EXPECT_TRUE(std::holds_alternative<Reg>(ret));
                EXPECT_EQ(std::get<Reg>(ret), Reg(i, INT32_TYPE));
                EXPECT_EQ(std::get<Reg>(ret).GetId(), i);
            }
        }

        // 2 uint64_t params - in registers
        {
            auto paramInfo = GetCallconv()->GetParameterInfo(0);
            auto ret = paramInfo->GetNativeParam(INT64_TYPE);
            EXPECT_TRUE(std::holds_alternative<Reg>(ret));
            EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
            EXPECT_EQ(std::get<Reg>(ret), Reg(0, INT64_TYPE));

            ret = paramInfo->GetNativeParam(INT64_TYPE);
            EXPECT_TRUE(std::holds_alternative<Reg>(ret));
            EXPECT_EQ(std::get<Reg>(ret).GetId(), 2U);
            EXPECT_EQ(std::get<Reg>(ret), Reg(2U, INT64_TYPE));
        }
    }

private:
    ArenaAllocator *allocator_ {nullptr};
    Encoder *encoder_ {nullptr};
    RegistersDescription *regfile_ {nullptr};
    CallingConvention *callconv_ {nullptr};
    CodeAllocator *codeAlloc_ {nullptr};
    BaseMemStats *memStats_ {nullptr};
};

TEST_F(Callconv32Test, NativeParams)
{
    // Test for
    // std::variant<Reg, uint8_t> GetNativeParam(const TypeInfo& type)

    CheckUintParams();

    // 4 float params - in registers
    {
        auto paramInfo = GetCallconv()->GetParameterInfo(0);
        auto ret = paramInfo->GetNativeParam(FLOAT32_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, FLOAT_PARAM_TYPE));

        for (uint32_t i = 1; i <= 3U; ++i) {
            ret = paramInfo->GetNativeParam(FLOAT32_TYPE);
            EXPECT_TRUE(std::holds_alternative<Reg>(ret));
            EXPECT_EQ(std::get<Reg>(ret).GetId(), i);
            EXPECT_EQ(std::get<Reg>(ret), Reg(i, FLOAT_PARAM_TYPE));
        }
    }

    // 2 double params - in registers
    {
        auto paramInfo = GetCallconv()->GetParameterInfo(0);
        auto ret = paramInfo->GetNativeParam(FLOAT64_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, DOUBLE_PARAM_TYPE));

        ret = paramInfo->GetNativeParam(FLOAT64_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 2U);
        EXPECT_EQ(std::get<Reg>(ret), Reg(2U, DOUBLE_PARAM_TYPE));
    }

    CheckMissesDueAlign();

    CheckMixHfloatSfloat1();

    BigCheckMixHfloatSfloatSlots();
}

class Callconv32ParamTypedTest : public Callconv32Test, public testing::WithParamInterface<TypeInfo> {};

TEST_P(Callconv32ParamTypedTest, NativeParamsAndTmpRegs)
{
    // no tmp registers in param registers
    auto param_info = GetCallconv()->GetParameterInfo(0);
    const auto &target = GetEncoder()->GetTarget();
    auto getTempRegsMaskByType = [&target](Reg &reg) {
        return reg.IsFloat() ? target.GetTempVRegsMask() : target.GetTempRegsMask();
    };

    while (true) {
        auto ret = param_info->GetNativeParam(GetParam());
        if (!std::holds_alternative<Reg>(ret)) {
            break;
        }
        auto &reg = std::get<Reg>(ret);
        EXPECT_FALSE(getTempRegsMaskByType(reg).Test(reg.GetId()));
    }
}

INSTANTIATE_TEST_SUITE_P(AllTypes, Callconv32ParamTypedTest,
                         testing::Values(INT8_TYPE, INT16_TYPE, INT32_TYPE, INT64_TYPE, FLOAT32_TYPE, FLOAT64_TYPE));
}  // namespace ark::compiler
