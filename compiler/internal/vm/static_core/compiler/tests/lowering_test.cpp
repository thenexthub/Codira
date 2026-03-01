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

#include "unit_test.h"
#include "libarkbase/utils/utils.h"
#include "optimizer/ir/graph_cloner.h"
#include "optimizer/code_generator/codegen.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/lowering.h"
#include "optimizer/optimizations/regalloc/reg_alloc.h"

namespace ark::compiler {

namespace {
using TypeTriple = std::array<DataType::Type, 3U>;
using ShiftOpPair = std::tuple<Opcode, Opcode, ShiftType>;
using ShiftOp = std::pair<Opcode, ShiftType>;
using OpcodePair = std::pair<Opcode, Opcode>;
}  // namespace

class LoweringTest : public GraphTest {
public:
    LoweringTest()  // NOLINT(modernize-use-equals-default)
    {
#ifndef NDEBUG
        graph_->SetLowLevelInstructionsEnabled();
#endif
    }
    template <class T>
    void ReturnTest(T val, DataType::Type type)
    {
        auto graph = CreateGraphStartEndBlocks();
#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        auto cnst = graph->FindOrCreateConstant(val);
        auto block = graph->CreateEmptyBlock();
        graph->GetStartBlock()->AddSucc(block);
        block->AddSucc(graph->GetEndBlock());
        auto ret = graph->CreateInstReturn(type, INVALID_PC, cnst);
        block->AppendInst(ret);
        graph->RunPass<LoopAnalyzer>();
        GraphChecker(graph).Check();

        graph->RunPass<Lowering>();
        EXPECT_FALSE(cnst->HasUsers());
        GraphChecker(graph).Check();
#ifndef NDEBUG
        graph->SetRegAllocApplied();
#endif
        EXPECT_TRUE(graph->RunPass<Codegen>());
    }

    Graph *CreateEmptyLowLevelGraph()
    {
        auto graph = CreateEmptyGraph();
#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif
        return graph;
    }

    void BuildGraphLoweringAddSub();
    void BuildGraphLoweringLogic();
    void BuildGraphSaveStateTest();
    Graph *BuildGraphIf1(ConditionCode cc);
    Graph *BuildGraphIf2(ConditionCode cc);
    Graph *BuildGraphIfRef();
    Graph *BuildGraphIfFcmplNoJoin2(ConditionCode cc);
    Graph *BuildGraphMultiplyAddInteger(const std::array<DataType::Type, 4U> &types);
    Graph *BuildExpectedMultiplyAddInteger(const std::array<DataType::Type, 4U> &types);
    Graph *BuildGraphMultiplySubInteger(const std::array<DataType::Type, 4U> &types);
    Graph *BuildExpectedMultiplySubInteger(const std::array<DataType::Type, 4U> &types);
    Graph *BuildGraphMultiplyNegate(const TypeTriple &types);
    Graph *BuildExpectedMultiplyNegate(const TypeTriple &types);

    void TestBitwiseBinaryOpWithInvertedOperand(const TypeTriple &types, OpcodePair ops);
    Graph *BuildGraphCommutativeBinaryOpWithShiftedOperand(const TypeTriple &types, ShiftOpPair shiftOp,
                                                           OpcodePair ops);
    Graph *BuildExpectedCommutativeBinaryOpWithShiftedOperand(const TypeTriple &types, ShiftOpPair shiftOp,
                                                              OpcodePair ops);
    void TestCommutativeBinaryOpWithShiftedOperandWithIncompatibleInstructionTypes(ShiftOpPair shiftOp, Opcode op);
    Graph *BuildGraphSubWithShiftedOperand(TypeTriple types, ShiftOpPair shiftOp);
    void TestSubWithShiftedOperand(TypeTriple types, ShiftOpPair shiftOp);
    void TestNonCommutativeBinaryOpWithShiftedOperand(const TypeTriple &types, const ShiftOpPair &shiftOp,
                                                      OpcodePair ops);
    void TestNonCommutativeBinaryOpWithShiftedOperandWithIncompatibleInstructionTypes(const ShiftOpPair &shiftOp,
                                                                                      Opcode op);
    void TestBitwiseInstructionsWithInvertedShiftedOperand(const TypeTriple &types, ShiftOp shiftOp, OpcodePair ops);
    void TestBitwiseInstructionsWithInvertedShiftedOperandWithIncompatibleInstructionTypes(ShiftOp shiftOp, Opcode op);
    void BuildGraphDeoptimizeCompareImmDoesNotFit(Graph *graph);

    void BuildGraphLowerMoveScaleInLoadStore(Graph *graph);
    void BuildExpectedLowerMoveScaleInLoadStoreAArch64(Graph *graphExpected);
    void BuildExpectedLowerMoveScaleInLoadStoreAmd64(Graph *graphExpected);

    void BuildGraphLowerUnsignedCast(Graph *graph);

    void DoTestCompareBoolConstZero(ConditionCode code, bool swap);
};

// NOLINTBEGIN(readability-magic-numbers)
void LoweringTest::BuildGraphLoweringAddSub()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(11U, 1U).f64();
        PARAMETER(12U, 2U).f32();
        CONSTANT(1U, 12U);
        CONSTANT(2U, -1L);
        CONSTANT(3U, 100000000U);
        CONSTANT(21U, 1.2_D);
        CONSTANT(22U, 0.5F);

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(5U, Opcode::Add).u64().Inputs(0U, 2U);
            INST(6U, Opcode::Add).u64().Inputs(0U, 3U);
            INST(7U, Opcode::Sub).u64().Inputs(0U, 1U);
            INST(8U, Opcode::Sub).u64().Inputs(0U, 2U);
            INST(9U, Opcode::Sub).u64().Inputs(0U, 3U);
            INST(13U, Opcode::Add).f64().Inputs(11U, 21U);
            INST(14U, Opcode::Sub).f64().Inputs(11U, 21U);
            INST(15U, Opcode::Add).f32().Inputs(12U, 22U);
            INST(16U, Opcode::Sub).f32().Inputs(12U, 22U);
            INST(17U, Opcode::Add).u64().Inputs(0U, 0U);
            INST(18U, Opcode::Sub).u64().Inputs(0U, 0U);
            INST(19U, Opcode::Add).u16().Inputs(0U, 1U);
            INST(20U, Opcode::Add).u16().Inputs(0U, 2U);
            INST(10U, Opcode::SafePoint)
                .Inputs(0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 11U, 12U, 13U, 14U, 15U, 16U, 17U, 18U, 19U, 20U, 21U,
                        22U)
                .SrcVregs({0U,  1U,  2U,  3U,  4U,  5U,  6U,  7U,  8U,  9U,  10U,
                           11U, 12U, 13U, 14U, 15U, 16U, 17U, 18U, 19U, 20U, 21U});
            INST(23U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(LoweringTest, LoweringAddSub)
{
    BuildGraphLoweringAddSub();
    GetGraph()->RunPass<Lowering>();
    ASSERT_FALSE(INS(4U).HasUsers());
    ASSERT_FALSE(INS(5U).HasUsers());
    ASSERT_TRUE(INS(6U).HasUsers());
    ASSERT_FALSE(INS(7U).HasUsers());
    ASSERT_FALSE(INS(8U).HasUsers());
    ASSERT_TRUE(INS(9U).HasUsers());
    ASSERT_TRUE(INS(13U).HasUsers());
    ASSERT_TRUE(INS(14U).HasUsers());
    ASSERT_TRUE(INS(15U).HasUsers());
    ASSERT_TRUE(INS(16U).HasUsers());
    ASSERT_TRUE(INS(17U).HasUsers());
    ASSERT_TRUE(INS(18U).HasUsers());
    ASSERT_TRUE(INS(19U).HasUsers());
    ASSERT_TRUE(INS(20U).HasUsers());
    ASSERT_EQ(INS(4U).GetPrev()->GetOpcode(), Opcode::AddI);
    ASSERT_EQ(INS(5U).GetPrev()->GetOpcode(), Opcode::SubI);
    ASSERT_EQ(INS(6U).GetPrev()->GetOpcode(), Opcode::Add);
    ASSERT_EQ(INS(7U).GetPrev()->GetOpcode(), Opcode::SubI);
    ASSERT_EQ(INS(8U).GetPrev()->GetOpcode(), Opcode::AddI);
    ASSERT_EQ(INS(9U).GetPrev()->GetOpcode(), Opcode::Sub);
}

TEST_F(LoweringTest, AddSubCornerCase)
{
    auto graph = CreateEmptyBytecodeGraph();
    constexpr int MIN = std::numeric_limits<int8_t>::min();
    ASSERT_TRUE(MIN < 0);
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(2U, MIN).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Add).s32().Inputs(0U, 2U);
            INST(4U, Opcode::Sub).s32().Inputs(0U, 2U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::CallStatic).b().InputsAutoType(3U, 4U, 20U);
            INST(6U, Opcode::Return).b().Inputs(5U);
        }
    }
#ifndef NDEBUG
    graph->SetLowLevelInstructionsEnabled();
#endif
    graph->RunPass<compiler::Lowering>();
    graph->RunPass<compiler::Cleanup>();
    auto expected = CreateEmptyBytecodeGraph();
    GRAPH(expected)
    {
        PARAMETER(0U, 0U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            // As MIN = -128 and -MIN cannot be encoded with 8-bit, the opcodes will not be reversed.
            INST(7U, Opcode::AddI).s32().Inputs(0U).Imm(MIN);
            INST(8U, Opcode::SubI).s32().Inputs(0U).Imm(MIN);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::CallStatic).b().InputsAutoType(7U, 8U, 20U);
            INST(6U, Opcode::Return).b().Inputs(5U);
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(graph, expected));
}

void LoweringTest::BuildGraphLoweringLogic()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 12U);
        CONSTANT(2U, 50U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Or).u32().Inputs(0U, 1U);
            INST(4U, Opcode::Or).u64().Inputs(0U, 1U);
            INST(5U, Opcode::Or).u64().Inputs(0U, 2U);
            INST(6U, Opcode::And).u32().Inputs(0U, 1U);
            INST(7U, Opcode::And).u64().Inputs(0U, 1U);
            INST(8U, Opcode::And).u64().Inputs(0U, 2U);
            INST(9U, Opcode::Xor).u32().Inputs(0U, 1U);
            INST(10U, Opcode::Xor).u64().Inputs(0U, 1U);
            INST(11U, Opcode::Xor).u64().Inputs(0U, 2U);
            INST(12U, Opcode::Or).u8().Inputs(0U, 1U);
            INST(13U, Opcode::And).u64().Inputs(0U, 0U);
            INST(14U, Opcode::SafePoint)
                .Inputs(0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U, 11U, 12U, 13U)
                .SrcVregs({0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U, 11U, 12U, 13U});
            INST(23U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(LoweringTest, LoweringLogic)
{
    BuildGraphLoweringLogic();
    GetGraph()->RunPass<Lowering>();
    if (GetGraph()->GetArch() != Arch::AARCH32) {
        ASSERT_FALSE(INS(3U).HasUsers());
        ASSERT_FALSE(INS(4U).HasUsers());
        ASSERT_TRUE(INS(5U).HasUsers());
        ASSERT_FALSE(INS(6U).HasUsers());
        ASSERT_FALSE(INS(7U).HasUsers());
        ASSERT_TRUE(INS(8U).HasUsers());
        ASSERT_FALSE(INS(9U).HasUsers());
        ASSERT_FALSE(INS(10U).HasUsers());
        ASSERT_TRUE(INS(11U).HasUsers());
        ASSERT_TRUE(INS(12U).HasUsers());
        ASSERT_TRUE(INS(13U).HasUsers());
        ASSERT_EQ(INS(3U).GetPrev()->GetOpcode(), Opcode::OrI);
        ASSERT_EQ(INS(4U).GetPrev()->GetOpcode(), Opcode::OrI);
        ASSERT_EQ(INS(5U).GetPrev()->GetOpcode(), Opcode::Or);
        ASSERT_EQ(INS(6U).GetPrev()->GetOpcode(), Opcode::AndI);
        ASSERT_EQ(INS(7U).GetPrev()->GetOpcode(), Opcode::AndI);
        ASSERT_EQ(INS(8U).GetPrev()->GetOpcode(), Opcode::And);
        ASSERT_EQ(INS(9U).GetPrev()->GetOpcode(), Opcode::XorI);
        ASSERT_EQ(INS(10U).GetPrev()->GetOpcode(), Opcode::XorI);
        ASSERT_EQ(INS(11U).GetPrev()->GetOpcode(), Opcode::Xor);
        return;
    }
    // Then check graph only for arm32
    ASSERT_FALSE(INS(3U).HasUsers());
    ASSERT_FALSE(INS(4U).HasUsers());
    ASSERT_FALSE(INS(5U).HasUsers());
    ASSERT_FALSE(INS(6U).HasUsers());
    ASSERT_FALSE(INS(7U).HasUsers());
    ASSERT_FALSE(INS(8U).HasUsers());
    ASSERT_FALSE(INS(9U).HasUsers());
    ASSERT_FALSE(INS(10U).HasUsers());
    ASSERT_FALSE(INS(11U).HasUsers());
    ASSERT_TRUE(INS(12U).HasUsers());
    ASSERT_TRUE(INS(13U).HasUsers());
    ASSERT_EQ(INS(3U).GetPrev()->GetOpcode(), Opcode::OrI);
    ASSERT_EQ(INS(4U).GetPrev()->GetOpcode(), Opcode::OrI);
    ASSERT_EQ(INS(5U).GetPrev()->GetOpcode(), Opcode::OrI);
    ASSERT_EQ(INS(6U).GetPrev()->GetOpcode(), Opcode::AndI);
    ASSERT_EQ(INS(7U).GetPrev()->GetOpcode(), Opcode::AndI);
    ASSERT_EQ(INS(8U).GetPrev()->GetOpcode(), Opcode::AndI);
    ASSERT_EQ(INS(9U).GetPrev()->GetOpcode(), Opcode::XorI);
    ASSERT_EQ(INS(10U).GetPrev()->GetOpcode(), Opcode::XorI);
    ASSERT_EQ(INS(11U).GetPrev()->GetOpcode(), Opcode::XorI);
}

// 0.i64 Constant n1
// 1.i64 Constant M = 2^n2 - 1
// 2. ...
// 3.Type Shr|Ashr v2, v0
// 4.Type And v3, v1
// ====>
// 0.i64 Constant n1
// 1.i64 Constant M = 2^n2 - 1
// 2. ...
// 3.Type ExtractBitfield v2, n1, n2 - n1 [ZeroExtension]
TEST_F(LoweringTest, LoweringAndToBitfieldExtraction)
{
    struct TestCase {
        bool expectsApplication;
        DataType::Type dataType;
        uint32_t shrValue;
        uint64_t mask;
        uint32_t expectedUbfxWidth;
    };
    constexpr auto IGNORED = std::numeric_limits<unsigned>::max();
    auto testCases = std::vector<TestCase> {
        {true, DataType::INT32, 2, 0x3FU, 6U},
        {true, DataType::UINT32, 9, 0x7FU, 7U},
        {true, DataType::INT32, 19, 0x1FFFU, 13U},
        {true, DataType::UINT32, 14, 0x3FFU, 10U},
        {true, DataType::INT64, 17, 0x3FF'FFFFU, 26U},
        {true, DataType::UINT64, 25, 0x7F'FFFF'FFFFULL, 39U},
        // mask is not 2^n2 - 1
        {false, DataType::INT32, 4U, 0xDU, IGNORED},
        {false, DataType::UINT32, 6U, 0x6FU, IGNORED},
        {false, DataType::INT64, 20U, 0x8FF8U, IGNORED},
        {false, DataType::UINT64, 40U, 0x7F7F7FU, IGNORED},
        // shift amount + bit size of mask > bit size of data type
        {false, DataType::INT32, 18U, 0xFFFFU, IGNORED},
        {false, DataType::INT64, 45U, 0xFFFFFU, IGNORED},
    };
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    for (size_t i = 0; i < testCases.size(); i++) {
        const auto &curCase = testCases[i];
        auto *graph = CreateEmptyLowLevelGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 1U).type(curCase.dataType);
            CONSTANT(1U, curCase.shrValue);
            CONSTANT(2U, curCase.mask);
            BASIC_BLOCK(2U, -1L)
            {
                auto opc = (i % 2 == 0) ? Opcode::Shr : Opcode::AShr;  // Tests Shr and AShr alternatively
                INST(3U, opc).type(curCase.dataType).Inputs(0U, 1U);
                INST(4U, Opcode::And).type(curCase.dataType).Inputs(3U, 2U);
                INST(5U, Opcode::Return).type(curCase.dataType).Inputs(4U);
            }
        }
        graph->RunPass<Lowering>();
        GraphChecker(graph).Check();

        auto ubfxInstRaw = INS(5U).GetInput(0).GetInst();
        if (curCase.expectsApplication) {
            ASSERT_EQ(Opcode::ExtractBitfield, ubfxInstRaw->GetOpcode()) << "Failure in test case #" << i;

            auto *ubfxInst = ubfxInstRaw->CastToExtractBitfield();
            ASSERT_EQ(curCase.dataType, ubfxInst->GetType());
            ASSERT_TRUE(ubfxInst->IsZeroExtension());
            uint32_t expectedSourceBit = curCase.shrValue;
            ASSERT_EQ(expectedSourceBit, ubfxInst->GetSourceBit());
            ASSERT_EQ(curCase.expectedUbfxWidth, ubfxInst->GetWidth());
            ASSERT_EQ(ubfxInst->GetInput(0).GetInst(), &INS(0U));
        } else {
            ASSERT_NE(Opcode::ExtractBitfield, ubfxInstRaw->GetOpcode()) << "Failure in test case #" << i;
        }
    }
}

