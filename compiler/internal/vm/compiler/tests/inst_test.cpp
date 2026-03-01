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
#include "optimizer/ir_builder/inst_builder-inl.h"

namespace panda::compiler {
inline bool operator==(const Input &lhs, const Inst *rhs)
{
    return lhs.GetInst() == rhs;
}

class InstTest : public GraphTest {
};

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
    GRAPH(GetGraph())
    {
        CONSTANT(0, 12);
        CONSTANT(1, 13);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(2, Opcode::Add).u64().Inputs(0, 1);
            INST(8, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(9, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(8);
        }
        BASIC_BLOCK(3, 5)
        {
            INST(3, Opcode::Not).u64().Inputs(0);
        }
        BASIC_BLOCK(4, 8, 6)
        {
            INST(4, Opcode::Not).u64().Inputs(1);
            INST(11, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(12, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(11);
        }
        BASIC_BLOCK(5, 8)
        {
            INST(7, Opcode::Sub).u64().Inputs(3, 2);
        }
        BASIC_BLOCK(6, 7) {}
        BASIC_BLOCK(7, 8)
        {
            INST(5, Opcode::Not).u64().Inputs(4);
        }
        BASIC_BLOCK(8, -1)
        {
            INST(6, Opcode::Phi).u64().Inputs({{5, 3}, {4, 4}, {7, 5}});
            INST(16, Opcode::ReturnVoid);
        }
    }

    // Check constructed dataflow
    ASSERT_TRUE(CheckUsers(INS(0), {2, 3, 8, 11}));
    ASSERT_TRUE(CheckUsers(INS(1), {2, 4, 8, 11}));
    ASSERT_TRUE(CheckUsers(INS(2), {7}));
    ASSERT_TRUE(CheckUsers(INS(3), {6, 7}));
    ASSERT_TRUE(CheckUsers(INS(4), {5, 6}));
    ASSERT_TRUE(CheckUsers(INS(5), {6}));
    ASSERT_TRUE(CheckInputs(INS(2), {0, 1}));
    ASSERT_TRUE(CheckInputs(INS(3), {0}));
    ASSERT_TRUE(CheckInputs(INS(7), {3, 2}));
    ASSERT_TRUE(CheckInputs(INS(4), {1}));
    ASSERT_TRUE(CheckInputs(INS(5), {4}));
    ASSERT_TRUE(CheckInputs(INS(6), {3, 4, 5}));
    ASSERT_EQ(static_cast<PhiInst &>(INS(6)).GetPhiInput(&BB(5)), &INS(3));
    ASSERT_EQ(static_cast<PhiInst &>(INS(6)).GetPhiInput(&BB(4)), &INS(4));
    ASSERT_EQ(static_cast<PhiInst &>(INS(6)).GetPhiInput(&BB(7)), &INS(5));

    {  // Test iterating over users of constant instruction
        const Inst *inst = &INS(2);
        for (auto &user : inst->GetUsers()) {
            ASSERT_EQ(inst, user.GetInput());
        }
    }

    {  // Test iterating over users of non-constant instruction
        Inst *inst = &INS(2);
        for (auto &user : inst->GetUsers()) {
            user.GetInst()->SetId(user.GetInst()->GetId());
        }
    }

    // 1. Remove instruction #3, replace its users by its input
    INS(3).ReplaceUsers(INS(3).GetInput(0).GetInst());
    INS(3).GetBasicBlock()->RemoveInst(&INS(3));
    ASSERT_TRUE(INS(6).GetInput(0).GetInst() == &INS(0));
    ASSERT_TRUE(INS(3).GetInput(0).GetInst() == nullptr);
    ASSERT_TRUE(CheckUsers(INS(0), {2, 6, 7, 8, 11}));
    ASSERT_EQ(static_cast<PhiInst &>(INS(6)).GetPhiInput(&BB(5)), &INS(0));
    GraphChecker(GetGraph()).Check();

    // TODO(A.Popov): refactor RemovePredsBlocks
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
    for (int i = 0; i < 4; ++i) {
        INS(6).AppendInput(&INS(0));
    }
}

TEST_F(InstTest, Arithmetics)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 12);
        CONSTANT(1, 17.23);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Cast).u64().SrcType(DataType::FLOAT64).Inputs(1);
            INST(3, Opcode::Add).u64().Inputs(0, 2);
            INST(4, Opcode::ReturnVoid);
        }
    }
}

