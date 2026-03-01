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
#include "target/amd64/target.h"
#include "mem/base_mem_stats.h"

const uint64_t SEED = 0x1234;
#ifndef PANDA_NIGHTLY_TEST_ON
const uint64_t ITERATION = 40;
#else
const uint64_t ITERATION = 0xffffff;
#endif
static inline auto random_gen = std::mt19937_64(SEED);

namespace panda::compiler {
class Callconv64Test : public ::testing::Test {
public:
    Callconv64Test()
    {
        panda::mem::MemConfig::Initialize(64_MB, 64_MB, 64_MB, 32_MB);
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
        encoder_ = Encoder::Create(allocator_, Arch::X86_64, false);
        encoder_->InitMasm();
        regfile_ = RegistersDescription::Create(allocator_, Arch::X86_64);
        callconv_ = CallingConvention::Create(allocator_, encoder_, regfile_, Arch::X86_64);
        mem_stats_ = new BaseMemStats();
        code_alloc_ = new (std::nothrow) CodeAllocator(mem_stats_);
    }
    ~Callconv64Test()
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

TEST_F(Callconv64Test, NativeParams)
{
    // Test for
    // std::variant<Reg, uint8_t> GetNativeParam(const ArenaVector<TypeInfo>& reg_list,
    //                                           const TypeInfo& type)

    Target target(Arch::X86_64);
    // 6 uint8_t params - in registers
    {
        auto param_info = GetCallconv()->GetParameterInfo(0);
        auto ret = param_info->GetNativeParam(INT8_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), target.GetParamRegId(0));
        EXPECT_EQ(std::get<Reg>(ret), Reg(target.GetParamRegId(0), INT8_TYPE));

        for (uint32_t i = 1; i <= 5; ++i) {
            ret = param_info->GetNativeParam(INT8_TYPE);
            EXPECT_TRUE(std::holds_alternative<Reg>(ret));
            EXPECT_EQ(std::get<Reg>(ret).GetId(), target.GetParamRegId(i));
            EXPECT_EQ(std::get<Reg>(ret), Reg(target.GetParamRegId(i), INT8_TYPE));
        }
    }

    // 6 uint32_t params - in registers
    {
        auto param_info = GetCallconv()->GetParameterInfo(0);
        auto ret = param_info->GetNativeParam(INT32_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), target.GetParamRegId(0));
        EXPECT_EQ(std::get<Reg>(ret), Reg(target.GetParamRegId(0), INT32_TYPE));

        for (uint32_t i = 1; i <= 5; ++i) {
            ret = param_info->GetNativeParam(INT32_TYPE);
            EXPECT_TRUE(std::holds_alternative<Reg>(ret));
            EXPECT_EQ(std::get<Reg>(ret).GetId(), target.GetParamRegId(i));
            EXPECT_EQ(std::get<Reg>(ret), Reg(target.GetParamRegId(i), INT32_TYPE));
        }
    }

    // 6 uint64_t params - in registers
    {
        auto param_info = GetCallconv()->GetParameterInfo(0);
        auto ret = param_info->GetNativeParam(INT64_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), target.GetParamRegId(0));
        EXPECT_EQ(std::get<Reg>(ret), Reg(target.GetParamRegId(0), INT64_TYPE));

        for (uint32_t i = 1; i <= 5; ++i) {
            ret = param_info->GetNativeParam(INT64_TYPE);
            EXPECT_TRUE(std::holds_alternative<Reg>(ret));
            EXPECT_EQ(std::get<Reg>(ret).GetId(), target.GetParamRegId(i));
            EXPECT_EQ(std::get<Reg>(ret), Reg(target.GetParamRegId(i), INT64_TYPE));
        }
    }

    // 8 float params - in registers
    {
        auto param_info = GetCallconv()->GetParameterInfo(0);
        auto ret = param_info->GetNativeParam(FLOAT32_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, FLOAT32_TYPE));

        for (uint32_t i = 1; i <= 7; ++i) {
            ret = param_info->GetNativeParam(FLOAT32_TYPE);
            EXPECT_TRUE(std::holds_alternative<Reg>(ret));
            EXPECT_EQ(std::get<Reg>(ret).GetId(), i);
            EXPECT_EQ(std::get<Reg>(ret), Reg(i, FLOAT32_TYPE));
        }
    }

    // 8 double params - in registers
    {
        auto param_info = GetCallconv()->GetParameterInfo(0);
        auto ret = param_info->GetNativeParam(FLOAT64_TYPE);
        EXPECT_TRUE(std::holds_alternative<Reg>(ret));
        EXPECT_EQ(std::get<Reg>(ret).GetId(), 0);
        EXPECT_EQ(std::get<Reg>(ret), Reg(0, FLOAT64_TYPE));

        for (uint32_t i = 1; i <= 7; ++i) {
            ret = param_info->GetNativeParam(FLOAT64_TYPE);
            EXPECT_TRUE(std::holds_alternative<Reg>(ret));
            EXPECT_EQ(std::get<Reg>(ret).GetId(), i);
            EXPECT_EQ(std::get<Reg>(ret), Reg(i, FLOAT64_TYPE));
        }
    }
}
}  // namespace panda::compiler