TEST_F(LoweringTest, LoweringShift)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 12U);
        CONSTANT(2U, 64U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Shr).u32().Inputs(0U, 1U);
            INST(4U, Opcode::Shr).u64().Inputs(0U, 1U);
            INST(5U, Opcode::Shr).u64().Inputs(0U, 2U);
            INST(6U, Opcode::AShr).u32().Inputs(0U, 1U);
            INST(7U, Opcode::AShr).u64().Inputs(0U, 1U);
            INST(8U, Opcode::AShr).u64().Inputs(0U, 2U);
            INST(9U, Opcode::Shl).u32().Inputs(0U, 1U);
            INST(10U, Opcode::Shl).u64().Inputs(0U, 1U);
            INST(11U, Opcode::Shl).u64().Inputs(0U, 2U);
            INST(12U, Opcode::Shl).u8().Inputs(0U, 1U);
            INST(13U, Opcode::Shr).u64().Inputs(0U, 0U);
            INST(14U, Opcode::SafePoint)
                .Inputs(0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U, 11U, 12U, 13U)
                .SrcVregs({0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U, 11U, 12U, 13U});
            INST(23U, Opcode::ReturnVoid);
        }
    }
    GetGraph()->RunPass<Lowering>();
    ASSERT_FALSE(INS(3U).HasUsers());
    ASSERT_FALSE(INS(4U).HasUsers());
    ASSERT_TRUE(INS(5U).HasUsers());
    ASSERT_FALSE(INS(6U).HasUsers());
    ASSERT_FALSE(INS(7U).HasUsers());
    ASSERT_TRUE(INS(8U).HasUsers());
    ASSERT_FALSE(INS(9U).HasUsers());
    ASSERT_FALSE(INS(10U).HasUsers());
    ASSERT_TRUE(INS(11U).HasUsers());
    ASSERT_TRUE(INS(12U).HasUsers());
    ASSERT_TRUE(INS(13U).HasUsers());
    ASSERT_EQ(INS(3U).GetPrev()->GetOpcode(), Opcode::ShrI);
    ASSERT_EQ(INS(4U).GetPrev()->GetOpcode(), Opcode::ShrI);
    ASSERT_EQ(INS(5U).GetPrev()->GetOpcode(), Opcode::Shr);
    ASSERT_EQ(INS(6U).GetPrev()->GetOpcode(), Opcode::AShrI);
    ASSERT_EQ(INS(7U).GetPrev()->GetOpcode(), Opcode::AShrI);
    ASSERT_EQ(INS(8U).GetPrev()->GetOpcode(), Opcode::AShr);
    ASSERT_EQ(INS(9U).GetPrev()->GetOpcode(), Opcode::ShlI);
    ASSERT_EQ(INS(10U).GetPrev()->GetOpcode(), Opcode::ShlI);
    ASSERT_EQ(INS(11U).GetPrev()->GetOpcode(), Opcode::Shl);
}

// 0.i64 Constant (B - n1 - n2)
// 1.i64 Constant (B - n2)
// 2. ...
// 3.Type Shl v2, v0
// 4.Type Shr|AShr v3, v1
// ====>
// 0.i64 Constant (B - n1 - n2)
// 1.i64 Constant (B - n2)
// 2. ...
// 3.Type ExtractBitfield v2, n1, n2
TEST_F(LoweringTest, LoweringShiftToBitfieldExtractionCase1)
{
    struct TestCase {
        bool expectsApplication;
        DataType::Type dataType;
        uint32_t shlValue;
        uint32_t shrValue;
        uint32_t expectedBfxSourceBit;
        uint32_t expectedBfxWidth;
    };
    constexpr auto IGNORED = std::numeric_limits<unsigned>::max();
    auto testCases = std::vector<TestCase> {
        {true, DataType::INT32, 3U, 4U, 1U, 28U},
        {true, DataType::INT32, 6U, 6U, 0U, 26U},
        {true, DataType::INT32, 9U, 12U, 3U, 20U},
        {true, DataType::INT64, 12U, 19U, 7U, 45U},
        {true, DataType::INT64, 45U, 45U, 0U, 19U},
        // shr amount < shl amount
        {false, DataType::INT32, 3U, 2U, IGNORED, IGNORED},
        {false, DataType::INT32, 6U, 4U, IGNORED, IGNORED},
        {false, DataType::INT64, 31U, 15U, IGNORED, IGNORED},
    };
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    for (size_t i = 0; i < testCases.size(); i++) {
        const auto &curCase = testCases[i];
        auto *graph = CreateEmptyLowLevelGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 1U).type(curCase.dataType);
            CONSTANT(1U, curCase.shlValue);
            CONSTANT(2U, curCase.shrValue);
            BASIC_BLOCK(2U, -1L)
            {
                auto opc = (i % 2 == 0) ? Opcode::Shr : Opcode::AShr;  // 2 : Tests Shr and AShr alternatively
                INST(3U, Opcode::Shl).type(curCase.dataType).Inputs(0U, 1U);
                INST(4U, opc).type(curCase.dataType).Inputs(3U, 2U);
                INST(5U, Opcode::Return).type(curCase.dataType).Inputs(4U);
            }
        }
        graph->RunPass<Lowering>();
        GraphChecker(graph).Check();

        auto bfxInstRaw = INS(5U).GetInput(0).GetInst();
        if (curCase.expectsApplication) {
            ASSERT_EQ(Opcode::ExtractBitfield, bfxInstRaw->GetOpcode()) << "Failure in test case #" << i;

            auto *bfxInst = bfxInstRaw->CastToExtractBitfield();
            ASSERT_EQ(curCase.dataType, bfxInst->GetType());
            // 2 : Tests Shr and AShr alternatively
            ASSERT_TRUE(i % 2 == 0 ? bfxInst->IsZeroExtension() : bfxInst->IsSignBitExtension());
            ASSERT_EQ(curCase.expectedBfxSourceBit, bfxInst->GetSourceBit());
            ASSERT_EQ(curCase.expectedBfxWidth, bfxInst->GetWidth());
            ASSERT_EQ(bfxInst->GetInput(0).GetInst(), &INS(0U));
        } else {
            ASSERT_NE(Opcode::ExtractBitfield, bfxInstRaw->GetOpcode()) << "Failure in test case #" << i;
        }
    }
}

TEST_F(LoweringTest, LoweringShiftToBitfieldExtractionCase1NonConstant1)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    // Not applied since shl mount is not constant
    auto *graph = CreateEmptyLowLevelGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(1U, 1U).u32();
        CONSTANT(2U, 8U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Shl).u32().Inputs(0U, 1U);
            INST(4U, Opcode::AShr).u32().Inputs(3U, 2U);
            INST(5U, Opcode::Return).u32().Inputs(4U);
        }
    }
    graph->RunPass<Lowering>();
    GraphChecker(graph).Check();

    auto notSbfxInst = INS(5U).GetInput(0).GetInst();
    ASSERT_NE(Opcode::ExtractBitfield, notSbfxInst->GetOpcode());
}

TEST_F(LoweringTest, LoweringShiftToBitfieldExtractionCase1NonConstant2)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    // Not applied since shr mount is not constant
    auto *graph = CreateEmptyLowLevelGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(1U, 1U).u32();
        CONSTANT(2U, 8U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Shl).u32().Inputs(0U, 2U);
            INST(4U, Opcode::AShr).u32().Inputs(3U, 1U);
            INST(5U, Opcode::Return).u32().Inputs(4U);
        }
    }
    graph->RunPass<Lowering>();
    GraphChecker(graph).Check();

    auto notSbfxInst = INS(5U).GetInput(0).GetInst();
    ASSERT_NE(Opcode::ExtractBitfield, notSbfxInst->GetOpcode());
}

// 0.i64 Constant M, where 2^n2 - 2^n1 <= M <= 2^n2 - 1
// 1.i64 Constant n1
// 2. ...
// 3.Type And v2, v0
// 4.Type Shr|AShr v3, v1
// ====>
// 0.i64 Constant M, where 2^n2 - 2^n1 <= M <= 2^n2 - 1
// 1.i64 Constant n1
// 2. ...
// 3.Type ExtractBitfield v2, n1, n2 - n1
TEST_F(LoweringTest, LoweringShiftToBitfieldExtractionCase2)
{
    struct TestCase {
        bool expectsApplication;
        bool expectsSbfxOnAShr;
        DataType::Type dataType;
        uint64_t mask;
        uint32_t shrValue;
        uint32_t expectedBfxWidth;
    };
    constexpr auto IGNORED = std::numeric_limits<unsigned>::max();
    auto testCases = std::vector<TestCase> {
        {true, false, DataType::INT32, 0x1FU, 2U, 3U},
        {true, false, DataType::INT32, 0x1FF5U, 4U, 9U},
        {true, false, DataType::INT32, 0x1FF'FFF8ULL, 8U, 17U},
        {true, false, DataType::INT64, 0xFFFF'FFFF'FFFFULL, 15U, 33U},
        {true, true, DataType::INT32, 0xFFFF'FFF8ULL, 8U, 24U},
        {true, true, DataType::INT64, 0xFFFF'FFFF'FFFF'F123ULL, 15U, 49U},
        // Invalid mask
        {false, false, DataType::INT32, 0x1FULL, 5U, IGNORED},
        {false, false, DataType::INT64, 0x1FF0ULL, 14U, IGNORED},
        {false, false, DataType::INT32, 0x1FF'FE80ULL, 7U, IGNORED},
        {false, false, DataType::INT64, 0xFFFF'FFFF'7FFFULL, 12U, IGNORED},
    };
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    for (auto opc : {Opcode::Shr, Opcode::AShr}) {
        for (const auto &curCase : testCases) {
            auto *graph = CreateEmptyLowLevelGraph();
            GRAPH(graph)
            {
                PARAMETER(0U, 1U).type(curCase.dataType);
                CONSTANT(1U, curCase.mask);
                CONSTANT(2U, curCase.shrValue);
                BASIC_BLOCK(2U, -1L)
                {
                    INST(3U, Opcode::And).type(curCase.dataType).Inputs(0U, 1U);
                    INST(4U, opc).type(curCase.dataType).Inputs(3U, 2U);
                    INST(5U, Opcode::Return).type(curCase.dataType).Inputs(4U);
                }
            }
            graph->RunPass<Lowering>();
            GraphChecker(graph).Check();

            auto bfxInstRaw = INS(5U).GetInput(0).GetInst();
            if (curCase.expectsApplication) {
                ASSERT_EQ(Opcode::ExtractBitfield, bfxInstRaw->GetOpcode());

                auto *bfxInst = bfxInstRaw->CastToExtractBitfield();
                ASSERT_EQ(curCase.dataType, bfxInst->GetType());
                bool expectsSbfx = opc == Opcode::AShr && curCase.expectsSbfxOnAShr;
                ASSERT_TRUE(expectsSbfx ? bfxInst->IsSignBitExtension() : bfxInst->IsZeroExtension());
                uint32_t expectedSourceBit = curCase.shrValue;
                ASSERT_EQ(expectedSourceBit, bfxInst->GetSourceBit());
                ASSERT_EQ(curCase.expectedBfxWidth, bfxInst->GetWidth());
                ASSERT_EQ(bfxInst->GetInput(0).GetInst(), &INS(0U));
            } else {
                ASSERT_NE(Opcode::ExtractBitfield, bfxInstRaw->GetOpcode());
            }
        }
    }
}

TEST_F(LoweringTest, LoweringShiftToBitfieldExtractionCase2NonConstant1)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    // Not applied since mask is not constant
    auto *graph = CreateEmptyLowLevelGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(1U, 1U).u32();
        CONSTANT(2U, 8U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::And).u32().Inputs(0U, 1U);
            INST(4U, Opcode::Shr).u32().Inputs(3U, 2U);
            INST(5U, Opcode::Return).u32().Inputs(4U);
        }
    }
    graph->RunPass<Lowering>();
    GraphChecker(graph).Check();

    auto notUbfxInst = INS(5U).GetInput(0).GetInst();
    ASSERT_NE(Opcode::ExtractBitfield, notUbfxInst->GetOpcode());
}

TEST_F(LoweringTest, LoweringShiftToBitfieldExtractionCase2NonConstant2)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    // Not applied since shr amount is not constant
    auto *graph = CreateEmptyLowLevelGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(1U, 1U).u32();
        CONSTANT(2U, 0xFFFFFFU);
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::And).u32().Inputs(0U, 2U);
            INST(4U, Opcode::Shr).u32().Inputs(3U, 1U);
            INST(5U, Opcode::Return).u32().Inputs(4U);
        }
    }
    graph->RunPass<Lowering>();
    GraphChecker(graph).Check();

    auto ubfxInst = INS(5U).GetInput(0).GetInst();
    ASSERT_NE(Opcode::ExtractBitfield, ubfxInst->GetOpcode());
}

void LoweringTest::BuildGraphSaveStateTest()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        CONSTANT(3U, 50U);
        CONSTANT(4U, 0.5_D);
        PARAMETER(5U, 1U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U, 4U, 5U).SrcVregs({10U, 11U, 12U, 13U, 14U, 15U});
            INST(11U, Opcode::SafePoint).Inputs(0U, 1U, 2U, 3U, 4U, 5U).SrcVregs({10U, 11U, 12U, 13U, 14U, 15U});
            INST(9U, Opcode::CallStatic).u64().InputsAutoType(0U, 1U, 8U);
            INST(10U, Opcode::Return).u64().Inputs(9U);
        }
    }
}