TEST_F(InstTest, Memory)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();  // array
        PARAMETER(1, 1).u64();  // index
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::NullCheck).ref().Inputs(0, 2);
            INST(4, Opcode::LenArray).s32().Inputs(3);
            INST(5, Opcode::BoundsCheck).s32().Inputs(4, 1, 2);
            INST(6, Opcode::LoadArray).u64().Inputs(3, 5);
            INST(7, Opcode::Add).u64().Inputs(6, 6);
            INST(8, Opcode::StoreArray).u64().Inputs(3, 5, 7);
            INST(9, Opcode::ReturnVoid);
        }
    }
}

TEST_F(InstTest, Const)
{
    int32_t int32_const[3] = {-5, 0, 5};
    int64_t int64_const[3] = {-5, 0, 5};
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2, -1)
        {
            INST(0, Opcode::ReturnVoid);
        }
    }
    auto start = GetGraph()->GetStartBlock();
    for (auto i = 0; i < 3; i++) {
        int32_t val = int32_const[i];
        auto const1 = GetGraph()->FindOrCreateConstant(val);
        ASSERT_EQ(const1->GetType(), DataType::INT64);
        ASSERT_EQ(const1->GetBasicBlock(), start);
        uint64_t val1 = int64_const[i];
        auto const2 = GetGraph()->FindOrCreateConstant(val1);
        ASSERT_EQ(const2->GetType(), DataType::INT64);
        ASSERT_EQ(const1, const2);
        ASSERT_EQ(const1->GetIntValue(), val1);
    }
    GraphChecker(GetGraph()).Check();
    float float_const[3] = {-5.5f, 0.1f, 5.2f};
    for (auto i = 0; i < 3; i++) {
        float val = float_const[i];
        auto const1 = GetGraph()->FindOrCreateConstant(val);
        ASSERT_EQ(const1->GetType(), DataType::FLOAT32);
        ASSERT_EQ(const1->GetBasicBlock(), start);
        auto const2 = GetGraph()->FindOrCreateConstant(val);
        ASSERT_EQ(const1, const2);
        ASSERT_EQ(const1->GetFloatValue(), val);
    }
    GraphChecker(GetGraph()).Check();
    double double_const[3] = {-5.5, 0.1, 5.2};
    for (auto i = 0; i < 3; i++) {
        double val = double_const[i];
        auto const1 = GetGraph()->FindOrCreateConstant(val);
        ASSERT_EQ(const1->GetType(), DataType::FLOAT64);
        ASSERT_EQ(const1->GetBasicBlock(), start);
        auto const2 = GetGraph()->FindOrCreateConstant(val);
        ASSERT_EQ(const1, const2);
        ASSERT_EQ(const1->GetDoubleValue(), val);
    }
    int i = 0;
    for (auto current_const = GetGraph()->GetFirstConstInst(); current_const != nullptr;
         current_const = current_const->GetNextConst()) {
        i++;
    }
    ASSERT_EQ(i, 9);
}

TEST_F(InstTest, Const32)
{
    int32_t int32_const[3] = {-5, 0, 5};
    int64_t int64_const[3] = {-5, 0, 5};
    auto graph = CreateEmptyBytecodeGraph();

    GRAPH(graph)
    {
        BASIC_BLOCK(2, -1)
        {
            INST(0, Opcode::ReturnVoid);
        }
    }
    auto start = graph->GetStartBlock();
    for (auto i = 0; i < 3; i++) {
        // add first int32 constant
        int32_t val = int32_const[i];
        auto const1 = graph->FindOrCreateConstant(val);
        ASSERT_EQ(const1->GetType(), DataType::INT32);
        ASSERT_EQ(const1->GetBasicBlock(), start);
        uint64_t val1 = int64_const[i];
        // add int64 constant, graph creates new constant
        auto const2 = graph->FindOrCreateConstant(val1);
        ASSERT_EQ(const2->GetType(), DataType::INT64);
        ASSERT_NE(const1, const2);
        ASSERT_EQ(const2->GetBasicBlock(), start);
        ASSERT_EQ(const1->GetIntValue(), val1);
        // add second int32 constant, graph doesn't create new constant
        int32_t val2 = int32_const[i];
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
        BASIC_BLOCK(2, -1)
        {
            INST(0, Opcode::ReturnVoid);
        }
    }
}

TEST_F(InstTest, ReturnFloat)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 1.1f);
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Return).f32().Inputs(0);
        }
    }
}

