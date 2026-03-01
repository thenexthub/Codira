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

#include <sys/types.h>
#include "codegen_test.h"
#include "optimizer/ir/inst.h"

namespace ark::compiler {

// NOLINTBEGIN(readability-magic-numbers,modernize-avoid-c-arrays,cppcoreguidelines-pro-bounds-pointer-arithmetic)
TEST_F(CodegenTest, ZeroCheck)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::ZeroCheck).s64().Inputs(0U, 2U);
            INST(4U, Opcode::Div).s64().Inputs(1U, 3U);
            INST(5U, Opcode::Mod).s64().Inputs(1U, 3U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::Add).s64().Inputs(0U, 1U);  // Some return value
            INST(7U, Opcode::Return).s64().Inputs(6U);
        }
    }
    RegAlloc(GetGraph());

    SetNumVirtRegs(GetGraph()->GetVRegsCount());

    EXPECT_TRUE(RunCodegen(GetGraph()));
    auto codeEntry = reinterpret_cast<char *>(GetGraph()->GetCode().Data());
    auto codeExit = codeEntry + GetGraph()->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    // param1 < 0 [OK]
    auto param1 = CutValue<uint64_t>(std::numeric_limits<int64_t>::min(), DataType::INT64);
    auto param2 = CutValue<uint64_t>(std::numeric_limits<int64_t>::max(), DataType::INT64);
    GetExecModule().SetParameter(0U, param1);
    GetExecModule().SetParameter(1U, param2);
    GetExecModule().Execute();
    EXPECT_EQ(GetExecModule().GetRetValue(), param1 + param2);

    // param1 > 0 [OK]
    param1 = CutValue<uint64_t>(std::numeric_limits<int64_t>::max(), DataType::INT64);
    param2 = CutValue<uint64_t>(0U, DataType::INT64);
    GetExecModule().SetParameter(0U, param1);
    GetExecModule().SetParameter(1U, param2);
    GetExecModule().Execute();
    EXPECT_EQ(GetExecModule().GetRetValue(), param1 + param2);

    // param1 == 0 [THROW]
}

SRC_GRAPH(BoundsCheckI, Graph *graph, unsigned index)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();  // array
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::NullCheck).ref().Inputs(0U, 1U);
            INST(3U, Opcode::LenArray).s32().Inputs(2U);
            INST(4U, Opcode::BoundsCheckI).s32().Inputs(3U, 1U).Imm(index);
            INST(5U, Opcode::LoadArrayI).u64().Inputs(2U).Imm(index);
            INST(6U, Opcode::Return).u64().Inputs(5U);
        }
    }
}

TEST_F(CodegenTest, BoundsCheckI)
{
    uint64_t arrayData[4098U];
    for (unsigned i = 0; i < 4098U; i++) {
        arrayData[i] = i;
    }

    for (unsigned index = 4095U; index <= 4097U; index++) {
        auto graph = CreateEmptyGraph();
        src_graph::BoundsCheckI::CREATE(graph, index);

        SetNumVirtRegs(0U);
        SetNumArgs(1U);

        RegAlloc(graph);

        // Run codegen
        EXPECT_TRUE(RunCodegen(graph));
        auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
        auto codeExit = codeEntry + graph->GetCode().Size();
        ASSERT(codeEntry != nullptr && codeExit != nullptr);
        GetExecModule().SetInstructions(codeEntry, codeExit);

        // Enable dumping
        GetExecModule().SetDump(false);

        auto param = GetExecModule().CreateArray(arrayData, index + 1U, GetObjectAllocator());
        GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param));

        GetExecModule().Execute();
        GetExecModule().SetDump(false);
        // End dump

        auto retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, index);

        GetExecModule().FreeArray(param);
    }
}

TEST_F(CodegenTest, MultiplyAddInteger)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-add instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0U, 10U);
        CONSTANT(1U, 42U);
        CONSTANT(2U, 13U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::MAdd).s64().Inputs(0U, 1U, 2U);
            INST(4U, Opcode::Return).s64().Inputs(3U);
        }
    }

    CheckReturnValue(GetGraph(), 433U);
}