TEST_F(LoweringTest, SaveStateTest)
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        GTEST_SKIP() << "The optimization for float isn't supported on Aarch32";
    }
    BuildGraphSaveStateTest();

    GetGraph()->RunPass<Lowering>();
    GraphChecker(GetGraph()).Check();
    EXPECT_TRUE(INS(0U).HasUsers());
    EXPECT_TRUE(INS(1U).HasUsers());
    EXPECT_FALSE(INS(2U).HasUsers());
    EXPECT_FALSE(INS(3U).HasUsers());
    EXPECT_FALSE(INS(4U).HasUsers());
    EXPECT_TRUE(INS(5U).HasUsers());

    auto saveState = INS(8U).CastToSaveState();
    auto safePoint = INS(11U).CastToSafePoint();

    EXPECT_EQ(saveState->GetInputsCount(), 2U);
    EXPECT_EQ(saveState->GetImmediatesCount(), 4U);
    EXPECT_EQ((*saveState->GetImmediates())[0U].value, (bit_cast<uint64_t, uint64_t>(0U)));
    EXPECT_EQ((*saveState->GetImmediates())[1U].value, (bit_cast<uint64_t, uint64_t>(1U)));
    EXPECT_EQ((*saveState->GetImmediates())[2U].value, (bit_cast<uint64_t, double>(0.5_D)));
    EXPECT_EQ((*saveState->GetImmediates())[3U].value, (bit_cast<uint64_t, uint64_t>(50U)));
    EXPECT_EQ(saveState->GetInput(0U).GetInst(), &INS(0U));
    EXPECT_EQ(saveState->GetInput(1U).GetInst(), &INS(5U));
    EXPECT_EQ(saveState->GetVirtualRegister(0U).Value(), 10U);
    EXPECT_EQ(saveState->GetVirtualRegister(1U).Value(), 15U);

    EXPECT_EQ(safePoint->GetInputsCount(), 2U);
    EXPECT_EQ(safePoint->GetImmediatesCount(), 4U);
    EXPECT_EQ(safePoint->GetInput(0U).GetInst(), &INS(0U));
    EXPECT_EQ(safePoint->GetInput(1U).GetInst(), &INS(5U));
    EXPECT_EQ(safePoint->GetVirtualRegister(0U).Value(), 10U);
    EXPECT_EQ(safePoint->GetVirtualRegister(1U).Value(), 15U);

    GetGraph()->RunPass<LoopAnalyzer>();
    RegAlloc(GetGraph());
    SetNumVirtRegs(GetGraph()->GetVRegsCount());
    EXPECT_TRUE(GetGraph()->RunPass<Codegen>());
}

TEST_F(LoweringTest, BoundCheck)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();  // array
        CONSTANT(1U, 10U);        // index
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
            INST(10U, Opcode::Add).u64().Inputs(7U, 7U);
            INST(13U, Opcode::SaveState).Inputs(0U, 10U).SrcVregs({0U, 1U});
            INST(11U, Opcode::CallStatic).u64().InputsAutoType(0U, 10U, 13U);
            INST(12U, Opcode::Return).u64().Inputs(11U);
        }
    }

    GetGraph()->RunPass<Lowering>();
    EXPECT_TRUE(INS(0U).HasUsers());
    EXPECT_FALSE(INS(1U).HasUsers());
    EXPECT_TRUE(INS(2U).HasUsers());
    EXPECT_TRUE(INS(3U).HasUsers());
    EXPECT_TRUE(INS(4U).HasUsers());
    EXPECT_FALSE(INS(5U).HasUsers());
    EXPECT_FALSE(INS(6U).HasUsers());
    EXPECT_TRUE(INS(7U).HasUsers());
    EXPECT_FALSE(INS(8U).HasUsers());
    GraphChecker(GetGraph()).Check();
    // Run codegen
    GetGraph()->RunPass<LoopAnalyzer>();
    RegAlloc(GetGraph());
    SetNumVirtRegs(GetGraph()->GetVRegsCount());
    EXPECT_TRUE(GetGraph()->RunPass<Codegen>());
}

TEST_F(LoweringTest, LoadStoreArray)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();  // array
        CONSTANT(1U, 10U);        // index
        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::LoadArray).u64().Inputs(0U, 1U);
            INST(4U, Opcode::Add).u64().Inputs(3U, 3U);
            INST(5U, Opcode::StoreArray).u64().Inputs(0U, 1U, 4U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(9U, Opcode::SaveState).Inputs(0U, 4U).SrcVregs({0U, 1U});
            INST(7U, Opcode::CallStatic).u64().InputsAutoType(0U, 4U, 9U);
            INST(8U, Opcode::Return).u64().Inputs(7U);
        }
    }

    GetGraph()->RunPass<Lowering>();
    EXPECT_TRUE(INS(0U).HasUsers());
    EXPECT_FALSE(INS(1U).HasUsers());
    EXPECT_FALSE(INS(2U).HasUsers());
    EXPECT_FALSE(INS(3U).HasUsers());
    EXPECT_TRUE(INS(4U).HasUsers());
    EXPECT_FALSE(INS(5U).HasUsers());
    GraphChecker(GetGraph()).Check();
    // Run codegen
    GetGraph()->RunPass<LoopAnalyzer>();
    RegAlloc(GetGraph());
    SetNumVirtRegs(GetGraph()->GetVRegsCount());
    EXPECT_TRUE(GetGraph()->RunPass<Codegen>());
}

TEST_F(LoweringTest, Return)
{
    ReturnTest<int64_t>(10U, DataType::INT64);
    ReturnTest<float>(10.0F, DataType::FLOAT32);
    ReturnTest<double>(10.0_D, DataType::FLOAT64);
    ReturnTest<int64_t>(0U, DataType::INT64);
    ReturnTest<float>(0.0F, DataType::FLOAT32);
    ReturnTest<double>(0.0, DataType::FLOAT64);
}

TEST_F(LoweringTest, If)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint <= ConditionCode::CC_LAST; ccint++) {
        auto cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateEmptyLowLevelGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).u64();
            PARAMETER(1U, 1U).u64();
            CONSTANT(2U, 0U);
            CONSTANT(3U, 1U);
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(4U, Opcode::Compare).b().CC(cc).Inputs(0U, 1U);
                INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(6U, Opcode::Return).b().Inputs(3U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(7U, Opcode::Return).b().Inputs(2U);
            }
        }

        EXPECT_TRUE(graph->RunPass<Lowering>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());

        auto graphIf = CreateEmptyGraph();
        GRAPH(graphIf)
        {
            PARAMETER(0U, 0U).u64();
            PARAMETER(1U, 1U).u64();
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(2U, Opcode::If).SrcType(DataType::UINT64).CC(cc).Inputs(0U, 1U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(3U, Opcode::ReturnI).b().Imm(1U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(4U, Opcode::ReturnI).b().Imm(0U);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph, graphIf));
    }
}

Graph *LoweringTest::BuildGraphIf1(ConditionCode cc)
{
    auto graph = CreateEmptyLowLevelGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        CONSTANT(2U, 0U);
        CONSTANT(3U, 1U);
        CONSTANT(10U, 2U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::Compare).b().CC(cc).Inputs(0U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 5U, 6U)
        {
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(7U, Opcode::Return).s32().Inputs(2U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(8U, Opcode::Return).s32().Inputs(3U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(9U, Opcode::Return).s32().Inputs(10U);
        }
    }
    return graph;
}

TEST_F(LoweringTest, If1)
{
    // Applied
    // Compare have several IfImm users.
    for (int ccint = ConditionCode::CC_FIRST; ccint <= ConditionCode::CC_LAST; ccint++) {
        auto cc = static_cast<ConditionCode>(ccint);
        auto graph = BuildGraphIf1(cc);

        EXPECT_TRUE(graph->RunPass<Lowering>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());

        auto graphIf = CreateEmptyGraph();
        GRAPH(graphIf)
        {
            PARAMETER(0U, 0U).u64();
            PARAMETER(1U, 1U).u64();
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(2U, Opcode::If).SrcType(DataType::UINT64).CC(cc).Inputs(0U, 1U);
            }
            BASIC_BLOCK(3U, 5U, 6U)
            {
                INST(3U, Opcode::If).SrcType(DataType::UINT64).CC(cc).Inputs(0U, 1U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(4U, Opcode::ReturnI).s32().Imm(0U);
            }
            BASIC_BLOCK(5U, -1L)
            {
                INST(5U, Opcode::ReturnI).s32().Imm(1U);
            }
            BASIC_BLOCK(6U, -1L)
            {
                INST(6U, Opcode::ReturnI).s32().Imm(2U);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph, graphIf));
    }
}

Graph *LoweringTest::BuildGraphIf2(ConditionCode cc)
{
    auto graph = CreateEmptyLowLevelGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        CONSTANT(2U, 0U);
        CONSTANT(3U, 1U);
        CONSTANT(10U, 2U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::Compare).b().CC(cc).Inputs(0U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 5U, 6U)
        {
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(11U, Opcode::CallStatic).b().InputsAutoType(4U, 20U);
            INST(7U, Opcode::Return).s32().Inputs(2U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(8U, Opcode::Return).s32().Inputs(3U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(9U, Opcode::Return).s32().Inputs(10U);
        }
    }
    return graph;
}

TEST_F(LoweringTest, If2)
{
    // Not applied
    // Compare have several users, not only IfImm.
    for (int ccint = ConditionCode::CC_FIRST; ccint <= ConditionCode::CC_LAST; ccint++) {
        auto cc = static_cast<ConditionCode>(ccint);
        auto graph = BuildGraphIf2(cc);

        EXPECT_TRUE(graph->RunPass<Lowering>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());

        auto graphIf = CreateEmptyGraph();
        GRAPH(graphIf)
        {
            PARAMETER(0U, 0U).u64();
            PARAMETER(1U, 1U).u64();
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(4U, Opcode::Compare).b().CC(cc).Inputs(0U, 1U);
                INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
            }
            BASIC_BLOCK(3U, 5U, 6U)
            {
                INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(20U, Opcode::SaveState).NoVregs();
                INST(11U, Opcode::CallStatic).b().InputsAutoType(4U, 20U);
                INST(7U, Opcode::ReturnI).s32().Imm(0U);
            }
            BASIC_BLOCK(5U, -1L)
            {
                INST(8U, Opcode::ReturnI).s32().Imm(1U);
            }
            BASIC_BLOCK(6U, -1L)
            {
                INST(9U, Opcode::ReturnI).s32().Imm(2U);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph, graphIf));
    }
}

Graph *LoweringTest::BuildGraphIfRef()
{
    auto graph = CreateEmptyLowLevelGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 2U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().CC(CC_NE).Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 4U) {}
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(5U, Opcode::Phi).u64().Inputs(1U, 2U);
            INST(6U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::CallStatic).v0id().InputsAutoType(5U, 6U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(9U, Opcode::ReturnI).s32().Imm(1U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(10U, Opcode::ReturnI).s32().Imm(1U);
        }
    }
    return graph;
}

TEST_F(LoweringTest, IfRef)
{
    auto graph = BuildGraphIfRef();
    EXPECT_TRUE(graph->RunPass<Lowering>());

    auto graphIf = CreateEmptyGraph();
    GRAPH(graphIf)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 2U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().CC(CC_NE).Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::REFERENCE).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(3U, 4U) {}
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(5U, Opcode::Phi).u64().Inputs(1U, 2U);
            INST(6U, Opcode::SaveState).Inputs(0U).SrcVregs({VirtualRegister::BRIDGE});
            INST(7U, Opcode::CallStatic).v0id().InputsAutoType(5U, 6U);
            INST(8U, Opcode::IfImm).SrcType(DataType::REFERENCE).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(9U, Opcode::ReturnI).s32().Imm(1U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(10U, Opcode::ReturnI).s32().Imm(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, graphIf));
}

TEST_F(LoweringTest, IfFcmpl)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint <= ConditionCode::CC_GE; ++ccint) {
        auto cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateEmptyLowLevelGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).f64();
            PARAMETER(1U, 1U).f64();
            CONSTANT(2U, 0U);
            CONSTANT(3U, 1U);

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(4U, Opcode::Cmp).s32().SrcType(DataType::FLOAT64).Fcmpg(false).Inputs(0U, 1U);
                INST(5U, Opcode::Compare).b().CC(cc).Inputs(4U, 2U);
                INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(7U, Opcode::Return).b().Inputs(3U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(8U, Opcode::Return).b().Inputs(2U);
            }
        }

        EXPECT_TRUE(graph->RunPass<Lowering>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());

        auto graphIf = CreateEmptyGraph();
        GRAPH(graphIf)
        {
            PARAMETER(0U, 0U).f64();
            PARAMETER(1U, 1U).f64();

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(2U, Opcode::If).SrcType(DataType::FLOAT64).CC(cc).Inputs(0U, 1U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(3U, Opcode::ReturnI).b().Imm(1U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(4U, Opcode::ReturnI).b().Imm(0U);
            }
        }

        ASSERT_TRUE(GraphComparator().Compare(graph, graphIf));
    }
}

TEST_F(LoweringTest, IfFcmplNoJoin)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint <= ConditionCode::CC_GE; ++ccint) {
        auto cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateEmptyLowLevelGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).f64();
            PARAMETER(1U, 1U).f64();
            CONSTANT(2U, 0U);
            CONSTANT(3U, 1U);

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(4U, Opcode::Cmp).s32().SrcType(DataType::FLOAT64).Fcmpg(false).Inputs(0U, 1U);
                INST(5U, Opcode::Compare).b().CC(cc).Inputs(4U, 3U);
                INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(7U, Opcode::Return).b().Inputs(3U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(8U, Opcode::Return).b().Inputs(2U);
            }
        }

        EXPECT_TRUE(graph->RunPass<Lowering>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());

        auto graphIf = CreateEmptyGraph();
        GRAPH(graphIf)
        {
            PARAMETER(0U, 0U).f64();
            PARAMETER(1U, 1U).f64();

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(4U, Opcode::Cmp).s32().SrcType(DataType::FLOAT64).Fcmpg(false).Inputs(0U, 1U);
                INST(5U, Opcode::IfImm).SrcType(DataType::INT32).CC(cc).Imm(1U).Inputs(4U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(7U, Opcode::ReturnI).b().Imm(1U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(8U, Opcode::ReturnI).b().Imm(0U);
            }
        }

        ASSERT_TRUE(GraphComparator().Compare(graph, graphIf));
    }
}

Graph *LoweringTest::BuildGraphIfFcmplNoJoin2(ConditionCode cc)
{
    auto graph = CreateEmptyLowLevelGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f64();
        PARAMETER(1U, 1U).f64();
        CONSTANT(2U, 0U);
        CONSTANT(3U, 1U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::Cmp).s32().SrcType(DataType::FLOAT64).Fcmpg(false).Inputs(0U, 1U);
            INST(10U, Opcode::Add).s32().Inputs(4U, 3U);
            INST(5U, Opcode::Compare).b().CC(cc).Inputs(4U, 2U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(7U, Opcode::Return).b().Inputs(3U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(8U, Opcode::Return).b().Inputs(10U);
        }
    }
    return graph;
}

TEST_F(LoweringTest, IfFcmplNoJoin2)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint <= ConditionCode::CC_GE; ++ccint) {
        auto cc = static_cast<ConditionCode>(ccint);
        auto graph = BuildGraphIfFcmplNoJoin2(cc);

        EXPECT_TRUE(graph->RunPass<Lowering>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());

        auto graphIf = CreateEmptyGraph();
        GRAPH(graphIf)
        {
            PARAMETER(0U, 0U).f64();
            PARAMETER(1U, 1U).f64();

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(4U, Opcode::Cmp).s32().SrcType(DataType::FLOAT64).Fcmpg(false).Inputs(0U, 1U);
                INST(10U, Opcode::AddI).s32().Imm(1U).Inputs(4U);
                INST(5U, Opcode::IfImm).SrcType(DataType::INT32).CC(cc).Imm(0U).Inputs(4U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(7U, Opcode::ReturnI).b().Imm(1U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(8U, Opcode::Return).b().Inputs(10U);
            }
        }

        ASSERT_TRUE(GraphComparator().Compare(graph, graphIf));
    }
}

