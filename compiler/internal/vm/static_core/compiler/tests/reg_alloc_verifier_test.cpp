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

#include "libarkbase/macros.h"
#include "unit_test.h"
#include "optimizer/analysis/reg_alloc_verifier.h"
#include "optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"
#include "optimizer/optimizations/regalloc/spill_fills_resolver.h"
namespace ark::compiler {
class RegAllocVerifierTest : public GraphTest {
public:
    RegAllocVerifierTest() : defaultVerifyOption_(g_options.IsCompilerVerifyRegalloc())
    {
        // Avoid fatal errors in the negative-tests
        g_options.SetCompilerVerifyRegalloc(false);
    }

    ~RegAllocVerifierTest() override
    {
        g_options.SetCompilerVerifyRegalloc(defaultVerifyOption_);
    }

    NO_COPY_SEMANTIC(RegAllocVerifierTest);
    NO_MOVE_SEMANTIC(RegAllocVerifierTest);

private:
    bool defaultVerifyOption_ {};
};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(RegAllocVerifierTest, VerifyBranchelessCode)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Mul).u64().Inputs(0U, 2U);
            INST(4U, Opcode::Return).u64().Inputs(3U);
        }
    }

    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    ASSERT_TRUE(RegAllocVerifier(GetGraph()).Run());
}

TEST_F(RegAllocVerifierTest, VerifyBranching)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 42U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().Inputs(0U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }

        BASIC_BLOCK(3U, 5U)
        {
            INST(4U, Opcode::Mul).u64().Inputs(0U, 0U);
        }

        BASIC_BLOCK(4U, 5U)
        {
            INST(5U, Opcode::Mul).u64().Inputs(1U, 1U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(6U, Opcode::Phi).u64().Inputs(4U, 5U);
            INST(7U, Opcode::Return).u64().Inputs(6U);
        }
    }

    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    ASSERT_TRUE(RegAllocVerifier(GetGraph()).Run());
}

TEST_F(RegAllocVerifierTest, VerifyLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Phi).u64().Inputs(0U, 7U);
            INST(3U, Opcode::Phi).u64().Inputs(1U, 6U);
            INST(4U, Opcode::Compare).b().Inputs(2U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }

        BASIC_BLOCK(3U, 2U)
        {
            INST(6U, Opcode::Add).u64().Inputs(3U, 0U);
            INST(7U, Opcode::SubI).u64().Imm(1U).Inputs(2U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(8U, Opcode::Return).u64().Inputs(3U);
        }
    }

    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    ASSERT_TRUE(RegAllocVerifier(GetGraph()).Run());
}

TEST_F(RegAllocVerifierTest, VerifySingleBlockLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);

        BASIC_BLOCK(2U, 2U, 3U)
        {
            INST(2U, Opcode::Phi).u64().Inputs(0U, 3U);
            INST(3U, Opcode::SubI).u64().Imm(1U).Inputs(2U);
            INST(4U, Opcode::Compare).b().Inputs(3U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::Return).u64().Inputs(3U);
        }
    }

    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    ASSERT_TRUE(RegAllocVerifier(GetGraph()).Run());
}

TEST_F(RegAllocVerifierTest, LoadPair)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::NullCheck).ref().Inputs(0U, 1U);
            INST(3U, Opcode::LoadArrayPairI).s64().Inputs(2U).Imm(0x0U);
            INST(4U, Opcode::LoadPairPart).s64().Inputs(3U).Imm(0x0U);
            INST(5U, Opcode::LoadPairPart).s64().Inputs(3U).Imm(0x1U);
            INST(6U, Opcode::SaveState).Inputs(2U, 4U, 5U).SrcVregs({2U, 4U, 5U});
            INST(7U, Opcode::CallStatic).v0id().InputsAutoType(4U, 5U, 6U);
            INST(8U, Opcode::Add).s64().Inputs(4U, 5U);
            INST(9U, Opcode::Return).s64().Inputs(8U);
        }
    }
    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    ASSERT_TRUE(RegAllocVerifier(GetGraph()).Run());
}

TEST_F(RegAllocVerifierTest, TestVoidMethodCall)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::CallStatic).v0id().InputsAutoType(0U, 1U);
            INST(3U, Opcode::ReturnVoid).v0id();
        }
    }
    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    ASSERT_TRUE(RegAllocVerifier(GetGraph()).Run());
}

TEST_F(RegAllocVerifierTest, ZeroReg)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0U);
        CONSTANT(2U, nullptr);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(4U, Opcode::NullCheck).ref().Inputs(0U, 3U);
            INST(5U, Opcode::StoreObject).u64().Inputs(4U, 1U);
            INST(6U, Opcode::StoreObject).ref().Inputs(4U, 2U);
            INST(7U, Opcode::ReturnVoid).v0id();
        }
    }
    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    ASSERT_TRUE(RegAllocVerifier(GetGraph()).Run());
}

TEST_F(RegAllocVerifierTest, TooManyParameters)
{
    GRAPH(GetGraph())
    {
        for (size_t i = 0; i < 32U; i++) {
            PARAMETER(i, i).u64();
        }

        BASIC_BLOCK(2U, -1L)
        {
            for (size_t i = 0; i < 32U; i++) {
                INST(32U + i, Opcode::Add).u64().Inputs(i, 32U + i - 1L);
            }
            INST(64U, Opcode::Return).u64().Inputs(63U);
        }
    }
    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    ASSERT_TRUE(RegAllocVerifier(GetGraph()).Run());
}