TEST_F(CodegenTest, MultiplyAddFloat)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-add instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0U, 10.0_D);
        CONSTANT(1U, 42.0_D);
        CONSTANT(2U, 13.0_D);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::MAdd).f64().Inputs(0U, 1U, 2U);
            INST(4U, Opcode::Return).f64().Inputs(3U);
        }
    }

    CheckReturnValue(GetGraph(), 433.0_D);
}

TEST_F(CodegenTest, MultiplySubtractInteger)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-subtract instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0U, 10U);
        CONSTANT(1U, 42U);
        CONSTANT(2U, 13U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::MSub).s64().Inputs(0U, 1U, 2U);
            INST(4U, Opcode::Return).s64().Inputs(3U);
        }
    }

    CheckReturnValue(GetGraph(), -407L);
}

TEST_F(CodegenTest, MultiplySubtractFloat)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-subtract instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0U, 10.0_D);
        CONSTANT(1U, 42.0_D);
        CONSTANT(2U, 13.0_D);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::MSub).f64().Inputs(0U, 1U, 2U);
            INST(4U, Opcode::Return).f64().Inputs(3U);
        }
    }

    CheckReturnValue(GetGraph(), -407.0_D);
}

TEST_F(CodegenTest, MultiplyNegateInteger)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-negate instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0U, 5U);
        CONSTANT(1U, 5U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::MNeg).s64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s64().Inputs(2U);
        }
    }

    CheckReturnValue(GetGraph(), -25L);
}

TEST_F(CodegenTest, MultiplyNegateFloat)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-negate instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0U, 5.0_D);
        CONSTANT(1U, 5.0_D);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::MNeg).f64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).f64().Inputs(2U);
        }
    }

    CheckReturnValue(GetGraph(), -25.0_D);
}

TEST_F(CodegenTest, OrNot)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-negate instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0x0000beefU);
        CONSTANT(1U, 0x2152ffffU);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::OrNot).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }

    CheckReturnValue(GetGraph(), 0xdeadbeefU);
}

TEST_F(CodegenTest, AndNot)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-negate instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0xf0000003U);
        CONSTANT(1U, 0x1U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::AndNot).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }

    CheckReturnValue(GetGraph(), 0xf0000002U);
}

TEST_F(CodegenTest, XorNot)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-negate instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0xf0f1ffd0U);
        CONSTANT(1U, 0xcf0fc0f1U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::XorNot).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }

    CheckReturnValue(GetGraph(), 0xc001c0deU);
}

template <Opcode OPCODE, uint32_t L, uint32_t R, ShiftType SHIFT_TYPE, uint32_t SHIFT, uint32_t ERV>
void CodegenTest::TestBinaryOperationWithShiftedOperand()
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, L);
        CONSTANT(1U, R);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, OPCODE).Shift(SHIFT_TYPE, SHIFT).u32().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u32().Inputs(2U);
        }
    }

    CheckReturnValue(GetGraph(), ERV);
}

TEST_F(CodegenTest, AddSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "AddSR instruction is only supported on Aarch64";
    }

    TestBinaryOperationWithShiftedOperand<Opcode::AddSR, 10U, 2U, ShiftType::LSL, 1U, 14U>();
}

TEST_F(CodegenTest, SubSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "SubSR instruction is only supported on Aarch64";
    }

    TestBinaryOperationWithShiftedOperand<Opcode::SubSR, 10U, 4U, ShiftType::LSR, 2U, 9U>();
}

TEST_F(CodegenTest, AndSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "AndSR instruction is only supported on Aarch64";
    }

    TestBinaryOperationWithShiftedOperand<Opcode::AndSR, 1U, 1U, ShiftType::LSL, 1U, 0U>();
}

TEST_F(CodegenTest, OrSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "OrSR instruction is only supported on Aarch64";
    }

    TestBinaryOperationWithShiftedOperand<Opcode::OrSR, 1U, 1U, ShiftType::LSL, 1U, 3U>();
}

TEST_F(CodegenTest, XorSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "XorSR instruction is only supported on Aarch64";
    }

    TestBinaryOperationWithShiftedOperand<Opcode::XorSR, 3U, 1U, ShiftType::LSL, 1U, 1U>();
}

TEST_F(CodegenTest, AndNotSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "AndNotSR instruction is only supported on Aarch64";
    }

    TestBinaryOperationWithShiftedOperand<Opcode::AndNotSR, 6U, 12U, ShiftType::LSR, 2U, 4U>();
}