TEST_F(LoweringTest, IfFcmpg)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint <= ConditionCode::CC_GE; ++ccint) {
        auto cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateEmptyLowLevelGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).f64();
            PARAMETER(1U, 1U).f64();
            CONSTANT(2U, 0U);
            CONSTANT(3U, 1U);

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(4U, Opcode::Cmp).s32().SrcType(DataType::FLOAT64).Fcmpg(true).Inputs(0U, 1U);
                INST(5U, Opcode::Compare).b().CC(cc).Inputs(4U, 2U);
                INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(7U, Opcode::Return).b().Inputs(3U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(8U, Opcode::Return).b().Inputs(2U);
            }
        }

        EXPECT_TRUE(graph->RunPass<Lowering>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());

        auto graphIf = CreateEmptyGraph();
        GRAPH(graphIf)
        {
            PARAMETER(0U, 0U).f64();
            PARAMETER(1U, 1U).f64();

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(2U, Opcode::If).SrcType(DataType::FLOAT64).CC(InverseSignednessConditionCode(cc)).Inputs(0U, 1U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(3U, Opcode::ReturnI).b().Imm(1U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(4U, Opcode::ReturnI).b().Imm(0U);
            }
        }

        ASSERT_TRUE(GraphComparator().Compare(graph, graphIf));
    }
}

TEST_F(LoweringTest, IfAnd)
{
    std::array<std::array<ConditionCode, 2U>, 2U> codes = {
        {{ConditionCode::CC_EQ, ConditionCode::CC_TST_EQ}, {ConditionCode::CC_NE, ConditionCode::CC_TST_NE}}};
    for (auto cc : codes) {
        auto graph = CreateEmptyLowLevelGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).b();
            PARAMETER(1U, 1U).b();
            CONSTANT(2U, 0U);
            CONSTANT(3U, 1U);
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(4U, Opcode::And).b().Inputs(0U, 1U);
                INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(cc[0U]).Imm(0U).Inputs(4U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(6U, Opcode::Return).b().Inputs(3U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(7U, Opcode::Return).b().Inputs(2U);
            }
        }

        EXPECT_TRUE(graph->RunPass<Lowering>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());

        auto graphIf = CreateEmptyGraph();
        GRAPH(graphIf)
        {
            PARAMETER(0U, 0U).b();
            PARAMETER(1U, 1U).b();
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(2U, Opcode::If).SrcType(DataType::BOOL).CC(cc[1U]).Inputs(0U, 1U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(3U, Opcode::ReturnI).b().Imm(1U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(4U, Opcode::ReturnI).b().Imm(0U);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph, graphIf));
    }
}

Graph *LoweringTest::BuildGraphMultiplyAddInteger(const std::array<DataType::Type, 4U> &types)
{
    auto type = types[0U];
    auto graph = CreateEmptyLowLevelGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).type(types[1U]);
        PARAMETER(1U, 1U).type(types[2U]);
        PARAMETER(2U, 2U).type(types[3U]);

        BASIC_BLOCK(2U, -1L)
        {
            // a * b + c
            INST(3U, Opcode::Mul).type(type).Inputs(0U, 1U);
            INST(4U, Opcode::Add).type(type).Inputs(3U, 2U);

            // c + a * b
            INST(5U, Opcode::Mul).type(type).Inputs(1U, 2U);
            INST(6U, Opcode::Add).type(type).Inputs(0U, 5U);

            // a * b + c, but a * b is reused
            INST(7U, Opcode::Mul).type(type).Inputs(0U, 1U);
            INST(8U, Opcode::Add).type(type).Inputs(7U, 2U);

            INST(9U, Opcode::Add).type(type).Inputs(4U, 6U);
            INST(10U, Opcode::Add).type(type).Inputs(9U, 8U);
            INST(11U, Opcode::Add).type(type).Inputs(10U, 7U);
            INST(12U, Opcode::Return).type(type).Inputs(11U);
        }
    }
    return graph;
}

Graph *LoweringTest::BuildExpectedMultiplyAddInteger(const std::array<DataType::Type, 4U> &types)
{
    auto type = types[0U];
    auto graphMadd = CreateEmptyGraph();
    GRAPH(graphMadd)
    {
        PARAMETER(0U, 0U).type(types[1U]);
        PARAMETER(1U, 1U).type(types[2U]);
        PARAMETER(2U, 2U).type(types[3U]);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::MAdd).type(type).Inputs(0U, 1U, 2U);
            INST(4U, Opcode::MAdd).type(type).Inputs(1U, 2U, 0U);

            INST(5U, Opcode::Mul).type(type).Inputs(0U, 1U);
            INST(6U, Opcode::Add).type(type).Inputs(5U, 2U);

            INST(7U, Opcode::Add).type(type).Inputs(3U, 4U);
            INST(8U, Opcode::Add).type(type).Inputs(7U, 6U);
            INST(9U, Opcode::Add).type(type).Inputs(8U, 5U);
            INST(10U, Opcode::Return).type(type).Inputs(9U);
        }
    }
    return graphMadd;
}

TEST_F(LoweringTest, MultiplyAddInteger)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-add instruction is only supported on Aarch64";
    }

    std::initializer_list<std::array<DataType::Type, 4U>> typeCombinations = {
        {DataType::UINT32, DataType::UINT32, DataType::UINT32, DataType::UINT32},
        {DataType::INT32, DataType::INT32, DataType::INT32, DataType::INT32},
        {DataType::INT32, DataType::INT16, DataType::INT8, DataType::INT16},
        {DataType::UINT64, DataType::UINT64, DataType::UINT64, DataType::UINT64},
        {DataType::INT64, DataType::INT64, DataType::INT64, DataType::INT64}};

    for (auto &types : typeCombinations) {
        auto graph = BuildGraphMultiplyAddInteger(types);

        EXPECT_TRUE(graph->RunPass<Lowering>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());

        auto graphMadd = BuildExpectedMultiplyAddInteger(types);

        ASSERT_TRUE(GraphComparator().Compare(graph, graphMadd));
    }
}

TEST_F(LoweringTest, MultiplyAddWithIncompatibleInstructionTypes)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-add instruction is only supported on Aarch64";
    }
    auto graph = CreateEmptyLowLevelGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(1U, 1U).u16();
        PARAMETER(2U, 2U).u16();

        BASIC_BLOCK(2U, -1L)
        {
            // a + b * c
            INST(3U, Opcode::Mul).u16().Inputs(1U, 2U);
            INST(4U, Opcode::Add).u32().Inputs(3U, 0U);
            INST(5U, Opcode::Return).u32().Inputs(4U);
        }
    }
    auto clone = GraphCloner(graph, GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_TRUE(graph->RunPass<Lowering>());
    ASSERT_TRUE(GraphComparator().Compare(graph, clone));
}

Graph *LoweringTest::BuildGraphMultiplySubInteger(const std::array<DataType::Type, 4U> &types)
{
    auto type = types[0U];
    auto graph = CreateEmptyLowLevelGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).type(types[1U]);
        PARAMETER(1U, 1U).type(types[2U]);
        PARAMETER(2U, 2U).type(types[3U]);

        BASIC_BLOCK(2U, -1L)
        {
            // c - a * b
            INST(3U, Opcode::Mul).type(type).Inputs(0U, 1U);
            INST(4U, Opcode::Sub).type(type).Inputs(2U, 3U);

            // (- a * b) + c
            INST(5U, Opcode::Mul).type(type).Inputs(0U, 1U);
            INST(6U, Opcode::Neg).type(type).Inputs(5U);
            INST(7U, Opcode::Add).type(type).Inputs(6U, 2U);

            // c + (-a) * b
            INST(8U, Opcode::Neg).type(type).Inputs(0U);
            INST(9U, Opcode::Mul).type(type).Inputs(8U, 1U);
            INST(10U, Opcode::Add).type(type).Inputs(9U, 2U);

            // c + a * (-b)
            INST(11U, Opcode::Neg).type(type).Inputs(1U);
            INST(12U, Opcode::Mul).type(type).Inputs(0U, 11U);
            INST(13U, Opcode::Add).type(type).Inputs(12U, 2U);

            // c - a * b, but a * b is reused
            INST(14U, Opcode::Mul).type(type).Inputs(0U, 1U);
            INST(15U, Opcode::Sub).type(type).Inputs(2U, 14U);

            INST(16U, Opcode::Add).type(type).Inputs(4U, 7U);
            INST(17U, Opcode::Add).type(type).Inputs(16U, 10U);
            INST(18U, Opcode::Add).type(type).Inputs(17U, 13U);
            INST(19U, Opcode::Add).type(type).Inputs(18U, 15U);
            INST(20U, Opcode::Add).type(type).Inputs(19U, 14U);
            INST(21U, Opcode::Return).type(type).Inputs(20U);
        }
    }
    return graph;
}

Graph *LoweringTest::BuildExpectedMultiplySubInteger(const std::array<DataType::Type, 4U> &types)
{
    auto type = types[0U];
    auto graphMsub = CreateEmptyGraph();
    GRAPH(graphMsub)
    {
        PARAMETER(0U, 0U).type(types[1U]);
        PARAMETER(1U, 1U).type(types[2U]);
        PARAMETER(2U, 2U).type(types[3U]);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::MSub).type(type).Inputs(0U, 1U, 2U);
            INST(4U, Opcode::MSub).type(type).Inputs(0U, 1U, 2U);
            INST(5U, Opcode::MSub).type(type).Inputs(0U, 1U, 2U);
            // add(mul(0, neg(1)), 2) -> add(mneg(1, 0), 2)
            INST(6U, Opcode::MSub).type(type).Inputs(1U, 0U, 2U);

            INST(7U, Opcode::Mul).type(type).Inputs(0U, 1U);
            INST(8U, Opcode::Sub).type(type).Inputs(2U, 7U);

            INST(9U, Opcode::Add).type(type).Inputs(3U, 4U);
            INST(10U, Opcode::Add).type(type).Inputs(9U, 5U);
            INST(11U, Opcode::Add).type(type).Inputs(10U, 6U);
            INST(12U, Opcode::Add).type(type).Inputs(11U, 8U);
            INST(13U, Opcode::Add).type(type).Inputs(12U, 7U);
            INST(14U, Opcode::Return).type(type).Inputs(13U);
        }
    }
    return graphMsub;
}

TEST_F(LoweringTest, MultiplySubInteger)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-subtract instruction is only supported on Aarch64";
    }

    std::initializer_list<std::array<DataType::Type, 4U>> typeCombinations = {
        {DataType::INT32, DataType::INT32, DataType::INT32, DataType::INT32},
        {DataType::INT32, DataType::INT16, DataType::INT8, DataType::INT16},
        {DataType::INT64, DataType::INT64, DataType::INT64, DataType::INT64}};

    for (auto &types : typeCombinations) {
        auto graph = BuildGraphMultiplySubInteger(types);
        EXPECT_TRUE(graph->RunPass<Lowering>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());

        auto graphMsub = BuildExpectedMultiplySubInteger(types);

        ASSERT_TRUE(GraphComparator().Compare(graph, graphMsub));
    }
}

TEST_F(LoweringTest, MultiplySubIntegerWithIncompatibleInstructionTypes)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-subtract instruction is only supported on Aarch64";
    }

    auto graph = CreateEmptyLowLevelGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u8();
        PARAMETER(1U, 1U).u8();
        PARAMETER(2U, 2U).u32();

        BASIC_BLOCK(2U, -1L)
        {
            // c - a * b
            INST(3U, Opcode::Mul).u16().Inputs(0U, 1U);
            INST(4U, Opcode::Sub).u32().Inputs(2U, 3U);

            // (- a * b) + c
            INST(5U, Opcode::Mul).u16().Inputs(0U, 1U);
            INST(6U, Opcode::Neg).u32().Inputs(5U);
            INST(7U, Opcode::Add).u32().Inputs(6U, 2U);

            // c + (-a) * b
            INST(8U, Opcode::Neg).u8().Inputs(0U);
            INST(9U, Opcode::Mul).u16().Inputs(8U, 1U);
            INST(10U, Opcode::Add).u32().Inputs(9U, 2U);

            // c + a * (-b)
            INST(11U, Opcode::Neg).u8().Inputs(1U);
            INST(12U, Opcode::Mul).u16().Inputs(0U, 11U);
            INST(13U, Opcode::Add).u32().Inputs(12U, 2U);

            // c - a * b, but a * b is reused
            INST(14U, Opcode::Mul).u16().Inputs(0U, 1U);
            INST(15U, Opcode::Sub).u32().Inputs(2U, 14U);

            INST(16U, Opcode::Add).u32().Inputs(4U, 7U);
            INST(17U, Opcode::Add).u32().Inputs(16U, 10U);
            INST(18U, Opcode::Add).u32().Inputs(17U, 13U);
            INST(19U, Opcode::Add).u32().Inputs(18U, 15U);
            INST(20U, Opcode::Add).u32().Inputs(19U, 14U);
            INST(21U, Opcode::Return).u32().Inputs(20U);
        }
    }

    auto clone = GraphCloner(graph, GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    EXPECT_TRUE(graph->RunPass<Lowering>());
    ASSERT_TRUE(GraphComparator().Compare(graph, clone));
}

TEST_F(LoweringTest, MultiplyAddSubFloat)
{
    std::array<Graph *, 2U> graphs {CreateEmptyLowLevelGraph(), CreateEmptyLowLevelGraph()};
    for (auto &graph : graphs) {
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).f64();
            PARAMETER(1U, 1U).f64();
            PARAMETER(2U, 2U).f64();

            BASIC_BLOCK(2U, -1L)
            {
                INST(3U, Opcode::Mul).f64().Inputs(0U, 1U);
                INST(4U, Opcode::Add).f64().Inputs(3U, 2U);

                INST(5U, Opcode::Mul).f64().Inputs(0U, 1U);
                INST(6U, Opcode::Sub).f64().Inputs(2U, 5U);

                INST(7U, Opcode::Add).f64().Inputs(4U, 6U);
                INST(8U, Opcode::Return).f64().Inputs(7U);
            }
        }
    }

    EXPECT_TRUE(graphs[0U]->RunPass<Lowering>());
    ASSERT_TRUE(GraphComparator().Compare(graphs[0U], graphs[1U]));
}

Graph *LoweringTest::BuildGraphMultiplyNegate(const TypeTriple &types)
{
    auto type = types[0U];
    auto graph = CreateEmptyLowLevelGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).type(types[1U]);
        PARAMETER(1U, 1U).type(types[2U]);

        BASIC_BLOCK(2U, -1L)
        {
            // - (a * b)
            INST(3U, Opcode::Mul).type(type).Inputs(0U, 1U);
            INST(4U, Opcode::Neg).type(type).Inputs(3U);

            // a * (-b)
            INST(5U, Opcode::Neg).type(type).Inputs(1U);
            INST(6U, Opcode::Mul).type(type).Inputs(0U, 5U);

            // (-a) * b
            INST(7U, Opcode::Neg).type(type).Inputs(0U);
            INST(8U, Opcode::Mul).type(type).Inputs(7U, 1U);

            // - (a * b), but mul result is reused
            INST(9U, Opcode::Mul).type(type).Inputs(0U, 1U);
            INST(10U, Opcode::Neg).type(type).Inputs(9U);

            // (-a) * b, but neg result is reused
            INST(11U, Opcode::Neg).type(type).Inputs(0U);
            INST(12U, Opcode::Mul).type(type).Inputs(11U, 1U);

            INST(13U, Opcode::Max).type(type).Inputs(4U, 6U);
            INST(14U, Opcode::Max).type(type).Inputs(13U, 8U);
            INST(15U, Opcode::Max).type(type).Inputs(14U, 10U);
            INST(16U, Opcode::Max).type(type).Inputs(15U, 12U);
            INST(17U, Opcode::Max).type(type).Inputs(16U, 9U);
            INST(18U, Opcode::Max).type(type).Inputs(17U, 11U);
            INST(19U, Opcode::Return).type(type).Inputs(18U);
        }
    }
    return graph;
}