TEST_F(RegAllocVerifierTest, ReorderSpillFill)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1U);
        CONSTANT(1U, 2U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SpillFill);
            INST(3U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(4U, Opcode::Return).u64().Inputs(3U);
        }
    }

    INS(0U).SetDstReg(0U);
    INS(1U).SetDstReg(1U);
    INS(3U).SetSrcReg(0U, 1U);
    INS(3U).SetSrcReg(1U, 0U);
    INS(3U).SetDstReg(0U);
    INS(4U).SetSrcReg(0U, 0U);

    auto sf = INS(2U).CastToSpillFill();
    // swap 0 and 1: should be 0 -> tmp, 1 -> 0, tmp -> 1, but we intentionally reorder it
    sf->AddMove(1U, 0U, DataType::UINT64);
    sf->AddMove(0U, 16U, DataType::UINT64);
    sf->AddMove(16U, 1U, DataType::UINT64);

    ArenaVector<bool> regs = ArenaVector<bool>(std::max(MAX_NUM_REGS, MAX_NUM_VREGS), false, GetAllocator()->Adapter());
    GetGraph()->InitUsedRegs<DataType::INT64>(&regs);
    GetGraph()->InitUsedRegs<DataType::FLOAT64>(&regs);
    ASSERT_FALSE(RegAllocVerifier(GetGraph(), false).Run());
}

TEST_F(RegAllocVerifierTest, UseIncorrectInputReg)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Mul).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    INS(3U).SetSrcReg(0U, 15U);
    ASSERT_FALSE(RegAllocVerifier(GetGraph()).Run());
}

TEST_F(RegAllocVerifierTest, CallSpillFillReordering)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(1U, 1U).u32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::CallStatic).InputsAutoType(1U, 0U, 2U).v0id();
            INST(4U, Opcode::ReturnVoid).v0id();
        }
    }

    // Manually allocate registers to ensure that parameters 0 and 1 will be swapped
    // at CallStatic call site. That swap requires additional resolving move.
    INS(0).CastToParameter()->SetLocationData(
        {LocationType::REGISTER, LocationType::REGISTER, 1U, 1U, DataType::Type::INT32});
    INS(0U).SetDstReg(1U);

    INS(1).CastToParameter()->SetLocationData(
        {LocationType::REGISTER, LocationType::REGISTER, 2U, 2U, DataType::Type::INT32});
    INS(1U).SetDstReg(2U);

    ArenaVector<bool> regs = ArenaVector<bool>(std::max(MAX_NUM_REGS, MAX_NUM_VREGS), false, GetAllocator()->Adapter());
    GetGraph()->InitUsedRegs<DataType::INT64>(&regs);
    GetGraph()->InitUsedRegs<DataType::FLOAT64>(&regs);

    RegAllocLinearScan(GetGraph()).Run();
    ASSERT_TRUE(RegAllocVerifier(GetGraph(), false).Run());
}

TEST_F(RegAllocVerifierTest, VerifyIndirectCall)
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        // NOTE (zwx1004932) Support spill/fills in entrypoints frame and enable test
        GTEST_SKIP();
    }
    GetGraph()->SetMode(GraphMode::FastPath());
    auto ptrType = Is64BitsArch(GetGraph()->GetArch()) ? DataType::Type::UINT64 : DataType::Type::UINT32;

    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).type(ptrType);
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::CallIndirect).InputsAutoType(0U, 1U, 2U).u64();
            INST(4U, Opcode::Return).Inputs(3U).u64();
        }
    }

    GetGraph()->RunPass<RegAllocLinearScan>();
    // Inputs overlapping: Input #2 has fixed location r1, Input #0 has dst r1
    // Failed verification is correct for now. Fix it.
    ASSERT_FALSE(RegAllocVerifier(GetGraph()).Run());
}

TEST_F(RegAllocVerifierTest, VerifyInlinedCall)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Add).Inputs(0U, 1U).u64();
            INST(3U, Opcode::Mul).Inputs(0U, 2U).u64();
            INST(4U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(5U, Opcode::CallStatic).Inlined().InputsAutoType(3U, 2U, 1U, 0U, 4U).u64();
            INST(6U, Opcode::ReturnInlined).Inputs(4U).v0id();
            INST(7U, Opcode::ReturnVoid).v0id();
        }
    }

    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    ASSERT_EQ(INS(5U).GetDstReg(), INVALID_REG);
    ASSERT_TRUE(RegAllocVerifier(GetGraph()).Run());
}

TEST_F(RegAllocVerifierTest, VerifyEmptyStartBlock)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, -1L)
        {
            CONSTANT(0U, 0xABCDABCDABCDABCDLL);
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::CallStatic).InputsAutoType(0U, 1U).u64();
            INST(3U, Opcode::Mul).Inputs(0U, 2U).u64();
            INST(4U, Opcode::Return).Inputs(3U).u64();
        }
    }

    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    if (GetGraph()->GetCallingConvention() == nullptr) {
        ASSERT_FALSE(result);
        return;
    }
    ASSERT_TRUE(result);
    // set incorrect regs
    INS(3U).SetDstReg(INS(3U).GetDstReg() + 1U);
    ASSERT_FALSE(RegAllocVerifier(GetGraph()).Run());
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