TEST_F(CodegenTest, OrNotSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "OrNotSR instruction is only supported on Aarch64";
    }

    TestBinaryOperationWithShiftedOperand<Opcode::OrNotSR, 1U, 12U, ShiftType::LSR, 2U, 0xfffffffdU>();
}

TEST_F(CodegenTest, XorNotSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "XorNotSR instruction is only supported on Aarch64";
    }

    TestBinaryOperationWithShiftedOperand<Opcode::XorNotSR, static_cast<uint32_t>(-1L), 12U, ShiftType::LSR, 2U, 3U>();
}

TEST_F(CodegenTest, NegSR)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "NegSR instruction is only supported on Aarch64";
    }

    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0x80000000U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::NegSR).Shift(ShiftType::ASR, 1U).u32().Inputs(0U);
            INST(2U, Opcode::Return).u32().Inputs(1U);
        }
    }

    CheckReturnValue(GetGraph(), 0x40000000U);
}

TEST_F(CodegenTest, LoadArrayPairLivenessInfo)
{
    auto graph = GetGraph();

    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::LoadArrayPair).s32().Inputs(0U, 1U);
            INST(4U, Opcode::LoadPairPart).s32().Inputs(2U).Imm(0U);
            INST(5U, Opcode::LoadPairPart).s32().Inputs(2U).Imm(1U);
            INST(12U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(10U, Opcode::LoadClass)
                .ref()
                .Inputs(12U)
                .TypeId(42U)
                .Class(reinterpret_cast<RuntimeInterface::ClassPtr>(1U));
            INST(3U, Opcode::IsInstance).b().Inputs(0U, 10U, 12U).TypeId(42U);
            INST(6U, Opcode::Cast).s32().SrcType(DataType::BOOL).Inputs(3U);
            INST(7U, Opcode::Add).s32().Inputs(4U, 5U);
            INST(8U, Opcode::Add).s32().Inputs(7U, 6U);
            INST(9U, Opcode::Return).s32().Inputs(8U);
        }
    }

    SetNumVirtRegs(0U);
    SetNumArgs(2U);
    RegAlloc(graph);
    EXPECT_TRUE(RunCodegen(graph));

    RegMask ldpRegs {};

    auto cg = Codegen(graph);
    for (auto &bb : graph->GetBlocksLinearOrder()) {
        for (auto inst : bb->AllInsts()) {
            if (inst->GetOpcode() == Opcode::LoadArrayPair) {
                ldpRegs.set(inst->GetDstReg(0U));
                ldpRegs.set(inst->GetDstReg(1U));
            } else if (inst->GetOpcode() == Opcode::IsInstance) {
                auto liveRegs = cg.GetLiveRegisters(inst).first;
                // Both dst registers should be alive during IsInstance call
                ASSERT_EQ(ldpRegs & liveRegs, ldpRegs);
            }
        }
    }
}

TEST_F(CodegenTest, CompareAnyTypeInst)
{
    auto graph = GetGraph();
    graph->SetDynamicMethod();
    graph->SetDynamicStub();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U);
        INS(0U).SetType(DataType::Type::ANY);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::CompareAnyType).b().AnyType(AnyBaseType::UNDEFINED_TYPE).Inputs(0U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }

    SetNumVirtRegs(0U);
    ASSERT_TRUE(RegAlloc(graph));
    ASSERT_TRUE(RunCodegen(graph));

    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();

    ASSERT(codeEntry != nullptr && codeExit != nullptr);

    GetExecModule().SetInstructions(codeEntry, codeExit);
    GetExecModule().SetDump(false);

    GetExecModule().Execute();
    auto rv = GetExecModule().GetRetValue<bool>();
    EXPECT_EQ(rv, true);
}