Graph *LoweringTest::BuildExpectedMultiplyNegate(const TypeTriple &types)
{
    auto type = types[0U];
    auto graphExpected = CreateEmptyGraph();
    GRAPH(graphExpected)
    {
        PARAMETER(0U, 0U).type(types[1U]);
        PARAMETER(1U, 1U).type(types[2U]);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::MNeg).type(type).Inputs(0U, 1U);
            INST(4U, Opcode::MNeg).type(type).Inputs(1U, 0U);
            INST(5U, Opcode::MNeg).type(type).Inputs(0U, 1U);
            INST(6U, Opcode::Mul).type(type).Inputs(0U, 1U);
            INST(7U, Opcode::Neg).type(type).Inputs(6U);
            INST(8U, Opcode::Neg).type(type).Inputs(0U);
            INST(9U, Opcode::Mul).type(type).Inputs(8U, 1U);

            INST(10U, Opcode::Max).type(type).Inputs(3U, 4U);
            INST(11U, Opcode::Max).type(type).Inputs(10U, 5U);
            INST(12U, Opcode::Max).type(type).Inputs(11U, 7U);
            INST(13U, Opcode::Max).type(type).Inputs(12U, 9U);
            INST(14U, Opcode::Max).type(type).Inputs(13U, 6U);
            INST(15U, Opcode::Max).type(type).Inputs(14U, 8U);
            INST(16U, Opcode::Return).type(type).Inputs(15U);
        }
    }
    return graphExpected;
}

TEST_F(LoweringTest, MultiplyNegate)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-negate instruction is only supported on Aarch64";
    }

    std::initializer_list<TypeTriple> typeCombinations = {{DataType::FLOAT32, DataType::FLOAT32, DataType::FLOAT32},
                                                          {DataType::FLOAT64, DataType::FLOAT64, DataType::FLOAT64},
                                                          {DataType::INT32, DataType::INT32, DataType::INT32},
                                                          {DataType::INT32, DataType::INT16, DataType::INT8},
                                                          {DataType::INT64, DataType::INT64, DataType::INT64}};
    for (auto &types : typeCombinations) {
        auto graph = BuildGraphMultiplyNegate(types);

        EXPECT_TRUE(graph->RunPass<Lowering>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());

        auto graphExpected = BuildExpectedMultiplyNegate(types);

        ASSERT_TRUE(GraphComparator().Compare(graph, graphExpected));
    }
}

TEST_F(LoweringTest, MultiplyNegateWithIncompatibleInstructionTypes)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "multiply-negate instruction is only supported on Aarch64";
    }

    auto graph = CreateEmptyLowLevelGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s16();
        PARAMETER(1U, 1U).s16();

        BASIC_BLOCK(2U, -1L)
        {
            // - (a * b)
            INST(3U, Opcode::Mul).s16().Inputs(0U, 1U);
            INST(4U, Opcode::Neg).s32().Inputs(3U);

            // a * (-b)
            INST(5U, Opcode::Neg).s16().Inputs(1U);
            INST(6U, Opcode::Mul).s32().Inputs(0U, 5U);

            // (-a) * b
            INST(7U, Opcode::Neg).s16().Inputs(0U);
            INST(8U, Opcode::Mul).s32().Inputs(7U, 1U);

            // - (a * b), but mul result is reused
            INST(9U, Opcode::Mul).s16().Inputs(0U, 1U);
            INST(10U, Opcode::Neg).s32().Inputs(9U);

            // (-a) * b, but neg result is reused
            INST(11U, Opcode::Neg).s16().Inputs(0U);
            INST(12U, Opcode::Mul).s32().Inputs(11U, 1U);

            INST(13U, Opcode::Max).s32().Inputs(4U, 6U);
            INST(14U, Opcode::Max).s32().Inputs(13U, 8U);
            INST(15U, Opcode::Max).s32().Inputs(14U, 10U);
            INST(16U, Opcode::Max).s32().Inputs(15U, 12U);
            INST(17U, Opcode::Max).s32().Inputs(16U, 9U);
            INST(18U, Opcode::Max).s32().Inputs(17U, 11U);
            INST(19U, Opcode::Return).s32().Inputs(18U);
        }
    }

    auto clone = GraphCloner(graph, GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    EXPECT_TRUE(graph->RunPass<Lowering>());
    ASSERT_TRUE(GraphComparator().Compare(graph, clone));
}

void LoweringTest::TestBitwiseBinaryOpWithInvertedOperand(const TypeTriple &types, OpcodePair ops)
{
    auto type = types[0U];
    auto graph = CreateEmptyLowLevelGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).type(types[1U]);
        PARAMETER(1U, 1U).type(types[2U]);

        BASIC_BLOCK(2U, -1L)
        {
            // ~a op b
            INST(3U, Opcode::Not).type(type).Inputs(0U);
            INST(4U, ops.first).type(type).Inputs(3U, 1U);
            // a op ~b
            INST(5U, Opcode::Not).type(type).Inputs(1U);
            INST(6U, ops.first).type(type).Inputs(0U, 5U);
            // ~a op b, but ~a is reused
            INST(7U, Opcode::Not).type(type).Inputs(0U);
            INST(8U, ops.first).type(type).Inputs(7U, 1U);

            INST(9U, Opcode::Add).type(type).Inputs(4U, 6U);
            INST(10U, Opcode::Add).type(type).Inputs(9U, 8U);
            INST(11U, Opcode::Add).type(type).Inputs(10U, 7U);
            INST(12U, Opcode::Return).type(type).Inputs(11U);
        }
    }

    EXPECT_TRUE(graph->RunPass<Lowering>());
    EXPECT_TRUE(graph->RunPass<Cleanup>());

    auto graphExpected = CreateEmptyGraph();
    GRAPH(graphExpected)
    {
        PARAMETER(0U, 0U).type(types[1U]);
        PARAMETER(1U, 1U).type(types[2U]);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, ops.second).type(type).Inputs(1U, 0U);
            INST(4U, ops.second).type(type).Inputs(0U, 1U);
            INST(5U, Opcode::Not).type(type).Inputs(0U);
            INST(6U, ops.first).type(type).Inputs(5U, 1U);

            INST(7U, Opcode::Add).type(type).Inputs(3U, 4U);
            INST(8U, Opcode::Add).type(type).Inputs(7U, 6U);
            INST(9U, Opcode::Add).type(type).Inputs(8U, 5U);
            INST(10U, Opcode::Return).type(type).Inputs(9U);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(graph, graphExpected));
}

TEST_F(LoweringTest, BitwiseBinaryOpWithInvertedOperand)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "xor-not, or-not and and-not instructions are only supported on Aarch64";
    }

    std::initializer_list<OpcodePair> opcodes = {
        {Opcode::And, Opcode::AndNot}, {Opcode::Or, Opcode::OrNot}, {Opcode::Xor, Opcode::XorNot}};
    std::initializer_list<TypeTriple> typeCombinations = {{DataType::INT32, DataType::INT32, DataType::INT32},
                                                          {DataType::UINT32, DataType::UINT32, DataType::UINT32},
                                                          {DataType::UINT32, DataType::UINT16, DataType::UINT8},
                                                          {DataType::INT64, DataType::INT64, DataType::INT64},
                                                          {DataType::UINT64, DataType::UINT64, DataType::UINT64}};
    for (const auto &types : typeCombinations) {
        for (const auto &ops : opcodes) {
            TestBitwiseBinaryOpWithInvertedOperand(types, ops);
        }
    }
}

TEST_F(LoweringTest, BitwiseBinaryOpWithInvertedOperandWitnIncompatibleInstructionTypes)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "xor-not, or-not and and-not instructions are only supported on Aarch64";
    }

    std::initializer_list<Opcode> opcodes = {Opcode::And, Opcode::Or, Opcode::Xor};
    for (auto &ops : opcodes) {
        auto graph = CreateEmptyLowLevelGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).s16();
            PARAMETER(1U, 1U).s16();

            BASIC_BLOCK(2U, -1L)
            {
                // ~a op b
                INST(3U, Opcode::Not).s16().Inputs(0U);
                INST(4U, ops).s32().Inputs(3U, 1U);
                // a op ~b
                INST(5U, Opcode::Not).s16().Inputs(1U);
                INST(6U, ops).s32().Inputs(0U, 5U);
                // ~a op b, but ~a is reused
                INST(7U, Opcode::Not).s16().Inputs(0U);
                INST(8U, ops).s32().Inputs(7U, 1U);

                INST(9U, Opcode::Add).s32().Inputs(4U, 6U);
                INST(10U, Opcode::Add).s32().Inputs(9U, 8U);
                INST(11U, Opcode::Add).s32().Inputs(10U, 7U);
                INST(12U, Opcode::Return).s32().Inputs(11U);
            }
        }

        auto clone = GraphCloner(graph, GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
        EXPECT_TRUE(graph->RunPass<Lowering>());
        ASSERT_TRUE(GraphComparator().Compare(graph, clone));
    }
}

Graph *LoweringTest::BuildGraphCommutativeBinaryOpWithShiftedOperand(const TypeTriple &types, ShiftOpPair shiftOp,
                                                                     OpcodePair ops)
{
    auto type = types[0U];
    auto graph = CreateEmptyLowLevelGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).type(types[1U]);
        PARAMETER(1U, 1U).type(types[2U]);
        CONSTANT(2U, 3U);
        CONSTANT(3U, 9U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, std::get<0U>(shiftOp)).type(type).Inputs(0U, 2U);
            INST(5U, ops.first).type(type).Inputs(1U, 4U);

            INST(6U, std::get<0U>(shiftOp)).type(type).Inputs(1U, 2U);
            INST(7U, ops.first).type(type).Inputs(6U, 1U);

            INST(8U, std::get<0U>(shiftOp)).type(type).Inputs(1U, 2U);
            INST(9U, ops.first).type(type).Inputs(8U, 3U);

            INST(10U, std::get<0U>(shiftOp)).type(type).Inputs(0U, 2U);
            INST(11U, ops.first).type(type).Inputs(1U, 10U);

            INST(12U, Opcode::Max).type(type).Inputs(5U, 7U);
            INST(13U, Opcode::Max).type(type).Inputs(12U, 9U);
            INST(14U, Opcode::Max).type(type).Inputs(13U, 11U);
            INST(15U, Opcode::Max).type(type).Inputs(14U, 10U);
            INST(16U, Opcode::Return).type(type).Inputs(15U);
        }
    }
    return graph;
}

Graph *LoweringTest::BuildExpectedCommutativeBinaryOpWithShiftedOperand(const TypeTriple &types, ShiftOpPair shiftOp,
                                                                        OpcodePair ops)
{
    auto type = types[0];
    auto graphExpected = CreateEmptyGraph();
    GRAPH(graphExpected)
    {
        PARAMETER(0U, 0U).type(types[1U]);
        PARAMETER(1U, 1U).type(types[2U]);
        CONSTANT(2U, 9U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, ops.second).Shift(std::get<2U>(shiftOp), 3U).type(type).Inputs(1U, 0U);
            INST(4U, ops.second).Shift(std::get<2U>(shiftOp), 3U).type(type).Inputs(1U, 1U);
            INST(5U, ops.second).Shift(std::get<2U>(shiftOp), 3U).type(type).Inputs(2U, 1U);

            INST(6U, std::get<1U>(shiftOp)).Imm(3U).type(type).Inputs(0U);
            INST(7U, ops.first).type(type).Inputs(1U, 6U);

            INST(8U, Opcode::Max).type(type).Inputs(3U, 4U);
            INST(9U, Opcode::Max).type(type).Inputs(8U, 5U);
            INST(10U, Opcode::Max).type(type).Inputs(9U, 7U);
            INST(11U, Opcode::Max).type(type).Inputs(10U, 6U);
            INST(12U, Opcode::Return).type(type).Inputs(11U);
        }
    }
    return graphExpected;
}

TEST_F(LoweringTest, CommutativeBinaryOpWithShiftedOperand)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "instructions with shifted operands are only supported on Aarch64";
    }

    std::initializer_list<OpcodePair> opcodes = {{Opcode::Add, Opcode::AddSR},
                                                 {Opcode::And, Opcode::AndSR},
                                                 {Opcode::Or, Opcode::OrSR},
                                                 {Opcode::Xor, Opcode::XorSR}};

    std::initializer_list<ShiftOpPair> shiftOps = {{Opcode::Shl, Opcode::ShlI, ShiftType::LSL},
                                                   {Opcode::Shr, Opcode::ShrI, ShiftType::LSR},
                                                   {Opcode::AShr, Opcode::AShrI, ShiftType::ASR}};
    std::initializer_list<TypeTriple> typeCombinations = {{DataType::INT32, DataType::INT32, DataType::INT32},
                                                          {DataType::UINT32, DataType::UINT32, DataType::UINT32},
                                                          {DataType::UINT32, DataType::UINT16, DataType::UINT8},
                                                          {DataType::INT64, DataType::INT64, DataType::INT64},
                                                          {DataType::UINT64, DataType::UINT64, DataType::UINT64}};
    for (auto &types : typeCombinations) {
        for (auto &shiftOp : shiftOps) {
            for (auto &ops : opcodes) {
                auto graph = BuildGraphCommutativeBinaryOpWithShiftedOperand(types, shiftOp, ops);
                EXPECT_TRUE(graph->RunPass<Lowering>());
                EXPECT_TRUE(graph->RunPass<Cleanup>());

                auto graphExpected = BuildExpectedCommutativeBinaryOpWithShiftedOperand(types, shiftOp, ops);
                ASSERT_TRUE(GraphComparator().Compare(graph, graphExpected));
            }
        }
    }
}

void LoweringTest::TestCommutativeBinaryOpWithShiftedOperandWithIncompatibleInstructionTypes(ShiftOpPair shiftOp,
                                                                                             Opcode op)
{
    auto graph = CreateEmptyLowLevelGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s16();
        PARAMETER(1U, 1U).s16();
        CONSTANT(2U, 5U);
        PARAMETER(3U, 2U).s16();

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, std::get<0U>(shiftOp)).s16().Inputs(0U, 2U);
            INST(5U, op).s32().Inputs(1U, 4U);

            INST(6U, std::get<0U>(shiftOp)).s16().Inputs(1U, 2U);
            INST(7U, op).s32().Inputs(6U, 1U);

            INST(8U, std::get<0U>(shiftOp)).s16().Inputs(1U, 2U);
            INST(9U, op).s32().Inputs(8U, 3U);

            INST(10U, std::get<0U>(shiftOp)).s16().Inputs(0U, 2U);
            INST(11U, op).s32().Inputs(1U, 10U);

            INST(12U, Opcode::Max).s32().Inputs(5U, 7U);
            INST(13U, Opcode::Max).s32().Inputs(12U, 9U);
            INST(14U, Opcode::Max).s32().Inputs(13U, 11U);
            INST(15U, Opcode::Max).s32().Inputs(14U, 10U);
            INST(16U, Opcode::Return).s32().Inputs(15U);
        }
    }

    auto clone = GraphCloner(graph, GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    EXPECT_TRUE(graph->RunPass<Lowering>());
    ASSERT_TRUE(GraphComparator().Compare(graph, clone));
}

