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

namespace ark::compiler {

class InstTest : public GraphTest {};

// NOLINTBEGIN(readability-magic-numbers)
SRC_GRAPH(Dataflow, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 12U);
        CONSTANT(1U, 13U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(8U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(3U, Opcode::Not).u64().Inputs(0U);
        }
        BASIC_BLOCK(4U, 8U, 6U)
        {
            INST(4U, Opcode::Not).u64().Inputs(1U);
            INST(11U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(11U);
        }
        BASIC_BLOCK(5U, 8U)
        {
            INST(7U, Opcode::Sub).u64().Inputs(3U, 2U);
        }
        BASIC_BLOCK(6U, 7U) {}
        BASIC_BLOCK(7U, 8U)
        {
            INST(5U, Opcode::Not).u64().Inputs(4U);
        }
        BASIC_BLOCK(8U, -1L)
        {
            INST(6U, Opcode::Phi).u64().Inputs({{5U, 3U}, {4U, 4U}, {7U, 5U}});
            INST(16U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(InstTest, Dataflow)
{
    /**
     * '=' is a definition
     *
     *           [2]
     *            |
     *    /---------------\
     *    |               |
     *   [3]=            [4]=
     *    |               |
     *    |          /---------\
     *   [5]         |         |
     *    |          |        [6] (need for removing #6)
     *    |          |         |
     *    |          |        [7]=
     *    |          |         |
     *    \---------[8]--------/
     *          PHI(1,2,4)
     *
     */
    src_graph::Dataflow::CREATE(GetGraph());

    // Check constructed dataflow
    ASSERT_TRUE(CheckUsers(INS(0U), {2U, 3U, 8U, 11U}));
    ASSERT_TRUE(CheckUsers(INS(1U), {2U, 4U, 8U, 11U}));
    ASSERT_TRUE(CheckUsers(INS(2U), {7U}));
    ASSERT_TRUE(CheckUsers(INS(3U), {6U, 7U}));
    ASSERT_TRUE(CheckUsers(INS(4U), {5U, 6U}));
    ASSERT_TRUE(CheckUsers(INS(5U), {6U}));
    ASSERT_TRUE(CheckInputs(INS(2U), {0U, 1U}));
    ASSERT_TRUE(CheckInputs(INS(3U), {0U}));
    ASSERT_TRUE(CheckInputs(INS(7U), {3U, 2U}));
    ASSERT_TRUE(CheckInputs(INS(4U), {1U}));
    ASSERT_TRUE(CheckInputs(INS(5U), {4U}));
    ASSERT_TRUE(CheckInputs(INS(6U), {3U, 4U, 5U}));
    ASSERT_EQ(static_cast<PhiInst &>(INS(6U)).GetPhiInput(&BB(5U)), &INS(3U));
    ASSERT_EQ(static_cast<PhiInst &>(INS(6U)).GetPhiInput(&BB(4U)), &INS(4U));
    ASSERT_EQ(static_cast<PhiInst &>(INS(6U)).GetPhiInput(&BB(7U)), &INS(5U));

    {  // Test iterating over users of constant instruction
        const Inst *inst = &INS(2U);
        for (auto &user : inst->GetUsers()) {
            ASSERT_EQ(inst, user.GetInput());
        }
    }

    {  // Test iterating over users of non-constant instruction
        Inst *inst = &INS(2U);
        for (auto &user : inst->GetUsers()) {
            user.GetInst()->SetId(user.GetInst()->GetId());
        }
    }

    // 1. Remove instruction #3, replace its users by its input
    INS(3U).ReplaceUsers(INS(3U).GetInput(0U).GetInst());
    INS(3U).GetBasicBlock()->RemoveInst(&INS(3U));
    ASSERT_TRUE(INS(6U).GetInput(0U) == &INS(0U));
    ASSERT_TRUE(INS(3U).GetInput(0U) == nullptr);
    ASSERT_TRUE(CheckUsers(INS(0U), {2U, 6U, 7U, 8U, 11U}));
    ASSERT_EQ(static_cast<PhiInst &>(INS(6U)).GetPhiInput(&BB(5U)), &INS(0U));
    GraphChecker(GetGraph()).Check();

    // NOTE(A.Popov): refactor RemovePredsBlocks
    // 2. Remove basic block #4, phi should be fixed properly
    // INS(5).RemoveInputs()
    // INS(5).GetBasicBlock()->EraseInst(&INS(5))
    // GetGraph()->DisconnectBlock(&BB(7))
    // ASSERT_TRUE(INS(6).GetInputsCount() == 2)
    // static_cast<PhiInst&>(INS(6)).GetPhiInput(&BB(5)), &INS(0))
    // static_cast<PhiInst&>(INS(6)).GetPhiInput(&BB(4)), &INS(4))
    GraphChecker(GetGraph()).Check();

    // 3. Append additional inputs into PHI, thereby force it to reallocate inputs storage, dataflow is not valid  from
    // this moment
    for (size_t i = 0; i < 4U; ++i) {
        INS(6U).AppendInput(&INS(0U));
    }
}

TEST_F(InstTest, Arithmetics)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 12U);
        CONSTANT(1U, 17.23_D);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Cast).u64().SrcType(DataType::FLOAT64).Inputs(1U);
            INST(3U, Opcode::Add).u64().Inputs(0U, 2U);
            INST(4U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(InstTest, Memory)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();  // array
        PARAMETER(1U, 1U).u64();  // index
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::NullCheck).ref().Inputs(0U, 2U);
            INST(4U, Opcode::LenArray).s32().Inputs(3U);
            INST(5U, Opcode::BoundsCheck).s32().Inputs(4U, 1U, 2U);
            INST(6U, Opcode::LoadArray).u64().Inputs(3U, 5U);
            INST(7U, Opcode::Add).u64().Inputs(6U, 6U);
            INST(8U, Opcode::StoreArray).u64().Inputs(3U, 5U, 7U);
            INST(9U, Opcode::ReturnVoid);
        }
    }
}