TEST_F(CodegenTest, CastAnyTypeValueInst)
{
    auto graph = GetGraph();
    graph->SetDynamicMethod();
    graph->SetDynamicStub();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U);
        INS(0U).SetType(DataType::Type::ANY);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::CastAnyTypeValue).b().AnyType(AnyBaseType::UNDEFINED_TYPE).Inputs(0U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }

    SetNumVirtRegs(0U);
    ASSERT_TRUE(RegAlloc(graph));
    ASSERT_TRUE(RunCodegen(graph));

    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();

    ASSERT(codeEntry != nullptr && codeExit != nullptr);

    GetExecModule().SetInstructions(codeEntry, codeExit);
    GetExecModule().SetDump(false);

    GetExecModule().Execute();
    auto rv = GetExecModule().GetRetValue<uint32_t>();
    EXPECT_EQ(rv, 0U);
}

TEST_F(CodegenTest, NegativeCheck)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::NegativeCheck).s64().Inputs(0U, 2U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::Add).s64().Inputs(0U, 1U);  // Some return value
            INST(7U, Opcode::Return).s64().Inputs(6U);
        }
    }
    RegAlloc(GetGraph());

    SetNumVirtRegs(GetGraph()->GetVRegsCount());

    EXPECT_TRUE(RunCodegen(GetGraph()));
    auto codeEntry = reinterpret_cast<char *>(GetGraph()->GetCode().Data());
    auto codeExit = codeEntry + GetGraph()->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    // param1 > 0 [OK]
    auto param1 = CutValue<uint64_t>(std::numeric_limits<int64_t>::max(), DataType::INT64);
    auto param2 = CutValue<uint64_t>(std::numeric_limits<int64_t>::min(), DataType::INT64);
    GetExecModule().SetParameter(0U, param1);
    GetExecModule().SetParameter(1U, param2);
    GetExecModule().Execute();
    EXPECT_EQ(GetExecModule().GetRetValue(), param1 + param2);

    // param1 == 0 [OK]
    param1 = CutValue<uint64_t>(0U, DataType::INT64);
    param2 = CutValue<uint64_t>(std::numeric_limits<int64_t>::max(), DataType::INT64);
    GetExecModule().SetParameter(0U, param1);
    GetExecModule().SetParameter(1U, param2);
    GetExecModule().Execute();
    EXPECT_EQ(GetExecModule().GetRetValue(), param1 + param2);

    // param1 < 0 [THROW]
}

TEST_F(CodegenTest, NullCheckBoundsCheck)
{
    constexpr unsigned ARRAY_LEN = 10;

    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();  // array
        PARAMETER(1U, 1U).u64();  // index
        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::NullCheck).ref().Inputs(0U, 2U);
            INST(4U, Opcode::LenArray).s32().Inputs(3U);
            INST(5U, Opcode::BoundsCheck).s32().Inputs(4U, 1U, 2U);
            INST(6U, Opcode::LoadArray).u64().Inputs(3U, 5U);
            INST(7U, Opcode::Add).u64().Inputs(6U, 6U);
            INST(8U, Opcode::StoreArray).u64().Inputs(3U, 5U, 7U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(10U, Opcode::Add).u64().Inputs(7U, 7U);  // Some return value
            INST(11U, Opcode::Return).u64().Inputs(10U);
        }
    }
    SetNumVirtRegs(0U);
    SetNumArgs(2U);
    RegAlloc(GetGraph());

    EXPECT_TRUE(RunCodegen(GetGraph()));
    auto codeEntry = reinterpret_cast<char *>(GetGraph()->GetCode().Data());
    auto codeExit = codeEntry + GetGraph()->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    // NOTE (igorban) : fill Frame array == nullptr [THROW]

    uint64_t array[ARRAY_LEN];
    for (auto i = 0U; i < ARRAY_LEN; i++) {
        array[i] = i + 0x20U;
    }
    auto param1 = GetExecModule().CreateArray(array, ARRAY_LEN, GetObjectAllocator());
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param1));

    // 0 <= index < ARRAY_LEN [OK]
    auto index = CutValue<uint64_t>(1U, DataType::UINT64);
    GetExecModule().SetParameter(1U, index);
    GetExecModule().Execute();
    EXPECT_EQ(GetExecModule().GetRetValue(), array[index] * 4U);

    /*
    NOTE (igorban) : fill Frame
    // index < 0 [THROW]
    */
    GetExecModule().FreeArray(param1);
}
// NOLINTEND(readability-magic-numbers,modernize-avoid-c-arrays,cppcoreguidelines-pro-bounds-pointer-arithmetic)

}  // namespace ark::compiler