TEST_F(LoweringTest, CommutativeBinaryOpWithShiftedOperandWithIncompatibleInstructionTypes)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "instructions with shifted operands are only supported on Aarch64";
    }

    std::initializer_list<Opcode> opcodes = {Opcode::Add, Opcode::And, Opcode::Or, Opcode::Xor};

    std::initializer_list<ShiftOpPair> shiftOps = {{Opcode::Shl, Opcode::ShlI, ShiftType::LSL},
                                                   {Opcode::Shr, Opcode::ShrI, ShiftType::LSR},
                                                   {Opcode::AShr, Opcode::AShrI, ShiftType::ASR}};
    for (const auto &shiftOp : shiftOps) {
        for (const auto &op : opcodes) {
            TestCommutativeBinaryOpWithShiftedOperandWithIncompatibleInstructionTypes(shiftOp, op);
        }
    }
}

Graph *LoweringTest::BuildGraphSubWithShiftedOperand(TypeTriple types, ShiftOpPair shiftOp)
{
    auto graph = CreateEmptyLowLevelGraph();
    auto type = types[0];
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).type(types[1U]);
        PARAMETER(1U, 1U).type(types[2U]);
        CONSTANT(2U, 5U);
        CONSTANT(3U, 2U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, std::get<0U>(shiftOp)).type(type).Inputs(0U, 2U);
            INST(5U, Opcode::Sub).type(type).Inputs(1U, 4U);

            INST(6U, std::get<0U>(shiftOp)).type(type).Inputs(1U, 2U);
            INST(7U, Opcode::Sub).type(type).Inputs(6U, 1U);

            INST(8U, std::get<0U>(shiftOp)).type(type).Inputs(0U, 2U);
            INST(9U, Opcode::Sub).type(type).Inputs(1U, 8U);

            INST(10U, std::get<0U>(shiftOp)).type(type).Inputs(0U, 2U);
            INST(11U, Opcode::Sub).type(type).Inputs(3U, 10U);

            INST(12U, Opcode::Max).type(type).Inputs(5U, 7U);
            INST(13U, Opcode::Max).type(type).Inputs(12U, 8U);
            INST(14U, Opcode::Max).type(type).Inputs(13U, 9U);
            INST(15U, Opcode::Max).type(type).Inputs(14U, 11U);
            INST(16U, Opcode::Return).type(type).Inputs(15U);
        }
    }
    return graph;
}

void LoweringTest::TestSubWithShiftedOperand(TypeTriple types, ShiftOpPair shiftOp)
{
    auto graph = BuildGraphSubWithShiftedOperand(types, shiftOp);

    EXPECT_TRUE(graph->RunPass<Lowering>());
    EXPECT_TRUE(graph->RunPass<Cleanup>());

    auto graphExpected = CreateEmptyGraph();
    auto type = types[0U];
    GRAPH(graphExpected)
    {
        PARAMETER(0U, 0U).type(types[1U]);
        PARAMETER(1U, 1U).type(types[2U]);
        CONSTANT(2U, 2U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SubSR).Shift(std::get<2U>(shiftOp), 5U).type(type).Inputs(1U, 0U);
            INST(4U, std::get<1U>(shiftOp)).Imm(5U).type(type).Inputs(1U);
            INST(5U, Opcode::Sub).type(type).Inputs(4U, 1U);
            INST(6U, std::get<1U>(shiftOp)).Imm(5U).type(type).Inputs(0U);
            INST(7U, Opcode::Sub).type(type).Inputs(1U, 6U);
            INST(8U, Opcode::SubSR).Shift(std::get<2U>(shiftOp), 5U).type(type).Inputs(2U, 0U);

            INST(9U, Opcode::Max).type(type).Inputs(3U, 5U);
            INST(10U, Opcode::Max).type(type).Inputs(9U, 6U);
            INST(11U, Opcode::Max).type(type).Inputs(10U, 7U);
            INST(12U, Opcode::Max).type(type).Inputs(11U, 8U);
            INST(13U, Opcode::Return).type(type).Inputs(12U);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(graph, graphExpected));
}

TEST_F(LoweringTest, SubWithShiftedOperand)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "instructions with shifted operands are only supported on Aarch64";
    }

    std::initializer_list<ShiftOpPair> shiftOps = {{Opcode::Shl, Opcode::ShlI, ShiftType::LSL},
                                                   {Opcode::Shr, Opcode::ShrI, ShiftType::LSR},
                                                   {Opcode::AShr, Opcode::AShrI, ShiftType::ASR}};
    std::initializer_list<TypeTriple> typeCombinations = {{DataType::INT32, DataType::INT32, DataType::INT32},
                                                          {DataType::UINT32, DataType::UINT32, DataType::UINT32},
                                                          {DataType::UINT32, DataType::UINT16, DataType::UINT8},
                                                          {DataType::INT64, DataType::INT64, DataType::INT64},
                                                          {DataType::UINT64, DataType::UINT64, DataType::UINT64}};
    for (auto &types : typeCombinations) {
        for (auto &shiftOp : shiftOps) {
            TestSubWithShiftedOperand(types, shiftOp);
        }
    }
}

TEST_F(LoweringTest, SubWithShiftedOperandWithIncompatibleTypes)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "instructions with shifted operands are only supported on Aarch64";
    }

    std::initializer_list<ShiftOpPair> shiftOps = {{Opcode::Shl, Opcode::ShlI, ShiftType::LSL},
                                                   {Opcode::Shr, Opcode::ShrI, ShiftType::LSR},
                                                   {Opcode::AShr, Opcode::AShrI, ShiftType::ASR}};
    for (auto &shiftOp : shiftOps) {
        auto graph = CreateEmptyLowLevelGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).s16();
            PARAMETER(1U, 1U).s16();
            CONSTANT(2U, 5U);
            CONSTANT(3U, 2U);

            BASIC_BLOCK(2U, -1L)
            {
                INST(4U, std::get<0U>(shiftOp)).s16().Inputs(0U, 2U);
                INST(5U, Opcode::Sub).s32().Inputs(1U, 4U);

                INST(6U, std::get<0U>(shiftOp)).s16().Inputs(1U, 2U);
                INST(7U, Opcode::Sub).s32().Inputs(6U, 1U);

                INST(8U, std::get<0U>(shiftOp)).s16().Inputs(0U, 2U);
                INST(9U, Opcode::Sub).s32().Inputs(1U, 8U);

                INST(10U, std::get<0U>(shiftOp)).s16().Inputs(0U, 2U);
                INST(11U, Opcode::Sub).s32().Inputs(3U, 10U);

                INST(12U, Opcode::Max).s32().Inputs(5U, 7U);
                INST(13U, Opcode::Max).s32().Inputs(12U, 8U);
                INST(14U, Opcode::Max).s32().Inputs(13U, 9U);
                INST(15U, Opcode::Max).s32().Inputs(14U, 11U);
                INST(16U, Opcode::Return).s32().Inputs(15U);
            }
        }

        auto clone = GraphCloner(graph, GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
        EXPECT_TRUE(graph->RunPass<Lowering>());
        ASSERT_TRUE(GraphComparator().Compare(graph, clone));
    }
}

void LoweringTest::TestNonCommutativeBinaryOpWithShiftedOperand(const TypeTriple &types, const ShiftOpPair &shiftOp,
                                                                OpcodePair ops)
{
    auto type = types[0U];
    auto graph = CreateEmptyLowLevelGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).type(types[1U]);
        PARAMETER(1U, 1U).type(types[2U]);
        CONSTANT(2U, 5U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, std::get<0U>(shiftOp)).type(type).Inputs(0U, 2U);
            INST(4U, ops.first).type(type).Inputs(1U, 3U);

            INST(5U, std::get<0U>(shiftOp)).type(type).Inputs(1U, 2U);
            INST(6U, ops.first).type(type).Inputs(5U, 1U);

            INST(7U, std::get<0U>(shiftOp)).type(type).Inputs(0U, 2U);
            INST(8U, ops.first).type(type).Inputs(1U, 7U);

            INST(9U, Opcode::Max).type(type).Inputs(4U, 6U);
            INST(10U, Opcode::Max).type(type).Inputs(9U, 8U);
            INST(11U, Opcode::Max).type(type).Inputs(10U, 7U);
            INST(12U, Opcode::Return).type(type).Inputs(11U);
        }
    }

    EXPECT_TRUE(graph->RunPass<Lowering>());
    EXPECT_TRUE(graph->RunPass<Cleanup>());

    auto graphExpected = CreateEmptyGraph();
    GRAPH(graphExpected)
    {
        PARAMETER(0U, 0U).type(types[1U]);
        PARAMETER(1U, 1U).type(types[2U]);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, ops.second).Shift(std::get<2U>(shiftOp), 5U).type(type).Inputs(1U, 0U);
            INST(3U, std::get<1U>(shiftOp)).Imm(5U).type(type).Inputs(1U);
            INST(4U, ops.first).type(type).Inputs(3U, 1U);
            INST(5U, std::get<1U>(shiftOp)).Imm(5U).type(type).Inputs(0U);
            INST(6U, ops.first).type(type).Inputs(1U, 5U);

            INST(7U, Opcode::Max).type(type).Inputs(2U, 4U);
            INST(8U, Opcode::Max).type(type).Inputs(7U, 6U);
            INST(9U, Opcode::Max).type(type).Inputs(8U, 5U);
            INST(10U, Opcode::Return).type(type).Inputs(9U);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(graph, graphExpected));
}

TEST_F(LoweringTest, NonCommutativeBinaryOpWithShiftedOperand)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "instructions with shifted operands are only supported on Aarch64";
    }

    std::initializer_list<OpcodePair> opcodes = {{Opcode::Sub, Opcode::SubSR},
                                                 {Opcode::AndNot, Opcode::AndNotSR},
                                                 {Opcode::OrNot, Opcode::OrNotSR},
                                                 {Opcode::XorNot, Opcode::XorNotSR}};

    std::initializer_list<ShiftOpPair> shiftOps = {{Opcode::Shl, Opcode::ShlI, ShiftType::LSL},
                                                   {Opcode::Shr, Opcode::ShrI, ShiftType::LSR},
                                                   {Opcode::AShr, Opcode::AShrI, ShiftType::ASR}};
    std::initializer_list<TypeTriple> typeCombinations = {{DataType::INT32, DataType::INT32, DataType::INT32},
                                                          {DataType::UINT32, DataType::UINT32, DataType::UINT32},
                                                          {DataType::UINT32, DataType::UINT16, DataType::UINT8},
                                                          {DataType::INT64, DataType::INT64, DataType::INT64},
                                                          {DataType::UINT64, DataType::UINT64, DataType::UINT64}};
    for (const auto &types : typeCombinations) {
        for (const auto &shiftOp : shiftOps) {
            for (const auto &ops : opcodes) {
                TestNonCommutativeBinaryOpWithShiftedOperand(types, shiftOp, ops);
            }
        }
    }
}

void LoweringTest::TestNonCommutativeBinaryOpWithShiftedOperandWithIncompatibleInstructionTypes(
    const ShiftOpPair &shiftOp, Opcode op)
{
    auto graph = CreateEmptyLowLevelGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s16();
        PARAMETER(1U, 1U).s16();
        CONSTANT(2U, 5U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, std::get<0U>(shiftOp)).s16().Inputs(0U, 2U);
            INST(4U, op).s32().Inputs(1U, 3U);

            INST(5U, std::get<0U>(shiftOp)).s16().Inputs(1U, 2U);
            INST(6U, op).s32().Inputs(5U, 1U);

            INST(7U, std::get<0U>(shiftOp)).s16().Inputs(0U, 2U);
            INST(8U, op).s32().Inputs(1U, 7U);

            INST(9U, Opcode::Max).s32().Inputs(4U, 6U);
            INST(10U, Opcode::Max).s32().Inputs(9U, 8U);
            INST(11U, Opcode::Max).s32().Inputs(10U, 7U);
            INST(12U, Opcode::Return).s32().Inputs(11U);
        }
    }

    auto clone = GraphCloner(graph, GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    EXPECT_TRUE(graph->RunPass<Lowering>());
    ASSERT_TRUE(GraphComparator().Compare(graph, clone));
}

TEST_F(LoweringTest, NonCommutativeBinaryOpWithShiftedOperandWithIncompatibleInstructionTypes)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "instructions with shifted operands are only supported on Aarch64";
    }

    std::initializer_list<Opcode> opcodes = {Opcode::Sub, Opcode::AndNot, Opcode::OrNot, Opcode::XorNot};

    std::initializer_list<ShiftOpPair> shiftOps = {{Opcode::Shl, Opcode::ShlI, ShiftType::LSL},
                                                   {Opcode::Shr, Opcode::ShrI, ShiftType::LSR},
                                                   {Opcode::AShr, Opcode::AShrI, ShiftType::ASR}};

    for (const auto &shiftOp : shiftOps) {
        for (const auto &op : opcodes) {
            TestNonCommutativeBinaryOpWithShiftedOperandWithIncompatibleInstructionTypes(shiftOp, op);
        }
    }
}

void LoweringTest::TestBitwiseInstructionsWithInvertedShiftedOperand(const TypeTriple &types, ShiftOp shiftOp,
                                                                     OpcodePair ops)
{
    auto type = types[0U];
    auto graph = CreateEmptyLowLevelGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).type(types[1U]);
        PARAMETER(1U, 1U).type(types[2U]);
        CONSTANT(2U, 5U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, shiftOp.first).type(type).Inputs(0U, 2U);
            INST(4U, Opcode::Not).type(type).Inputs(3U);
            INST(5U, ops.first).type(type).Inputs(1U, 4U);
            INST(6U, Opcode::Return).type(type).Inputs(5U);
        }
    }

    EXPECT_TRUE(graph->RunPass<Lowering>());
    EXPECT_TRUE(graph->RunPass<Cleanup>());

    auto graphExpected = CreateEmptyGraph();
    GRAPH(graphExpected)
    {
        PARAMETER(0U, 0U).type(types[1U]);
        PARAMETER(1U, 1U).type(types[2U]);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, ops.second).Shift(shiftOp.second, 5U).type(type).Inputs(1U, 0U);
            INST(3U, Opcode::Return).type(type).Inputs(2U);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(graph, graphExpected));
}

TEST_F(LoweringTest, BitwiseInstructionsWithInvertedShiftedOperand)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "instructions with shifted operands are only supported on Aarch64";
    }

    std::initializer_list<OpcodePair> opcodes = {
        {Opcode::And, Opcode::AndNotSR}, {Opcode::Or, Opcode::OrNotSR}, {Opcode::Xor, Opcode::XorNotSR}};

    std::initializer_list<ShiftOp> shiftOps = {
        {Opcode::Shl, ShiftType::LSL}, {Opcode::Shr, ShiftType::LSR}, {Opcode::AShr, ShiftType::ASR}};

    std::initializer_list<TypeTriple> typeCombinations = {{DataType::INT32, DataType::INT32, DataType::INT32},
                                                          {DataType::UINT32, DataType::UINT32, DataType::UINT32},
                                                          {DataType::UINT32, DataType::UINT16, DataType::UINT8},
                                                          {DataType::INT64, DataType::INT64, DataType::INT64},
                                                          {DataType::UINT64, DataType::UINT64, DataType::UINT64}};

    for (const auto &types : typeCombinations) {
        for (const auto &shiftOp : shiftOps) {
            for (const auto &ops : opcodes) {
                TestBitwiseInstructionsWithInvertedShiftedOperand(types, shiftOp, ops);
            }
        }
    }
}