SRC_GRAPH(Const, Graph *graph)
{
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(0U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(InstTest, Const)
{
    std::array<int32_t, 3U> int32Const {-5L, 0U, 5U};
    std::array<int64_t, 3U> int64Const {-5L, 0U, 5U};
    src_graph::Const::CREATE(GetGraph());
    auto start = GetGraph()->GetStartBlock();
    for (size_t i = 0; i < 3U; i++) {
        int32_t val = int32Const[i];
        auto const1 = GetGraph()->FindOrCreateConstant(val);
        ASSERT_EQ(const1->GetType(), DataType::INT64);
        ASSERT_EQ(const1->GetBasicBlock(), start);
        uint64_t val1 = int64Const[i];
        auto const2 = GetGraph()->FindOrCreateConstant(val1);
        ASSERT_EQ(const2->GetType(), DataType::INT64);
        ASSERT_EQ(const1, const2);
        ASSERT_EQ(const1->GetIntValue(), val1);
    }
    GraphChecker(GetGraph()).Check();
    std::array<float, 3U> floatConst {-5.5F, 0.1F, 5.2F};
    for (size_t i = 0; i < 3U; i++) {
        float val = floatConst[i];
        auto const1 = GetGraph()->FindOrCreateConstant(val);
        ASSERT_EQ(const1->GetType(), DataType::FLOAT32);
        ASSERT_EQ(const1->GetBasicBlock(), start);
        auto const2 = GetGraph()->FindOrCreateConstant(val);
        ASSERT_EQ(const1, const2);
        ASSERT_EQ(const1->GetFloatValue(), val);
    }
    GraphChecker(GetGraph()).Check();
    std::array<double, 3U> doubleConst {-5.5, 0.1, 5.2};
    for (size_t i = 0; i < 3U; i++) {
        double val = doubleConst[i];
        auto const1 = GetGraph()->FindOrCreateConstant(val);
        ASSERT_EQ(const1->GetType(), DataType::FLOAT64);
        ASSERT_EQ(const1->GetBasicBlock(), start);
        auto const2 = GetGraph()->FindOrCreateConstant(val);
        ASSERT_EQ(const1, const2);
        ASSERT_EQ(const1->GetDoubleValue(), val);
    }
    int i = 0;
    for (auto currentConst = GetGraph()->GetFirstConstInst(); currentConst != nullptr;
         currentConst = currentConst->GetNextConst()) {
        i++;
    }
    ASSERT_EQ(i, 9U);
}

TEST_F(InstTest, Const32)
{
    std::array<int32_t, 3U> int32Const {-5L, 0U, 5U};
    std::array<int64_t, 3U> int64Const {-5L, 0U, 5U};
    auto graph = CreateEmptyBytecodeGraph();

    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(0U, Opcode::ReturnVoid);
        }
    }
    auto start = graph->GetStartBlock();
    for (size_t i = 0; i < 3L; i++) {
        // add first int32 constant
        int32_t val = int32Const[i];
        auto const1 = graph->FindOrCreateConstant(val);
        ASSERT_EQ(const1->GetType(), DataType::INT32);
        ASSERT_EQ(const1->GetBasicBlock(), start);
        uint64_t val1 = int64Const[i];
        // add int64 constant, graph creates new constant
        auto const2 = graph->FindOrCreateConstant(val1);
        ASSERT_EQ(const2->GetType(), DataType::INT64);
        ASSERT_NE(const1, const2);
        ASSERT_EQ(const2->GetBasicBlock(), start);
        ASSERT_EQ(const1->GetIntValue(), val1);
        // add second int32 constant, graph doesn't create new constant
        int32_t val2 = int32Const[i];
        auto const3 = graph->FindOrCreateConstant(val2);
        ASSERT_EQ(const3, const1);
        ASSERT_EQ(const1->GetInt32Value(), val2);
    }
    GraphChecker(graph).Check();
}