TEST_F(InstTest, ReturnDouble)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 1.1);
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Return).f64().Inputs(0);
        }
    }
}

TEST_F(InstTest, ReturnLong)
{
    uint64_t i = 1;
    GRAPH(GetGraph())
    {
        CONSTANT(0, i);
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Return).u64().Inputs(0);
        }
    }
}

TEST_F(InstTest, ReturnInt)
{
    int32_t i = 1;
    GRAPH(GetGraph())
    {
        CONSTANT(0, i);
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::Return).u32().Inputs(0);
        }
    }
}

TEST_F(InstTest, ArrayChecks)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).ref();  // array
        PARAMETER(1, 1).u64();  // index
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::NullCheck).ref().Inputs(0, 2);
            INST(4, Opcode::LenArray).s32().Inputs(3);
            INST(5, Opcode::BoundsCheck).s32().Inputs(4, 1, 2);
            INST(6, Opcode::LoadArray).u64().Inputs(3, 5);
            INST(7, Opcode::Add).u64().Inputs(6, 6);
            INST(8, Opcode::StoreArray).u64().Inputs(3, 5, 7);
            INST(9, Opcode::ReturnVoid);
        }
    }
}

TEST_F(InstTest, ZeroCheck)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(3, Opcode::ZeroCheck).u64().Inputs(0, 2);
            INST(4, Opcode::Div).u64().Inputs(1, 3);
            INST(5, Opcode::Mod).u64().Inputs(1, 3);
            INST(6, Opcode::ReturnVoid);
        }
    }
}

TEST_F(InstTest, Parametr)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u32();
        PARAMETER(2, 4).u16();
        PARAMETER(3, 5).u8();
        PARAMETER(4, 8).s64();
        PARAMETER(5, 10).s32();
        PARAMETER(6, 11).s16();
        PARAMETER(7, 24).s8();
        PARAMETER(8, 27).b();
        PARAMETER(9, 28).f64();
        PARAMETER(10, 29).f32();
        PARAMETER(11, 40).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(12, Opcode::Add).u32().Inputs(1, 5);
            INST(13, Opcode::ReturnVoid);
        }
    }
}

TEST_F(InstTest, LenArray)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 40).ref();
        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::LenArray).s32().Inputs(0);
            INST(2, Opcode::ReturnVoid);
        }
    }
}

TEST_F(InstTest, Call)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).ref();
        PARAMETER(1, 2).u32();
        PARAMETER(2, 4).u16();
        PARAMETER(3, 5).u8();
        PARAMETER(4, 8).s64();
        BASIC_BLOCK(2, -1)
        {
            using namespace DataType;
            INST(8, Opcode::SaveState).NoVregs();
            INST(5, Opcode::CallVirtual).s32().InputsAutoType(0, 2, 4, 8);
            INST(6, Opcode::CallStatic).b().InputsAutoType(1, 3, 4, 5, 8);
            INST(7, Opcode::ReturnVoid);
        }
    }
}

TEST_F(InstTest, BinaryImmOperation)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 1).u64();
        PARAMETER(1, 4).s32();
        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::AddI).s32().Imm(10ULL).Inputs(1);
            INST(3, Opcode::SubI).s32().Imm(15ULL).Inputs(2);
            INST(4, Opcode::AndI).u64().Imm(15ULL).Inputs(0);
            INST(5, Opcode::OrI).u64().Imm(1ULL).Inputs(4);
            INST(6, Opcode::XorI).u64().Imm(10ULL).Inputs(5);
            INST(7, Opcode::ShlI).u64().Imm(5ULL).Inputs(6);
            INST(8, Opcode::ShrI).u64().Imm(5ULL).Inputs(7);
            INST(9, Opcode::AShrI).s32().Imm(4ULL).Inputs(3);
            INST(10, Opcode::ReturnVoid);
        }
    }
}

TEST_F(InstTest, Fcmp)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).f32();
        PARAMETER(1, 1).f32();
        PARAMETER(2, 2).f64();
        PARAMETER(3, 3).f64();
        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::Cmp).s32().Inputs(0, 1);
            INST(5, Opcode::Cmp).s32().Inputs(2, 3);
            INST(6, Opcode::ReturnVoid);
        }
    }
    GraphChecker(GetGraph()).Check();
    auto inst4 = static_cast<CmpInst *>(&INS(4));
    auto inst5 = static_cast<CmpInst *>(&INS(5));
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
    Register R0 = 0;
    Register R1 = 1;
    StackSlot slot0 = 0;
    StackSlot slot1 = 1;

    auto spill_fill_inst = GetGraph()->CreateInstSpillFill();
    spill_fill_inst->AddFill(slot0, R0, DataType::UINT64);
    spill_fill_inst->AddMove(R0, R1, DataType::UINT64);
    spill_fill_inst->AddSpill(R1, slot1, DataType::UINT64);

    ASSERT_EQ(spill_fill_inst->GetSpillFills().size(), 3U);
}

