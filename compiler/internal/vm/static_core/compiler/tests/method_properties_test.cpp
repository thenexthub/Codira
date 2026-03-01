/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "unit_test.h"
#include "optimizer/code_generator/codegen.h"
#include "optimizer/code_generator/method_properties.h"

namespace ark::compiler {
class MethodPropertiesTest : public GraphTest {
public:
    void CheckCall(Opcode opcode)
    {
        GRAPH(GetGraph())
        {
            PARAMETER(0U, 0U).ref();
            BASIC_BLOCK(2U, -1L)
            {
                INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
                INST(2U, Opcode::NullCheck).ref().Inputs(0U, 1U);
                INST(5U, Opcode::SaveState).NoVregs();
                INST(3U, opcode).v0id().Inputs({{DataType::REFERENCE, 2U}, {DataType::NO_TYPE, 5U}});
                INST(4U, Opcode::ReturnVoid).v0id();
            }
        }
        MethodProperties props(GetGraph());
        EXPECT_TRUE(props.GetHasCalls());
        EXPECT_TRUE(props.GetHasRuntimeCalls());
        EXPECT_TRUE(props.GetHasRequireState());
        EXPECT_FALSE(props.GetHasSafepoints());
        EXPECT_TRUE(props.GetCanThrow());  // due to null check
    }
};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(MethodPropertiesTest, EmptyBlock)
{
    GetGraph()->CreateStartBlock();
    MethodProperties props(GetGraph());
    EXPECT_FALSE(props.GetHasCalls());
    EXPECT_FALSE(props.GetHasRuntimeCalls());
    EXPECT_FALSE(props.GetHasRequireState());
    EXPECT_FALSE(props.GetHasSafepoints());
    EXPECT_FALSE(props.GetCanThrow());
}

TEST_F(MethodPropertiesTest, SimpleMethod)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(1U, 1U).u32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Add).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }
    MethodProperties props(GetGraph());
    EXPECT_FALSE(props.GetHasCalls());
    EXPECT_FALSE(props.GetHasRuntimeCalls());
    EXPECT_FALSE(props.GetHasRequireState());
    EXPECT_FALSE(props.GetHasSafepoints());
    EXPECT_FALSE(props.GetCanThrow());
}

TEST_F(MethodPropertiesTest, SafePoint)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(1U, 1U).u32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Add).u32().Inputs(0U, 1U);
            INST(3U, Opcode::SafePoint).Inputs(2U).SrcVregs({0U});
            INST(4U, Opcode::Return).u32().Inputs(2U);
        }
    }
    MethodProperties props(GetGraph());
    EXPECT_FALSE(props.GetHasCalls());
    EXPECT_TRUE(props.GetHasRuntimeCalls());
    EXPECT_FALSE(props.GetHasRequireState());
    EXPECT_TRUE(props.GetHasSafepoints());
    EXPECT_FALSE(props.GetCanThrow());
}

TEST_F(MethodPropertiesTest, StaticCall)
{
    CheckCall(Opcode::CallStatic);
}

TEST_F(MethodPropertiesTest, VirtualCall)
{
    CheckCall(Opcode::CallVirtual);
}

TEST_F(MethodPropertiesTest, Intrinsic)
{
    CheckCall(Opcode::Intrinsic);
}

TEST_F(MethodPropertiesTest, Builtin)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).f64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Builtin).b().Inputs({{DataType::FLOAT64, 0U}});
            INST(2U, Opcode::Return).b().Inputs(1U);
        }
    }
    MethodProperties props(GetGraph());
    EXPECT_TRUE(props.GetHasCalls());
    EXPECT_FALSE(props.GetHasRuntimeCalls());
    EXPECT_FALSE(props.GetHasRequireState());
    EXPECT_FALSE(props.GetHasSafepoints());
    EXPECT_FALSE(props.GetCanThrow());
}

TEST_F(MethodPropertiesTest, SaveState)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(0U, Opcode::SaveState).NoVregs();
            INST(1U, Opcode::LoadString).ref().Inputs(0U).TypeId(42U);
            INST(2U, Opcode::Return).ref().Inputs(1U);
        }
    }
    MethodProperties props(GetGraph());
    EXPECT_FALSE(props.GetHasCalls());
    EXPECT_TRUE(props.GetHasRuntimeCalls());
    EXPECT_TRUE(props.GetHasRequireState());
    EXPECT_FALSE(props.GetHasSafepoints());
    EXPECT_TRUE(props.GetCanThrow());
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