TEST_F(InstTest, ReturnVoid)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(0U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(InstTest, ReturnFloat)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1.1F);
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Return).f32().Inputs(0U);
        }
    }
}

TEST_F(InstTest, ReturnDouble)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1.1_D);
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Return).f64().Inputs(0U);
        }
    }
}

TEST_F(InstTest, ReturnLong)
{
    uint64_t i = 1;
    GRAPH(GetGraph())
    {
        CONSTANT(0U, i);
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Return).u64().Inputs(0U);
        }
    }
}

TEST_F(InstTest, ReturnInt)
{
    int32_t i = 1;
    GRAPH(GetGraph())
    {
        CONSTANT(0U, i);
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Return).u32().Inputs(0U);
        }
    }
}

TEST_F(InstTest, ArrayChecks)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();  // array
        PARAMETER(1U, 1U).u64();  // index
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::NullCheck).ref().Inputs(0U, 2U);
            INST(4U, Opcode::LenArray).s32().Inputs(3U);
            INST(5U, Opcode::BoundsCheck).s32().Inputs(4U, 1U, 2U);
            INST(6U, Opcode::LoadArray).u64().Inputs(3U, 5U);
            INST(7U, Opcode::Add).u64().Inputs(6U, 6U);
            INST(8U, Opcode::StoreArray).u64().Inputs(3U, 5U, 7U);
            INST(9U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(InstTest, ZeroCheck)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::ZeroCheck).u64().Inputs(0U, 2U);
            INST(4U, Opcode::Div).u64().Inputs(1U, 3U);
            INST(5U, Opcode::Mod).u64().Inputs(1U, 3U);
            INST(6U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(InstTest, Parametr)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u32();
        PARAMETER(2U, 4U).u16();
        PARAMETER(3U, 5U).u8();
        PARAMETER(4U, 8U).s64();
        PARAMETER(5U, 10U).s32();
        PARAMETER(6U, 11U).s16();
        PARAMETER(7U, 24U).s8();
        PARAMETER(8U, 27U).b();
        PARAMETER(9U, 28U).f64();
        PARAMETER(10U, 29U).f32();
        PARAMETER(11U, 40U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(12U, Opcode::Add).u32().Inputs(1U, 5U);
            INST(13U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(InstTest, LenArray)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 40U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::LenArray).s32().Inputs(0U);
            INST(2U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(InstTest, Call)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).ref();
        PARAMETER(1U, 2U).u32();
        PARAMETER(2U, 4U).u16();
        PARAMETER(3U, 5U).u8();
        PARAMETER(4U, 8U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            using namespace DataType;  // NOLINT(*-build-using-namespace)
            INST(8U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::CallVirtual).s32().InputsAutoType(0U, 2U, 4U, 8U);
            INST(6U, Opcode::CallStatic).b().InputsAutoType(1U, 3U, 4U, 5U, 8U);
            INST(7U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(InstTest, BinaryImmOperation)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).u64();
        PARAMETER(1U, 4U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::AddI).s32().Imm(10ULL).Inputs(1U);
            INST(3U, Opcode::SubI).s32().Imm(15ULL).Inputs(2U);
            INST(4U, Opcode::AndI).u64().Imm(15ULL).Inputs(0U);
            INST(5U, Opcode::OrI).u64().Imm(1ULL).Inputs(4U);
            INST(6U, Opcode::XorI).u64().Imm(10ULL).Inputs(5U);
            INST(7U, Opcode::ShlI).u64().Imm(5ULL).Inputs(6U);
            INST(8U, Opcode::ShrI).u64().Imm(5ULL).Inputs(7U);
            INST(9U, Opcode::AShrI).s32().Imm(4ULL).Inputs(3U);
            INST(10U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(InstTest, Fcmp)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).f32();
        PARAMETER(1U, 1U).f32();
        PARAMETER(2U, 2U).f64();
        PARAMETER(3U, 3U).f64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::Cmp).s32().Inputs(0U, 1U);
            INST(5U, Opcode::Cmp).s32().Inputs(2U, 3U);
            INST(6U, Opcode::ReturnVoid);
        }
    }
    GraphChecker(GetGraph()).Check();
    auto inst4 = static_cast<CmpInst *>(&INS(4U));
    auto inst5 = static_cast<CmpInst *>(&INS(5U));
    inst4->SetFcmpl();
    ASSERT_EQ(inst4->IsFcmpg(), false);
    ASSERT_EQ(inst4->IsFcmpl(), true);
    inst4->SetFcmpl(true);
    ASSERT_EQ(inst4->IsFcmpg(), false);
    ASSERT_EQ(inst4->IsFcmpl(), true);
    inst4->SetFcmpl(false);
    ASSERT_EQ(inst4->IsFcmpg(), true);
    ASSERT_EQ(inst4->IsFcmpl(), false);
    inst5->SetFcmpg();
    ASSERT_EQ(inst5->IsFcmpg(), true);
    ASSERT_EQ(inst5->IsFcmpl(), false);
    inst5->SetFcmpg(true);
    ASSERT_EQ(inst5->IsFcmpg(), true);
    ASSERT_EQ(inst5->IsFcmpl(), false);
    inst5->SetFcmpg(false);
    ASSERT_EQ(inst5->IsFcmpg(), false);
    ASSERT_EQ(inst5->IsFcmpl(), true);
}