TEST_F(InstTest, RemovePhiInput)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0, 0);
        CONSTANT(1, 1);
        CONSTANT(2, 2);
        BASIC_BLOCK(2, 3, 5)
        {
            INST(5, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(0, 1);
            INST(6, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(5);
        }
        BASIC_BLOCK(3, 5) {}
        BASIC_BLOCK(5, -1)
        {
            INST(3, Opcode::Phi).u64().Inputs({{2, 0}, {3, 1}});
            INST(4, Opcode::ReturnVoid);
        }
    }
    auto init_inputs = INS(3).GetInputs();
    auto init_preds = BB(5).GetPredsBlocks();

    auto pred_bb_idx = INS(3).CastToPhi()->GetPredBlockIndex(&BB(3));
    BB(5).RemovePred(&BB(3));
    INS(3).RemoveInput(pred_bb_idx);

    auto curr_inputs = INS(3).GetInputs();
    auto curr_preds = BB(5).GetPredsBlocks();
    for (size_t idx = 0; idx < curr_inputs.size(); idx++) {
        if (idx != pred_bb_idx) {
            ASSERT_EQ(init_inputs[idx].GetInst(), curr_inputs[idx].GetInst());
            ASSERT_EQ(init_preds[idx], curr_preds[idx]);
        } else {
            ASSERT_EQ(init_inputs.rbegin()->GetInst(), curr_inputs[idx].GetInst());
            ASSERT_EQ(init_preds.back(), curr_preds[idx]);
        }
    }
}

/**
 * Test creating instruction with huge dynamic inputs amount
 */
TEST_F(InstTest, HugeDynamicOperandsAmount)
{
    auto graph = CreateGraphStartEndBlocks();
    const size_t COUNT = 1000;
    auto save_state = graph->CreateInstSaveState();

    for (size_t i = 0; i < COUNT; i++) {
        save_state->AppendInput(graph->FindOrCreateConstant(i));
        save_state->SetVirtualRegister(i, VirtualRegister(i, false));
    }

    for (size_t i = 0; i < COUNT; i++) {
        auto user = graph->FindOrCreateConstant(i)->GetUsers().begin()->GetInst();
        ASSERT_EQ(user, save_state);
    }
}

TEST_F(InstTest, FloatConstants)
{
    auto graph = CreateGraphStartEndBlocks();
    graph->GetStartBlock()->AddSucc(graph->GetEndBlock());
    auto positiv_zero_float = graph->FindOrCreateConstant(0.0f);
    auto negativ_zero_float = graph->FindOrCreateConstant(-0.0f);
    auto positiv_zero_double = graph->FindOrCreateConstant(0.0);
    auto negativ_zero_double = graph->FindOrCreateConstant(-0.0);

    ASSERT_NE(positiv_zero_float, negativ_zero_float);
    ASSERT_NE(positiv_zero_double, negativ_zero_double);
}

TEST_F(InstTest, Flags)
{
    auto initial_mask = inst_flags::GetFlagsMask(Opcode::LoadObject);
    auto inst = GetGraph()->CreateInstLoadObject();
    ASSERT_EQ(initial_mask, inst->GetFlagsMask());
    ASSERT_EQ(inst->GetFlagsMask(), initial_mask);
    ASSERT_TRUE(inst->IsLoad());
    inst->SetFlag(inst_flags::ALLOC);
    ASSERT_EQ(inst->GetFlagsMask(), initial_mask | inst_flags::ALLOC);
    inst->ClearFlag(inst_flags::LOAD);
    ASSERT_FALSE(inst->IsLoad());
    ASSERT_EQ(inst->GetFlagsMask(), (initial_mask | inst_flags::ALLOC) & ~inst_flags::LOAD);
}

TEST_F(InstTest, IntrinsicFlags)
{
    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
#include "intrinsic_flags_test.inl"
}

}  // namespace panda::compiler