void LoweringTest::TestBitwiseInstructionsWithInvertedShiftedOperandWithIncompatibleInstructionTypes(ShiftOp shiftOp,
                                                                                                     Opcode op)
{
    auto graph = CreateEmptyLowLevelGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s16();
        PARAMETER(1U, 1U).s16();
        CONSTANT(2U, 5U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, shiftOp.first).s16().Inputs(0U, 2U);
            INST(4U, Opcode::Not).s16().Inputs(3U);
            INST(5U, op).s32().Inputs(1U, 4U);
            INST(6U, Opcode::Return).s32().Inputs(5U);
        }
    }

    auto clone = GraphCloner(graph, GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    EXPECT_TRUE(graph->RunPass<Lowering>());
    ASSERT_TRUE(GraphComparator().Compare(graph, clone));
}

TEST_F(LoweringTest, BitwiseInstructionsWithInvertedShiftedOperandWithIncompatibleInstructionTypes)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "instructions with shifted operands are only supported on Aarch64";
    }

    std::initializer_list<Opcode> opcodes = {Opcode::And, Opcode::Or, Opcode::Xor};

    std::initializer_list<ShiftOp> shiftOps = {
        {Opcode::Shl, ShiftType::LSL}, {Opcode::Shr, ShiftType::LSR}, {Opcode::AShr, ShiftType::ASR}};

    for (const auto &shiftOp : shiftOps) {
        for (const auto &op : opcodes) {
            TestBitwiseInstructionsWithInvertedShiftedOperandWithIncompatibleInstructionTypes(shiftOp, op);
        }
    }
}

SRC_GRAPH(NegWithShiftedOperand, Graph *graph, ShiftOp shiftOp, std::pair<DataType::Type, DataType::Type> types)
{
    auto type = types.first;
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).type(types.second);
        CONSTANT(1U, 5U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, shiftOp.first).type(type).Inputs(0U, 1U);
            INST(3U, Opcode::Neg).type(type).Inputs(2U);
            INST(4U, Opcode::Return).type(type).Inputs(3U);
        }
    }
}

OUT_GRAPH(NegWithShiftedOperand, Graph *graph, ShiftOp shiftOp, std::pair<DataType::Type, DataType::Type> types)
{
    auto type = types.first;
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).type(types.second);

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::NegSR).Shift(shiftOp.second, 5U).type(type).Inputs(0U);
            INST(2U, Opcode::Return).type(type).Inputs(1U);
        }
    }
}

TEST_F(LoweringTest, NegWithShiftedOperand)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "instructions with shifted operand are only supported on Aarch64";
    }

    std::initializer_list<ShiftOp> shiftOps = {
        {Opcode::Shl, ShiftType::LSL}, {Opcode::Shr, ShiftType::LSR}, {Opcode::AShr, ShiftType::ASR}};
    std::initializer_list<std::pair<DataType::Type, DataType::Type>> typeCombinations = {
        {DataType::INT32, DataType::INT32},
        {DataType::UINT32, DataType::UINT32},
        {DataType::UINT32, DataType::UINT16},
        {DataType::INT64, DataType::INT64},
        {DataType::UINT64, DataType::UINT64}};
    for (auto &types : typeCombinations) {
        for (auto &shiftOp : shiftOps) {
            auto graph = CreateEmptyLowLevelGraph();
            src_graph::NegWithShiftedOperand::CREATE(graph, shiftOp, types);

            EXPECT_TRUE(graph->RunPass<Lowering>());
            EXPECT_TRUE(graph->RunPass<Cleanup>());

            auto graphExpected = CreateEmptyGraph();
            out_graph::NegWithShiftedOperand::CREATE(graphExpected, shiftOp, types);

            ASSERT_TRUE(GraphComparator().Compare(graph, graphExpected));
        }
    }
}

TEST_F(LoweringTest, NegWithShiftedOperandWithIncompatibleInstructionTypes)
{
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        GTEST_SKIP() << "instructions with shifted operand are only supported on Aarch64";
    }

    std::initializer_list<ShiftOp> shiftOps = {
        {Opcode::Shl, ShiftType::LSL}, {Opcode::Shr, ShiftType::LSR}, {Opcode::AShr, ShiftType::ASR}};

    for (auto &shiftOp : shiftOps) {
        auto graph = CreateEmptyLowLevelGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).s16();
            CONSTANT(1U, 5U);

            BASIC_BLOCK(2U, -1L)
            {
                INST(2U, shiftOp.first).s16().Inputs(0U, 1U);
                INST(3U, Opcode::Neg).s32().Inputs(2U);
                INST(4U, Opcode::Return).s32().Inputs(3U);
            }
        }

        auto clone = GraphCloner(graph, GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
        EXPECT_TRUE(graph->RunPass<Lowering>());
        ASSERT_TRUE(GraphComparator().Compare(graph, clone));
    }
}

TEST_F(LoweringTest, AddSwapInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        CONSTANT(1U, 5U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Add).s64().Inputs(1U, 0U);
            INST(3U, Opcode::Return).s64().Inputs(2U);
        }
    }
    EXPECT_TRUE(GetGraph()->RunPass<Lowering>());
    EXPECT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graphExpected = CreateEmptyGraph();
    GRAPH(graphExpected)
    {
        PARAMETER(0U, 0U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::AddI).s64().Inputs(0U).Imm(5U);
            INST(3U, Opcode::Return).s64().Inputs(2U);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphExpected));
}

// Not applied, sub isn't commutative inst
TEST_F(LoweringTest, SubSwapInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        CONSTANT(1U, 5U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Sub).s64().Inputs(1U, 0U);
            INST(3U, Opcode::Return).s64().Inputs(2U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_TRUE(GetGraph()->RunPass<Lowering>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(LoweringTest, DeoptimizeCompare)
{
    // Check if Compare + DeoptimizeIf ===> DeoptimizeCompare/DeoptimizeCompareImm transformations are applied.
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        CONSTANT(2U, 10U).s32();
        CONSTANT(3U, nullptr).ref();
        BASIC_BLOCK(2U, 1U)
        {
            INST(5U, Opcode::LoadArray).ref().Inputs(0U, 1U);
            INST(7U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(8U, Opcode::Compare).b().CC(ConditionCode::CC_EQ).Inputs(5U, 3U);
            INST(9U, Opcode::DeoptimizeIf).Inputs(8U, 7U);
            INST(10U, Opcode::LenArray).s32().Inputs(5U);
            INST(11U, Opcode::Compare).b().CC(ConditionCode::CC_GT).Inputs(2U, 10U);
            INST(12U, Opcode::DeoptimizeIf).Inputs(11U, 7U);
            INST(13U, Opcode::Return).ref().Inputs(5U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lowering>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        CONSTANT(2U, 10U).s32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(5U, Opcode::LoadArray).ref().Inputs(0U, 1U);
            INST(7U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(10U, Opcode::DeoptimizeCompareImm).CC(ConditionCode::CC_EQ).Imm(0U).Inputs(5U, 7U);
            INST(11U, Opcode::LenArray).s32().Inputs(5U);
            INST(14U, Opcode::DeoptimizeCompare).CC(ConditionCode::CC_GT).Inputs(2U, 11U, 7U);
            INST(15U, Opcode::Return).ref().Inputs(5U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

void LoweringTest::BuildGraphDeoptimizeCompareImmDoesNotFit(Graph *graph)
{
#ifndef NDEBUG
    graph->SetLowLevelInstructionsEnabled();
#endif
    if (graph->GetArch() == Arch::AARCH32 || graph->GetArch() == Arch::AARCH64) {
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).ref();
            PARAMETER(1U, 1U).s32();
            CONSTANT(2U, INT_MAX).s32();
            BASIC_BLOCK(2U, 1U)
            {
                INST(5U, Opcode::LoadArray).ref().Inputs(0U, 1U);
                INST(6U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U).SrcVregs({0U, 1U});
                INST(7U, Opcode::LenArray).s32().Inputs(5U);
                INST(8U, Opcode::Compare).b().CC(ConditionCode::CC_GT).Inputs(7U, 2U);
                INST(9U, Opcode::DeoptimizeIf).Inputs(8U, 6U);
                INST(10U, Opcode::Return).ref().Inputs(5U);
            }
        }
    } else if (graph->GetArch() == Arch::X86_64) {
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).ref();
            PARAMETER(1U, 1U).s32();
            CONSTANT(2U, LONG_MAX).s64();
            BASIC_BLOCK(2U, 1U)
            {
                INST(5U, Opcode::LoadArray).ref().Inputs(0U, 1U);
                INST(6U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U).SrcVregs({0U, 1U});
                INST(7U, Opcode::LenArray).s32().Inputs(5U);
                INST(8U, Opcode::Cast).s64().SrcType(compiler::DataType::INT32).Inputs(7U);
                INST(9U, Opcode::Compare).b().CC(ConditionCode::CC_GT).Inputs(8U, 2U);
                INST(10U, Opcode::DeoptimizeIf).Inputs(9U, 6U);
                INST(11U, Opcode::Return).ref().Inputs(5U);
            }
        }
    } else {
        UNREACHABLE();
    }
}

TEST_F(LoweringTest, DeoptimizeCompareImmDoesNotFit)
{
    auto graph = CreateEmptyGraph(RUNTIME_ARCH);
    BuildGraphDeoptimizeCompareImmDoesNotFit(graph);
    ASSERT_TRUE(graph->RunPass<Lowering>());
    ASSERT_TRUE(graph->RunPass<Cleanup>());

    auto graphExpected = CreateEmptyGraph(graph->GetArch());
    if (graphExpected->GetArch() == Arch::AARCH32 || graphExpected->GetArch() == Arch::AARCH64) {
        GRAPH(graphExpected)
        {
            PARAMETER(0U, 0U).ref();
            PARAMETER(1U, 1U).s32();
            CONSTANT(2U, INT_MAX).s32();
            BASIC_BLOCK(2U, 1U)
            {
                INST(5U, Opcode::LoadArray).ref().Inputs(0U, 1U);
                INST(6U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U).SrcVregs({0U, 1U});
                INST(7U, Opcode::LenArray).s32().Inputs(5U);
                INST(9U, Opcode::DeoptimizeCompare).CC(ConditionCode::CC_GT).Inputs(7U, 2U, 6U);
                INST(10U, Opcode::Return).ref().Inputs(5U);
            }
        }
    } else if (graphExpected->GetArch() == Arch::X86_64) {
        GRAPH(graphExpected)
        {
            PARAMETER(0U, 0U).ref();
            PARAMETER(1U, 1U).s32();
            CONSTANT(2U, LONG_MAX).s64();
            BASIC_BLOCK(2U, 1U)
            {
                INST(5U, Opcode::LoadArray).ref().Inputs(0U, 1U);
                INST(6U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U).SrcVregs({0U, 1U});
                INST(7U, Opcode::LenArray).s32().Inputs(5U);
                INST(8U, Opcode::Cast).s64().SrcType(compiler::DataType::INT32).Inputs(7U);
                INST(10U, Opcode::DeoptimizeCompare).CC(ConditionCode::CC_GT).Inputs(8U, 2U, 6U);
                INST(11U, Opcode::Return).ref().Inputs(5U);
            }
        }
    } else {
        UNREACHABLE();
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, graphExpected));
}

void LoweringTest::BuildGraphLowerMoveScaleInLoadStore(Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ptr();
        PARAMETER(1U, 1U).ptr();
        PARAMETER(2U, 2U).u64();
        PARAMETER(80U, 3U).u64();
        PARAMETER(3U, 4U).u32();
        CONSTANT(4U, 1U);
        CONSTANT(5U, 2U);
        CONSTANT(6U, 3U);
        CONSTANT(7U, 4U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(8U, Opcode::Shl).u64().Inputs(2U, 4U);
            INST(9U, Opcode::Load).u64().Inputs(0U, 8U);
            INST(10U, Opcode::Store).u64().Inputs(1U, 8U, 9U);
            INST(11U, Opcode::Shl).u64().Inputs(2U, 5U);
            INST(12U, Opcode::Load).u64().Inputs(0U, 11U);
            INST(13U, Opcode::Store).u64().Inputs(1U, 11U, 12U);
            INST(14U, Opcode::Shl).u64().Inputs(2U, 6U);
            INST(15U, Opcode::Load).u64().Inputs(0U, 14U);
            INST(16U, Opcode::Store).u64().Inputs(1U, 14U, 15U);
            INST(17U, Opcode::Shl).u64().Inputs(2U, 7U);
            INST(18U, Opcode::Load).u64().Inputs(0U, 17U);
            INST(19U, Opcode::Store).u64().Inputs(1U, 17U, 18U);
            INST(20U, Opcode::Shl).u64().Inputs(80U, 4U);
            INST(21U, Opcode::Load).u16().Inputs(0U, 20U);
            INST(22U, Opcode::Store).u16().Inputs(1U, 20U, 21U);
            INST(23U, Opcode::Shl).u64().Inputs(80U, 5U);
            INST(24U, Opcode::Load).u32().Inputs(0U, 23U);
            INST(25U, Opcode::Store).u32().Inputs(1U, 23U, 24U);
            INST(26U, Opcode::Shl).u32().Inputs(3U, 6U);
            INST(27U, Opcode::Load).u64().Inputs(0U, 26U);
            INST(28U, Opcode::Store).u64().Inputs(1U, 26U, 27U);
            INST(90U, Opcode::ReturnVoid).v0id();
        }
    }
}

void LoweringTest::BuildExpectedLowerMoveScaleInLoadStoreAArch64(Graph *graphExpected)
{
    GRAPH(graphExpected)
    {
        PARAMETER(0U, 0U).ptr();
        PARAMETER(1U, 1U).ptr();
        PARAMETER(2U, 2U).u64();
        PARAMETER(80U, 3U).u64();
        PARAMETER(3U, 4U).u32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(8U, Opcode::ShlI).u64().Inputs(2U).Imm(1U);
            INST(9U, Opcode::Load).u64().Inputs(0U, 8U);
            INST(10U, Opcode::Store).u64().Inputs(1U, 8U, 9U);
            INST(11U, Opcode::ShlI).u64().Inputs(2U).Imm(2U);
            INST(12U, Opcode::Load).u64().Inputs(0U, 11U);
            INST(13U, Opcode::Store).u64().Inputs(1U, 11U, 12U);
            INST(15U, Opcode::Load).u64().Inputs(0U, 2U).Scale(3U);
            INST(16U, Opcode::Store).u64().Inputs(1U, 2U, 15U).Scale(3U);
            INST(17U, Opcode::ShlI).u64().Inputs(2U).Imm(4U);
            INST(18U, Opcode::Load).u64().Inputs(0U, 17U);
            INST(19U, Opcode::Store).u64().Inputs(1U, 17U, 18U);
            INST(21U, Opcode::Load).u16().Inputs(0U, 80U).Scale(1U);
            INST(22U, Opcode::Store).u16().Inputs(1U, 80U, 21U).Scale(1U);
            INST(24U, Opcode::Load).u32().Inputs(0U, 80U).Scale(2U);
            INST(25U, Opcode::Store).u32().Inputs(1U, 80U, 24U).Scale(2U);
            INST(26U, Opcode::ShlI).u32().Inputs(3U).Imm(3U);
            INST(27U, Opcode::Load).u64().Inputs(0U, 26U);
            INST(28U, Opcode::Store).u64().Inputs(1U, 26U, 27U);
            INST(90U, Opcode::ReturnVoid).v0id();
        }
    }
}

void LoweringTest::BuildExpectedLowerMoveScaleInLoadStoreAmd64(Graph *graphExpected)
{
    GRAPH(graphExpected)
    {
        PARAMETER(0U, 0U).ptr();
        PARAMETER(1U, 1U).ptr();
        PARAMETER(2U, 2U).u64();
        PARAMETER(80U, 3U).u64();
        PARAMETER(3U, 4U).u32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(9U, Opcode::Load).u64().Inputs(0U, 2U).Scale(1U);
            INST(10U, Opcode::Store).u64().Inputs(1U, 2U, 9U).Scale(1U);
            INST(12U, Opcode::Load).u64().Inputs(0U, 2U).Scale(2U);
            INST(13U, Opcode::Store).u64().Inputs(1U, 2U, 12U).Scale(2U);
            INST(15U, Opcode::Load).u64().Inputs(0U, 2U).Scale(3U);
            INST(16U, Opcode::Store).u64().Inputs(1U, 2U, 15U).Scale(3U);
            INST(17U, Opcode::ShlI).u64().Inputs(2U).Imm(4U);
            INST(18U, Opcode::Load).u64().Inputs(0U, 17U);
            INST(19U, Opcode::Store).u64().Inputs(1U, 17U, 18U);
            INST(21U, Opcode::Load).u16().Inputs(0U, 80U).Scale(1U);
            INST(22U, Opcode::Store).u16().Inputs(1U, 80U, 21U).Scale(1U);
            INST(24U, Opcode::Load).u32().Inputs(0U, 80U).Scale(2U);
            INST(25U, Opcode::Store).u32().Inputs(1U, 80U, 24U).Scale(2U);
            INST(26U, Opcode::ShlI).u32().Inputs(3U).Imm(3U);
            INST(27U, Opcode::Load).u64().Inputs(0U, 26U);
            INST(28U, Opcode::Store).u64().Inputs(1U, 26U, 27U);
            INST(90U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(LoweringTest, LowerMoveScaleInLoadStore)
{
    auto graph = CreateEmptyFastpathGraph(RUNTIME_ARCH);
    if (graph->GetArch() == Arch::AARCH32) {
        GTEST_SKIP() << "The optimization isn't supported on Aarch32";
    }
    BuildGraphLowerMoveScaleInLoadStore(graph);
    EXPECT_TRUE(graph->RunPass<Lowering>());
    EXPECT_TRUE(graph->RunPass<Cleanup>());

    auto graphExpected = CreateEmptyFastpathGraph(RUNTIME_ARCH);
    if (graph->GetArch() == Arch::AARCH64) {
        BuildExpectedLowerMoveScaleInLoadStoreAArch64(graphExpected);
    } else {
        ASSERT(graph->GetArch() == Arch::X86_64);
        BuildExpectedLowerMoveScaleInLoadStoreAmd64(graphExpected);
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, graphExpected));

    EXPECT_TRUE(RegAlloc(graph));
    EXPECT_TRUE(graph->RunPass<Codegen>());
}

void LoweringTest::BuildGraphLowerUnsignedCast(Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ptr();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Load).u8().Inputs(0U, 1U);
            INST(3U, Opcode::Cast).u64().SrcType(DataType::UINT8).Inputs(2U);
            INST(4U, Opcode::Load).u16().Inputs(0U, 3U);
            INST(5U, Opcode::Cast).u64().SrcType(DataType::UINT16).Inputs(4U);
            INST(6U, Opcode::Load).u32().Inputs(0U, 5U);
            INST(7U, Opcode::Cast).u64().SrcType(DataType::UINT32).Inputs(6U);
            INST(8U, Opcode::Load).f32().Inputs(0U, 7U);
            INST(9U, Opcode::Cast).u64().SrcType(DataType::FLOAT32).Inputs(8U);
            INST(10U, Opcode::Load).s8().Inputs(0U, 9U);
            INST(11U, Opcode::Cast).u64().SrcType(DataType::INT8).Inputs(10U);
            INST(12U, Opcode::Load).u8().Inputs(0U, 11U);
            INST(13U, Opcode::Cast).u32().SrcType(DataType::UINT8).Inputs(12U);
            INST(14U, Opcode::Load).u64().Inputs(0U, 13U);
            INST(15U, Opcode::Cast).u32().SrcType(DataType::UINT64).Inputs(14U);
            INST(16U, Opcode::Mul).u8().Inputs(15U, 15U);
            INST(17U, Opcode::Cast).u64().SrcType(DataType::UINT8).Inputs(16U);
            INST(18U, Opcode::Return).u64().Inputs(17U);
        }
    }
}

TEST_F(LoweringTest, LowerUnsignedCast)
{
    auto graph = CreateEmptyFastpathGraph(RUNTIME_ARCH);
    BuildGraphLowerUnsignedCast(graph);
    auto graphExpected = CreateEmptyFastpathGraph(RUNTIME_ARCH);
    if (graph->GetArch() == Arch::AARCH64) {
        GRAPH(graphExpected)
        {
            PARAMETER(0U, 0U).ptr();
            PARAMETER(1U, 1U).u64();

            BASIC_BLOCK(2U, -1L)
            {
                INST(2U, Opcode::Load).u8().Inputs(0U, 1U);
                INST(4U, Opcode::Load).u16().Inputs(0U, 2U);
                INST(6U, Opcode::Load).u32().Inputs(0U, 4U);
                INST(8U, Opcode::Load).f32().Inputs(0U, 6U);
                INST(9U, Opcode::Cast).u64().SrcType(DataType::FLOAT32).Inputs(8U);
                INST(10U, Opcode::Load).s8().Inputs(0U, 9U);
                INST(11U, Opcode::Cast).u64().SrcType(DataType::INT8).Inputs(10U);
                INST(12U, Opcode::Load).u8().Inputs(0U, 11U);
                INST(14U, Opcode::Load).u64().Inputs(0U, 12U);
                INST(15U, Opcode::Cast).u32().SrcType(DataType::UINT64).Inputs(14U);
                INST(16U, Opcode::Mul).u8().Inputs(15U, 15U);
                INST(17U, Opcode::Cast).u64().SrcType(DataType::UINT8).Inputs(16U);
                INST(18U, Opcode::Return).u64().Inputs(17U);
            }
        }
    } else {
        graphExpected = GraphCloner(graph, graph->GetAllocator(), graph->GetLocalAllocator()).CloneGraph();
    }
    EXPECT_TRUE(graph->RunPass<Lowering>());
    if (graph->GetArch() == Arch::AARCH64) {
        EXPECT_TRUE(graph->RunPass<Cleanup>());
    } else {
        EXPECT_FALSE(graph->RunPass<Cleanup>());
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, graphExpected));

    EXPECT_TRUE(RegAlloc(graph));
    EXPECT_TRUE(graph->RunPass<Codegen>());
}