TEST_F(InstTest, SpillFill)
{
    Register r0 = 0;
    Register r1 = 1;
    StackSlot slot0 = 0;
    StackSlot slot1 = 1;

    auto spillFillInst = GetGraph()->CreateInstSpillFill();
    spillFillInst->AddFill(slot0, r0, DataType::UINT64);
    spillFillInst->AddMove(r0, r1, DataType::UINT64);
    spillFillInst->AddSpill(r1, slot1, DataType::UINT64);

    ASSERT_EQ(spillFillInst->GetSpillFills().size(), 3U);
}

TEST_F(InstTest, RemovePhiInput)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(2U, 2U);
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(5U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0U, 1U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 5U) {}
        BASIC_BLOCK(5U, -1L)
        {
            INST(3U, Opcode::Phi).u64().Inputs({{2U, 0U}, {3U, 1U}});
            INST(4U, Opcode::ReturnVoid);
        }
    }
    auto initInputs = INS(3U).GetInputs();
    auto initPreds = BB(5U).GetPredsBlocks();

    auto predBbIdx = INS(3U).CastToPhi()->GetPredBlockIndex(&BB(3U));
    BB(5U).RemovePred(&BB(3U));
    INS(3U).RemoveInput(predBbIdx);

    auto currInputs = INS(3U).GetInputs();
    auto currPreds = BB(5U).GetPredsBlocks();
    for (size_t idx = 0; idx < currInputs.size(); idx++) {
        if (idx != predBbIdx) {
            ASSERT_EQ(initInputs[idx].GetInst(), currInputs[idx].GetInst());
            ASSERT_EQ(initPreds[idx], currPreds[idx]);
        } else {
            ASSERT_EQ(initInputs.rbegin()->GetInst(), currInputs[idx].GetInst());
            ASSERT_EQ(initPreds.back(), currPreds[idx]);
        }
    }
}

