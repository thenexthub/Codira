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

#include "libarkbase/macros.h"
#include "libarkbase/mem/code_allocator.h"
#include "libarkbase/mem/pool_manager.h"
#include "target/amd64/target.h"
#include "libarkbase/mem/base_mem_stats.h"

namespace ark::compiler {
class Callconv64Test : public ::testing::Test {
public:
    Callconv64Test()
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        ark::mem::MemConfig::Initialize(64_MB, 64_MB, 64_MB, 32_MB, 0, 0);
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
        encoder_ = Encoder::Create(allocator_, Arch::X86_64, false);
        encoder_->InitMasm();
        regfile_ = RegistersDescription::Create(allocator_, Arch::X86_64);
        callconv_ = CallingConvention::Create(allocator_, encoder_, regfile_, Arch::X86_64);
        memStats_ = new BaseMemStats();
        codeAlloc_ = new (std::nothrow) CodeAllocator(memStats_);
    }
    ~Callconv64Test() override
    {
        Logger::Destroy();
        encoder_->~Encoder();
        delete allocator_;
        delete codeAlloc_;
        delete memStats_;
        PoolManager::Finalize();
        ark::mem::MemConfig::Finalize();
    }

    NO_COPY_SEMANTIC(Callconv64Test);
    NO_MOVE_SEMANTIC(Callconv64Test);

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
    CodeAllocator *codeAlloc_ {nullptr};
    BaseMemStats *memStats_ {nullptr};
};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(Callconv64Test, NativeParamsSix)
{
    // Test for
    // std::variant<Reg, uint8_t> GetNativeParam(const ArenaVector<TypeInfo>& reg_list,
    //                                           const TypeInfo& type)

    Target target(Arch::X86_64);
    // 6 uint8_t params - in registers
    {
        auto paramInfo = GetCallconv()->GetParameterInfo(0);
        auto ret = paramInfo->GetNativeParam(INT8_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), target.GetParamRegId(0));
        EXPECT_EQ(std::get<Reg>(ret), Reg(target.GetParamRegId(0), INT8_TYPE));

        for (uint32_t i = 1; i <= 5U; ++i) {
            ret = paramInfo->GetNativeParam(INT8_TYPE);
            EXPECT_TRUE(std::holds_alternative<Reg>(ret));
            EXPECT_EQ(std::get<Reg>(ret).GetId(), target.GetParamRegId(i));
            EXPECT_EQ(std::get<Reg>(ret), Reg(target.GetParamRegId(i), INT8_TYPE));
        }
    }

    // 6 uint32_t params - in registers
    {
        auto paramInfo = GetCallconv()->GetParameterInfo(0);
        auto ret = paramInfo->GetNativeParam(INT32_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), target.GetParamRegId(0));
        EXPECT_EQ(std::get<Reg>(ret), Reg(target.GetParamRegId(0), INT32_TYPE));

        for (uint32_t i = 1; i <= 5U; ++i) {
            ret = paramInfo->GetNativeParam(INT32_TYPE);
            EXPECT_TRUE(std::holds_alternative<Reg>(ret));
            EXPECT_EQ(std::get<Reg>(ret).GetId(), target.GetParamRegId(i));
            EXPECT_EQ(std::get<Reg>(ret), Reg(target.GetParamRegId(i), INT32_TYPE));
        }
    }

    // 6 uint64_t params - in registers
    {
        auto paramInfo = GetCallconv()->GetParameterInfo(0);
        auto ret = paramInfo->GetNativeParam(INT64_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), target.GetParamRegId(0));
        EXPECT_EQ(std::get<Reg>(ret), Reg(target.GetParamRegId(0), INT64_TYPE));

        for (uint32_t i = 1; i <= 5U; ++i) {
            ret = paramInfo->GetNativeParam(INT64_TYPE);
            EXPECT_TRUE(std::holds_alternative<Reg>(ret));
            EXPECT_EQ(std::get<Reg>(ret).GetId(), target.GetParamRegId(i));
            EXPECT_EQ(std::get<Reg>(ret), Reg(target.GetParamRegId(i), INT64_TYPE));
        }
    }
}

TEST_F(Callconv64Test, NativeParamsEight)
{
    // 8 float params - in registers
    {
        auto paramInfo = GetCallconv()->GetParameterInfo(0);
        auto ret = paramInfo->GetNativeParam(FLOAT32_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, FLOAT32_TYPE));

        for (uint32_t i = 1; i <= 7U; ++i) {
            ret = paramInfo->GetNativeParam(FLOAT32_TYPE);
            EXPECT_TRUE(std::holds_alternative<Reg>(ret));
            EXPECT_EQ(std::get<Reg>(ret).GetId(), i);
            EXPECT_EQ(std::get<Reg>(ret), Reg(i, FLOAT32_TYPE));
        }
    }

    // 8 double params - in registers
    {
        auto paramInfo = GetCallconv()->GetParameterInfo(0);
        auto ret = paramInfo->GetNativeParam(FLOAT64_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, FLOAT64_TYPE));

        for (uint32_t i = 1; i <= 7U; ++i) {
            ret = paramInfo->GetNativeParam(FLOAT64_TYPE);
            EXPECT_TRUE(std::holds_alternative<Reg>(ret));
            EXPECT_EQ(std::get<Reg>(ret).GetId(), i);
            EXPECT_EQ(std::get<Reg>(ret), Reg(i, FLOAT64_TYPE));
        }
    }
}

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class Callconv64ParamTypedTest : public Callconv64Test, public testing::WithParamInterface<TypeInfo> {};

TEST_P(Callconv64ParamTypedTest, NativeParamsAndTmpRegs)
{
    // no tmp registers in param registers
    auto paramInfo = GetCallconv()->GetParameterInfo(0);
    const auto &target = GetEncoder()->GetTarget();
    auto getTempRegsMaskByType = [&target](Reg &reg) {
        return reg.IsFloat() ? target.GetTempVRegsMask() : target.GetTempRegsMask();
    };

    while (true) {
        auto ret = paramInfo->GetNativeParam(GetParam());
        if (!std::holds_alternative<Reg>(ret)) {
            break;
        }
        auto &reg = std::get<Reg>(ret);
        EXPECT_FALSE(getTempRegsMaskByType(reg).Test(reg.GetId()));
    }
}

INSTANTIATE_TEST_SUITE_P(AllTypes, Callconv64ParamTypedTest,
                         testing::Values(INT8_TYPE, INT16_TYPE, INT32_TYPE, INT64_TYPE, FLOAT32_TYPE, FLOAT64_TYPE));
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