TEST_F(LoweringTest, UnsignedDivPowerOfTwo)
{
    for (auto type : {DataType::UINT32, DataType::UINT64}) {
        auto graph1 = CreateEmptyLowLevelGraph();
        GRAPH(graph1)
        {
            PARAMETER(0U, 0U);
            INS(0U).SetType(type);
            CONSTANT(1U, 4U);
            BASIC_BLOCK(2U, -1L)
            {
                INST(2U, Opcode::Div).Inputs(0U, 1U);
                INS(2U).SetType(type);
                INST(3U, Opcode::Return).Inputs(2U);
                INS(3U).SetType(type);
            }
        }
        ASSERT_TRUE(graph1->RunPass<Lowering>());
        graph1->RunPass<Cleanup>();
        auto graph2 = CreateEmptyLowLevelGraph();
        GRAPH(graph2)
        {
            PARAMETER(0U, 0U);
            INS(0U).SetType(type);
            BASIC_BLOCK(2U, -1L)
            {
                INST(2U, Opcode::ShrI).Inputs(0U).Imm(2U);
                INS(2U).SetType(type);
                INST(3U, Opcode::Return).Inputs(2U);
                INS(3U).SetType(type);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }
}

TEST_F(LoweringTest, SignedDivPowerOfTwoPositive)
{
    for (auto type : {DataType::INT32, DataType::INT64}) {
        auto graph1 = CreateEmptyLowLevelGraph();
        GRAPH(graph1)
        {
            PARAMETER(0U, 0U);
            INS(0U).SetType(type);
            CONSTANT(1U, 4U);
            BASIC_BLOCK(2U, -1L)
            {
                INST(2U, Opcode::Div).Inputs(0U, 1U);
                INS(2U).SetType(type);
                INST(3U, Opcode::Return).Inputs(2U);
                INS(3U).SetType(type);
            }
        }

        ASSERT_TRUE(graph1->RunPass<Lowering>());
        graph1->RunPass<Cleanup>();

        auto typeSize = DataType::GetTypeSize(type, graph1->GetArch());
        auto graph2 = CreateEmptyLowLevelGraph();
        GRAPH(graph2)
        {
            PARAMETER(0U, 0U);
            INS(0U).SetType(type);
            BASIC_BLOCK(2U, -1L)
            {
                INST(2U, Opcode::AShrI).Inputs(0U).Imm(typeSize - 1L);
                INS(2U).SetType(type);
                INST(4U, Opcode::ShrI).Inputs(2U).Imm(typeSize - 2L);  // type size - log2(4)
                INS(4U).SetType(type);
                INST(6U, Opcode::Add).Inputs(4U, 0U);
                INS(6U).SetType(type);
                INST(7U, Opcode::AShrI).Inputs(6U).Imm(2U);  // log2(4)
                INS(7U).SetType(type);
                INST(3U, Opcode::Return).Inputs(7U);
                INS(3U).SetType(type);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }
}

TEST_F(LoweringTest, SignedDivPowerOfTwoNegative)
{
    for (auto type : {DataType::INT32, DataType::INT64}) {
        auto graph1 = CreateEmptyLowLevelGraph();
        GRAPH(graph1)
        {
            PARAMETER(0U, 0U);
            INS(0U).SetType(type);
            CONSTANT(1U, -16L);
            BASIC_BLOCK(2U, -1L)
            {
                INST(2U, Opcode::Div).Inputs(0U, 1U);
                INS(2U).SetType(type);
                INST(3U, Opcode::Return).Inputs(2U);
                INS(3U).SetType(type);
            }
        }

        ASSERT_TRUE(graph1->RunPass<Lowering>());
        graph1->RunPass<Cleanup>();

        auto typeSize = DataType::GetTypeSize(type, graph1->GetArch());
        auto graph2 = CreateEmptyLowLevelGraph();
        GRAPH(graph2)
        {
            PARAMETER(0U, 0U);
            INS(0U).SetType(type);
            BASIC_BLOCK(2U, -1L)
            {
                INST(2U, Opcode::AShrI).Inputs(0U).Imm(typeSize - 1L);
                INS(2U).SetType(type);
                INST(4U, Opcode::ShrI).Inputs(2U).Imm(typeSize - 4L);  // type size - log2(16)
                INS(4U).SetType(type);
                INST(6U, Opcode::Add).Inputs(4U, 0U);
                INS(6U).SetType(type);
                INST(7U, Opcode::AShrI).Inputs(6U).Imm(4U);  // log2(16)
                INS(7U).SetType(type);
                INST(9U, Opcode::Neg).Inputs(7U);
                INS(9U).SetType(type);
                INST(3U, Opcode::Return).Inputs(9U);
                INS(3U).SetType(type);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }
}

TEST_F(LoweringTest, SignedModPowerOfTwo)
{
    auto graph1 = CreateEmptyLowLevelGraph();
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).i64();
        CONSTANT(1U, 4U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Mod).s64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).s64().Inputs(2U);
        }
    }
    auto graph2 = CreateEmptyGraph();
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        GRAPH(graph2)
        {
            PARAMETER(0U, 0U).i64();
            CONSTANT(7U, 0xfffffffffffffffcU);
            BASIC_BLOCK(2U, 1U)
            {
                INST(4U, Opcode::AddI).s64().Imm(3U).Inputs(0U);
                INST(5U, Opcode::SelectImm).s64().CC(CC_LT).Imm(0U).Inputs(4U, 0U, 0U);
                INST(6U, Opcode::And).s64().Inputs(5U, 7U);
                INST(8U, Opcode::Sub).s64().Inputs(0U, 6U);
                INST(3U, Opcode::Return).s64().Inputs(8U);
            }
        }
    } else {
        GRAPH(graph2)
        {
            PARAMETER(0U, 0U).i64();
            BASIC_BLOCK(2U, 1U)
            {
                INST(4U, Opcode::AddI).s64().Imm(3U).Inputs(0U);
                INST(5U, Opcode::SelectImm).s64().CC(CC_LT).Imm(0U).Inputs(4U, 0U, 0U);
                INST(6U, Opcode::AndI).s64().Imm(0xfffffffffffffffcU).Inputs(5U);
                INST(7U, Opcode::Sub).s64().Inputs(0U, 6U);
                INST(3U, Opcode::Return).s64().Inputs(7U);
            }
        }
    }
    ASSERT_TRUE(graph1->RunPass<Lowering>());
    graph1->RunPass<Cleanup>();
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
}

TEST_F(LoweringTest, UnsignedModPowerOfTwo)
{
    auto graph1 = CreateEmptyLowLevelGraph();
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 4U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Mod).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        PARAMETER(0U, 0U).u64();
        BASIC_BLOCK(2U, 1U)
        {
            INST(4U, Opcode::AndI).u64().Imm(3U).Inputs(0U);
            INST(3U, Opcode::Return).u64().Inputs(4U);
        }
    }
    ASSERT_TRUE(graph1->RunPass<Lowering>());
    graph1->RunPass<Cleanup>();
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
}

SRC_GRAPH(CompareBoolConstZero, Graph *graph, ConditionCode code, bool swap)
{
    GRAPH(graph)
    {
        CONSTANT(1U, 0U).s64();
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::SaveState).NoVregs();
            INST(3U, Opcode::CallStatic).b().InputsAutoType(2U);
            if (swap) {
                INST(4U, Opcode::Compare).b().Inputs(1U, 3U).CC(code);
            } else {
                INST(4U, Opcode::Compare).b().Inputs(3U, 1U).CC(code);
            }
            INST(5U, Opcode::Return).b().Inputs(4U);
        }
    }
}

OUT_GRAPH(CompareBoolConstZero, Graph *graph)
{
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::SaveState).NoVregs();
            INST(3U, Opcode::CallStatic).b().InputsAutoType(2U);
            INST(8U, Opcode::XorI).b().Inputs(3U).Imm(1U);
            INST(5U, Opcode::Return).b().Inputs(8U);
        }
    }
}

void LoweringTest::DoTestCompareBoolConstZero(ConditionCode code, bool swap)
{
    auto graph = CreateEmptyLowLevelGraph();
    src_graph::CompareBoolConstZero::CREATE(graph, code, swap);
    ASSERT_TRUE(graph->RunPass<Lowering>());
    graph->RunPass<Cleanup>();

    if (code == ConditionCode::CC_EQ) {
        auto graphFinal = CreateEmptyGraph();
        out_graph::CompareBoolConstZero::CREATE(graphFinal);
        ASSERT_TRUE(GraphComparator().Compare(graph, graphFinal));
    } else {
        auto clearGraph = GraphCloner(graph, GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
        ASSERT_TRUE(GraphComparator().Compare(graph, clearGraph));
    }
}

TEST_F(LoweringTest, CompareBoolConstZero)
{
    std::array<ConditionCode, 3U> codes {ConditionCode::CC_NE, ConditionCode::CC_EQ, ConditionCode::CC_BE};
    for (auto code : codes) {
        for (auto swap : {true, false}) {
            DoTestCompareBoolConstZero(code, swap);
        }
    }
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