/// Test creating instruction with huge dynamic inputs amount
TEST_F(InstTest, HugeDynamicOperandsAmount)
{
    auto graph = CreateGraphStartEndBlocks();
    const size_t count = 1000;
    auto saveState = graph->CreateInstSaveState();

    for (size_t i = 0; i < count; i++) {
        saveState->AppendInput(graph->FindOrCreateConstant(i));
        saveState->SetVirtualRegister(i, VirtualRegister(i, VRegInfo::VRegType::VREG));
    }

    for (size_t i = 0; i < count; i++) {
        auto user = graph->FindOrCreateConstant(i)->GetUsers().begin()->GetInst();
        ASSERT_EQ(user, saveState);
    }
}

TEST_F(InstTest, FloatConstants)
{
    auto graph = CreateGraphStartEndBlocks();
    graph->GetStartBlock()->AddSucc(graph->GetEndBlock());
    auto positivZeroFloat = graph->FindOrCreateConstant(0.0F);
    auto negativZeroFloat = graph->FindOrCreateConstant(-0.0F);
    auto positivZeroDouble = graph->FindOrCreateConstant(0.0);
    auto negativZeroDouble = graph->FindOrCreateConstant(-0.0);

    ASSERT_NE(positivZeroFloat, negativZeroFloat);
    ASSERT_NE(positivZeroDouble, negativZeroDouble);
}

TEST_F(InstTest, Flags)
{
    auto initialMask = inst_flags::GetFlagsMask(Opcode::LoadObject);
    auto inst = GetGraph()->CreateInstLoadObject();
    ASSERT_EQ(initialMask, inst->GetFlagsMask());
    ASSERT_EQ(inst->GetFlagsMask(), initialMask);
    ASSERT_TRUE(inst->IsLoad());
    inst->SetFlag(inst_flags::ALLOC);
    ASSERT_EQ(inst->GetFlagsMask(), initialMask | inst_flags::ALLOC);
    ASSERT_TRUE(inst->IsAllocation());
    inst->ClearFlag(inst_flags::LOAD);
    ASSERT_FALSE(inst->IsLoad());
    ASSERT_EQ(inst->GetFlagsMask(), (initialMask | inst_flags::ALLOC) & ~inst_flags::LOAD);
}

TEST_F(InstTest, IntrinsicFlags)
{
    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
#include "intrinsic_flags_test.inl"
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
